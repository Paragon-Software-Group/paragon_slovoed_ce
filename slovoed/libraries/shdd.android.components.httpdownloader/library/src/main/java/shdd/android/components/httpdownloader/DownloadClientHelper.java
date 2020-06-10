package shdd.android.components.httpdownloader;

import android.app.PendingIntent;
import android.graphics.Bitmap;

public interface DownloadClientHelper {
	NotificationSpecs getNotificationSpecs();
	String getString(DownloadInfo info, STRINGS s);
	enum STRINGS {
		TITLE,
		TEXT,
		SUB_TEXT,
		SIZE,
		REMAINING,
		SPEED,
	}
	int getLayout(DownloadInfo info, LAYOUT layout);
	enum LAYOUT {
		NOTIFICATION,
	};
	int getId(ID id);
	enum ID {
		LARGE_ICON,
		TITLE,
		WHEN,
		PROGRESS,
		TEXT,
		SUB_TEXT,
		INFO,
		SMALL_ICON,
	};
	
}
