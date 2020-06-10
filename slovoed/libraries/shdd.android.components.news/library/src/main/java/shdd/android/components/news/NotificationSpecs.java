package shdd.android.components.news;

import android.app.NotificationChannel;
import android.app.PendingIntent;
import android.os.Bundle;

import androidx.annotation.RequiresApi;

public interface NotificationSpecs {
    @RequiresApi(26)
    NotificationChannel getChannel(AdsManager.AdsType adsType);

    String getChannelId(AdsManager.AdsType adsType);

    /**
     * Use icon 24x24dip
     */
    int getNotificationIcon();

    /**
     * Use launcher icon. Internally scale to 64x64dip
     */
    int getLargeNotificationIcon();

    PendingIntent createPendingIntent(AdsManager.AdsType adsType, Bundle bundle);
}
