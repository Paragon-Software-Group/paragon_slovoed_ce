package shdd.android.components.httpdownloader;

import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.PowerManager;
import android.util.Pair;

public class Downloader {
	
	private final Context ctx;
	private final Handler handler;
	private PowerManager.WakeLock wakeLock;
	private final ExecutorService executorService;
	private final LinkedHashMap<Request, Pair<DownloadTask, Future<DownloadTask>>> queue;
	private DownloadListener listener;
	private final LinkedList<DownloadListener> listeners;
	private DownloadRawListener rawListener;
	private final Notifier notifier;
	
	public Downloader(Context ctx, DownloadClientHelper dch) {
		Helpers.assertMainTread();
		this.ctx = ctx;
		this.handler = new Handler();
		this.executorService = Executors.newFixedThreadPool(Constants.MAX_DOWNLOADS);
		this.queue = new LinkedHashMap<Request, Pair<DownloadTask, Future<DownloadTask>>>();
		this.listeners = new LinkedList<DownloadListener>();
		this.notifier = new Notifier(ctx, dch);
	}
	
	public boolean add(Request request) {
//android.util.Log.e("shdd", "Downloader.add(): new request: " + queue.containsKey(request) + "; " + request.url + "; " + request.destDir + "; " + request.destFilename + "; " + request.fileSize);
		Helpers.assertMainTread();
		if (queue.containsKey(request)) return false;
		DownloadTask task = new DownloadTask(ctx, this, request);
		queue.put(request, new Pair(task, executorService.submit(task)));
		updateLock();
		return true;
	}
	public boolean pause(Request request) {
//android.util.Log.e("shdd", "Downloader.pause(): new request: " + queue.containsKey(request) + "; " + request.url + "; " + request.destDir + "; " + request.destFilename + "; " + request.fileSize);
		Helpers.assertMainTread();
		if (!queue.containsKey(request)) return false;
		notifier.remove(request);
		Pair<DownloadTask, Future<DownloadTask>> pair = queue.remove(request);
		pair.first.setInterrupted(Status.PAUSED);
		pair.second.cancel(true);
		updateLock();
		return true;
	}
	public boolean cancel(Request request) {
//android.util.Log.e("shdd", "Downloader.cancel(): new request: " + queue.containsKey(request) + "; " + request.url + "; " + request.destDir + "; " + request.destFilename + "; " + request.fileSize);
		Helpers.assertMainTread();
		if (!queue.containsKey(request)) return false;
		notifier.remove(request);
		Pair<DownloadTask, Future<DownloadTask>> pair = queue.remove(request);
		pair.first.setInterrupted(Status.CANCELED);
		pair.second.cancel(true);
		updateLock();
		return true;
	}
	public void removeNotification(Request request) {
		Helpers.assertMainTread();
		notifier.remove(request);
	}
	public LinkedHashSet<Request> list() {
		Helpers.assertMainTread();
		return new LinkedHashSet<Request>(queue.keySet());
	}
	public boolean hasDownloading(Request r) {
		Helpers.assertMainTread();
		return queue.containsKey(r);
	}
	public void setListener(DownloadListener listener) {
		this.listener = listener;
	}
	public void registerListener(DownloadListener listener) {
		Helpers.assertMainTread();
		synchronized (listeners) {
			listeners.remove(listener);
			listeners.addFirst(listener);
		}
//android.util.Log.e("shdd", "Downloader.registerListener(): " + listener + "; " + listeners);
	}
	public void unregisterListener(DownloadListener listener) {
		Helpers.assertMainTread();
		synchronized (listeners) {
			listeners.remove(listener);
		}
//android.util.Log.e("shdd", "Downloader.unregisterListener(): " + listener + "; " + listeners);
	}
	private LinkedList<DownloadListener> copyListeners() {
		synchronized (listeners) {
			LinkedList<DownloadListener> listenersCopy = new LinkedList<DownloadListener>(listeners);
			if (listener != null)
				listenersCopy.addFirst(listener);
			return listenersCopy;
		}
	}
	public LinkedList<DownloadListener> getListeners() {
		Helpers.assertMainTread();
		return copyListeners();
	}
	public void setRawListener(DownloadRawListener rawListener) {
		this.rawListener = rawListener;
	}
	
	private void updateLock() {
//android.util.Log.e("shdd", "Downloader.updateLock(): queue.isEmpty(): " + queue.isEmpty() + "; wakeLock == null: " + (wakeLock == null));
		if (queue.isEmpty()) {
			if (wakeLock == null) return;
//android.util.Log.e("shdd", "Downloader.updateLock(): stop service and release lock");
			ctx.stopService(new Intent(ctx, DownloadService.class));
			wakeLock.release();
            wakeLock = null;
		} else {
			if (wakeLock != null) return;
//android.util.Log.e("shdd", "Downloader.updateLock(): start service and acquire lock");
			ctx.startService(new Intent(ctx, DownloadService.class));
			PowerManager pm = (PowerManager)ctx.getSystemService(Context.POWER_SERVICE);
			wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, ctx.getPackageName() + " HTTP DOWNLOADER");
			wakeLock.acquire();
		}
	}
	
	void onComplete(final DownloadTask task, final DownloadInfo info) {
		handler.postDelayed(new Runnable() {
			public void run() {
//android.util.Log.e("shdd", task.hashCode() + " Downloader.onComplete(): task.isInterrupted(): " + task.isInterrupted() + "; " + info.status + "; " + info.currentBytes + "/" + info.request.fileSize);
				if (task.isInterrupted()) return;
				queue.remove(info.request);
				updateLock();
				notifyDownloadChanged(task, info, DownloadListener.Type.STATE);
			}
		}, 200); //small timeout for good ui that user can see exactly 100% of download progress
	}
	void notifyDownloadChanged(final DownloadTask task, final DownloadInfo info, final DownloadListener.Type type) {
		if (info.copyOf == null) throw new IllegalArgumentException();
		final ConcurrentLinkedQueue<DownloadListener> callbacks = new ConcurrentLinkedQueue<>(copyListeners());
		handler.post(new Runnable() {
			public void run() {
//android.util.Log.e("shdd", task.hashCode() + " Downloader.notifyDownloadChanged(): task.isInterrupted(): " + task.isInterrupted() + "; " + type + "; " + info.status + "; " + info.currentBytes + "/" + info.request.fileSize);
				if (task.isInterrupted()) return;
				if (info.request.showNotification)
					notifier.process(info, type);
				for (DownloadListener listener : callbacks)
					listener.onDownloadChanged(info, type);
			}
		});
	}
	void notifyDownloadRaw(DownloadTask task, final DownloadInfo info, final byte[] raw, final int size) {
		if (info.copyOf == null) throw new IllegalArgumentException();
		if (rawListener == null) return;
		rawListener.onDownloadRaw(info, raw, size);
	}

}
