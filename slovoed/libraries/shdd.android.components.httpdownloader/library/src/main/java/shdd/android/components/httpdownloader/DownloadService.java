package shdd.android.components.httpdownloader;

import android.app.Notification;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class DownloadService extends Service {
	
	@Override
	public IBinder onBind(Intent intent) {
		throw new UnsupportedOperationException();
	}
	@Override
	public void onCreate() {
		if (android.os.Build.VERSION.SDK_INT <= android.os.Build.VERSION_CODES.JELLY_BEAN_MR1) {
			Notification note = new Notification(0, null, System.currentTimeMillis());
			note.flags |= Notification.FLAG_NO_CLEAR;
			startForeground(100, note);
		}
	}
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		return START_NOT_STICKY; //START_NOT_STICKY - cuz no need to recreate it after killing because it is a simple background service that does nothing.
	}
	@Override
	public void onDestroy() {
		stopForeground(true);
	}
	
}
