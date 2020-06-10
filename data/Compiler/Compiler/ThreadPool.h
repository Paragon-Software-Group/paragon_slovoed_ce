#pragma once

#include <functional>

// XXX: Some info on how to use this and on the expected lifetimes.
//      Though the API is very simple and straightforward.
class ThreadPool {
public:
	struct TaskStruct;

	using Task = TaskStruct*;
	using TaskFunc = std::function<void()>;

	// Creates and initializes a new thread pool
	// If @numThreads is 0 the thread count is equal to the number of logical
	// processor cores (which means it *will* "oversubscribe" the machine by 1 thread)
	// If @numThreads is <0 does not initialize a thread pool at all
	ThreadPool(int numThreads = 0);
	~ThreadPool();

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	// Returns if the *thread pool* is active, as in it has at least 1 worker thread
	bool active() const;

	// Queue a new task (the "main" std::function version...)
	Task queue(TaskFunc&& work);

	// Queue a new task - small helper template
	template <typename T>
	Task queue(void(*func)(T), T data) {
		return queue([func, data]() { func(data); });
	}

	// Wait for tasks to complete. Can be called from enqueued work.
	void complete(Task *tasks, size_t count);

	// Wait for a single task to complete. Can be called from enqueued work.
	void complete(Task task);

private:
	struct Private;
	Private *priv;
};
