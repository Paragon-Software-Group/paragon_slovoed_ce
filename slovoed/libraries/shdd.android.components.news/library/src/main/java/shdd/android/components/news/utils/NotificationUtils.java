package shdd.android.components.news.utils;


import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.NotificationCompat;
import android.text.Html;

import java.util.HashSet;

import shdd.android.components.news.AdsManager;
import shdd.android.components.news.NotificationSpecs;
import shdd.android.components.news.Resources;

public class NotificationUtils {

  private static final String TAG_NEWS = "n";

  public static void createChannels( Resources.Getters getters ) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            return;
        }

        HashSet<NotificationChannel> channels = new HashSet<>();
        for (AdsManager.AdsType adsType : AdsManager.AdsType.values()) {
            NotificationSpecs ns = getters.getNotificationSpecs();
            channels.add(ns.getChannel(adsType));
        }
        NotificationManager manager = (NotificationManager) getters.getAppContext().getSystemService(Context.NOTIFICATION_SERVICE);
        for (NotificationChannel channel : channels) {
            if (manager != null) {
                manager.createNotificationChannel(channel);
            }
        }
    }

    public static void showNotification(Context ctx, AdsManager.AdsType adsType, String ticker, String text, String title, Bundle data) {
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
      {
        showNotificationSystem(ctx, adsType, ticker, text, title, data);
      }
      else
      {
        showNotificationCompat(ctx, adsType, ticker, text, title, data);
      }
    }

  private static void showNotificationSystem( Context ctx, AdsManager.AdsType adsType, String ticker, String text,
                                              String title, Bundle data )
  {
    ShddLog.d("shdd","showNotificationSystem() called");
    if ( Build.VERSION.SDK_INT >= Build.VERSION_CODES.O )
    {
      float scaleFactor = ctx.getResources().getDisplayMetrics().density;
      NotificationSpecs specs = Resources.getters.getNotificationSpecs();
      Bitmap scaledLargeIcon = getScaledLargeIcon(ctx, scaleFactor, specs);

      Notification.Builder builder = new Notification.Builder(ctx);
      builder.setChannelId(specs.getChannelId(adsType));
      builder.setSmallIcon(specs.getNotificationIcon()).setLargeIcon(scaledLargeIcon)
             .setWhen(System.currentTimeMillis()).setTicker(ticker).setContentText(Html.fromHtml(text))
             .setContentTitle(title);

      builder.setStyle(new Notification.BigTextStyle(builder).bigText(Html.fromHtml(text)));
      builder.setContentIntent(specs.createPendingIntent(adsType, data == null ? new Bundle() : data));
      Notification notification = builder.build();
      notification.flags = Notification.FLAG_AUTO_CANCEL;
      notification.icon = specs.getNotificationIcon();
      NotificationManager service = (NotificationManager) ctx.getSystemService(Context.NOTIFICATION_SERVICE);
      if (service != null) {
        service.notify(TAG_NEWS, adsType.name().hashCode(), notification);
      }
    }
  }

  private static void showNotificationCompat( Context ctx, AdsManager.AdsType adsType, String ticker, String text,
                                              String title, Bundle data ) {
        ShddLog.d("shdd","showNotificationCompat() called");
        float scaleFactor = ctx.getResources().getDisplayMetrics().density;
        NotificationSpecs specs = Resources.getters.getNotificationSpecs();
        Bitmap scaledLargeIcon = getScaledLargeIcon(ctx, scaleFactor, specs);
        NotificationCompat.Builder builder = new NotificationCompat.Builder(ctx);
        builder.setSmallIcon(specs.getNotificationIcon())
                .setLargeIcon(scaledLargeIcon)
                .setWhen(System.currentTimeMillis())
                .setTicker(ticker)
                .setContentText(Html.fromHtml(text))
                .setContentTitle(title);
        builder.setStyle(new NotificationCompat.BigTextStyle(builder)
                .bigText(Html.fromHtml(text)))
                .setContentIntent(specs.createPendingIntent(adsType, data == null ? new Bundle() : data));
        Notification notification = builder.build();
        notification.flags = Notification.FLAG_AUTO_CANCEL;
        notification.icon = specs.getNotificationIcon();
        NotificationManager service = (NotificationManager) ctx.getSystemService(Context.NOTIFICATION_SERVICE);
        if (service != null) {
            service.notify(TAG_NEWS, adsType.name().hashCode(), notification);
        }
    }

    private static Bitmap getScaledLargeIcon(Context ctx, float scaleFactor, NotificationSpecs specs) {
        return Bitmap.createScaledBitmap(BitmapFactory.decodeResource(ctx.getResources(), specs.getLargeNotificationIcon()), (int) (64 * scaleFactor), (int) (64 * scaleFactor), true);
    }
}
