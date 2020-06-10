package shdd.android.components.httpdownloader;

import android.app.NotificationChannel;
import android.app.PendingIntent;
import android.graphics.Bitmap;
import android.support.annotation.RequiresApi;

public interface NotificationSpecs {
    @RequiresApi(26)
    NotificationChannel getChannel();

    String getChannelId();

    Bitmap getLargeIcon(DownloadInfo info);

    int getSmallIcon(DownloadInfo info);

    PendingIntent createPendingIntent(DownloadInfo info, DownloadListener.Type type);
}