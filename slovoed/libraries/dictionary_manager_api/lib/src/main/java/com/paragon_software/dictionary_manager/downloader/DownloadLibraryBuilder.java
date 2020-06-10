package com.paragon_software.dictionary_manager.downloader;

import android.app.NotificationChannel;
import android.app.PendingIntent;
import android.content.Context;
import android.graphics.Bitmap;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

public class DownloadLibraryBuilder
{
  public static final String META_DICTIONARY = "META_DICTIONARY";
  public static final String META_DICTIONARY_COMPONENT = "META_DICTIONARY_COMPONENT";

  private Context               appContext;
  private NotificationChannel   notificationChannel;
  private String                channelId;
  private Bitmap                largeIcon;
  private IDownloadNotification downloadNotification;
  private IDownloadStrings      downloadStrings;

  public Context getAppContext()
  {
    return appContext;
  }

  public void setAppContext( @NonNull Context appContext )
  {
    this.appContext = appContext;
  }

  @RequiresApi(api = Build.VERSION_CODES.O)
  public NotificationChannel getNotificationChannel()
  {
    return notificationChannel;
  }

  @RequiresApi(api = Build.VERSION_CODES.O)
  public void setNotificationChannel( @NonNull NotificationChannel notificationChannel )
  {
    this.notificationChannel = notificationChannel;
  }

  public String getChannelId()
  {
    return channelId;
  }

  public void setChannelId( @NonNull String channelId )
  {
    this.channelId = channelId;
  }

  public Bitmap getLargeIcon()
  {
    return largeIcon;
  }

  public void setLargeIcon( @NonNull Bitmap largeIcon )
  {
    this.largeIcon = largeIcon;
  }

  public int getSmallIcon( @NonNull DownloadStatus status )
  {
    return downloadNotification.getSmallIcon(status);
  }

  public PendingIntent createPendingIntent( DownloadInfoWrapper infoWrapper )
  {
    return downloadNotification.createPendingIntent(infoWrapper);
  }

  public void setDownloadNotification( @NonNull IDownloadNotification downloadNotification )
  {
    this.downloadNotification = downloadNotification;
  }

  public String getString( @NonNull DownloadInfoWrapper infoWrapper, @NonNull StringRes stringRes )
  {
    if (appContext == null) throw new IllegalStateException("Context must be initialized at this stage.");
    return downloadStrings.getString(infoWrapper, stringRes, appContext.getResources());
  }

  public void setDownloadStrings( IDownloadStrings downloadStrings )
  {
    this.downloadStrings = downloadStrings;
  }

  public enum DownloadStatus
  {
    CONNECTING,
    DOWNLOADING,
    SUCCESSFULL,
    DEFAULT
  }

  public enum StringRes
  {
    TITLE,
    TEXT,
    SUB_TEXT,
    SIZE,
    REMAINING,
    SPEED;
  }
}
