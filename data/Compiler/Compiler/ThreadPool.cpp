#include "ThreadPool.h"

#include <cassert>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

#include "simple_list.h"
#include "Log.h"

enum TaskState { Pending, Active, Completed, Free };

struct ThreadPool::TaskStruct
{
	sld::list_node link;

	TaskState state;
	std::condition_variable completed;
	std::function<void()> func;

	TaskStruct() : state(Free) {}
};

// private thread pool implementation that actually drives the whole thing
struct ThreadPool::Private
{
	// local typedef to save on the typing
	using lock = std::unique_lock<std::mutex>;

	std::mutex mutex;
	bool stopped;

	std::list<std::thread> workers;
	std::condition_variable wakeup;
	sld::simple_list<TaskStruct, &TaskStruct::link> pendingWork;
	// used to "cache" tasks to save a little bit of mem
	sld::simple_list<TaskStruct, &TaskStruct::link> freeList;

	Private(unsigned numThreads) : stopped(false)
	{
#ifndef NDEBUG
		sld::printf("\nInitializing ThreadPool with %u thread(s).\n", numThreads);
#endif
		for (unsigned i : xrange(numThreads))
			workers.emplace_back(worker, this);
	}

	~Private()
	{
		{
			lock lk(mutex);
			stopped = true;
		}

		wakeup.notify_all();
		for (std::thread &thread : workers)
			thread.join();

#ifndef NDEBUG
		const size_t freeCount = freeList.size();
		const size_t pendingCount = pendingWork.size();
		sld::printf("\nDestroying ThreadPool: [%lu] pending tasks; [%lu] free tasks.\n", pendingCount, freeCount);
#endif

		freeList.clear([](Task task) { delete task; });
	}

	// queues a new task
	Task queue(TaskFunc&& work)
	{
		lock lk(mutex);
		assert(!stopped);

		Task task;
		if (freeList.empty())
		{
			task = new TaskStruct();
		}
		else
		{
			task = freeList.front();
			sld::unlink(task->link);
		}
		task->func = std::move(work);
		task->state = Pending;

		pendingWork.push_back(task);
		wakeup.notify_one();

		return task;
	}

	// waits on a given array of @tasks to complete
	void complete(Task *tasks, size_t count)
	{
		lock lk(mutex);

		bool prioritized = false;
		for (size_t i : xrange(count))
		{
			Task task = tasks[i];
			if (task->state != Pending)
				continue;

			// prioritize the tasks by moving them to the head of the list
			// mostly required for "nested" work waited on from other enqueued work
			if (!prioritized)
			{
				// we want to preserve the order of tasks waited on
				for (size_t j = count - 1; j > i; j--)
				{
					if (tasks[j]->state == Pending)
						pendingWork.move_to_front(tasks[j]);
				}
				prioritized = true;
			}

			// exec the task in the *current* thread
			exec(task, lk);
		}

		wait(tasks, count, lk);
	}

	// waits on a given @task to complete
	void complete(Task task)
	{
		lock lk(mutex);

		if (task->state == Pending)
			exec(task, lk);

		wait(&task, 1, lk);
	}

	// "private"

	// executes the passed in @task, @lk must be held
	void exec(Task task, lock &lk)
	{
		assert(task->state == Pending);

		sld::unlink(task->link);

		task->state = Active;

		// do the work!
		lk.unlock();
		task->func();
		lk.lock();

		task->state = Completed;
	}

	// waits on a given array of @tasks, @lk must be held
	void wait(Task *tasks, size_t count, lock &lk)
	{
		for (Task task : iterate(tasks, count))
		{
			if (task->state == Free)
				continue;

			while (task->state != Completed)
				task->completed.wait(lk);

			task->func = nullptr;
			task->state = Free;
			freeList.push_back(task);
		}
	}

	// the main thread worker function
	static void worker(Private *pool)
	{
		lock lk(pool->mutex);

		for (;;)
		{
			// wait until there are tasks
			while (pool->pendingWork.empty() && !pool->stopped)
				pool->wakeup.wait(lk);

			if (pool->stopped)
				break;

			assert(!pool->pendingWork.empty());
			Task task = pool->pendingWork.front();
			pool->exec(task, lk);
			task->completed.notify_one();
		}
	}
};

ThreadPool::ThreadPool(int numThreads) : priv(nullptr)
{
	if (numThreads < 0)
		return;

	if (numThreads == 0)
		numThreads = std::thread::hardware_concurrency();

	assert(numThreads);
	priv = new Private(numThreads);
}

ThreadPool::~ThreadPool()
{
	if (priv)
		delete priv;
}

// Returns if the *thread pool* is active, as in it has worker threads
bool ThreadPool::active() const
{
	return priv != nullptr;
}

// Queue a new task
ThreadPool::Task ThreadPool::queue(TaskFunc&& work)
{
	return priv ? priv->queue(std::move(work)) : nullptr;
}

// Wait for tasks to complete, and destroy them. Can be called from enqued workers.
void ThreadPool::complete(Task *tasks, size_t count)
{
	if (priv)
		priv->complete(tasks, count);
}

// Wait for a single task to complete
void ThreadPool::complete(Task task)
{
	if (priv)
		priv->complete(task);
}
