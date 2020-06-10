package com.paragon_software.dictionary_manager.http_downloader_library;

import android.app.NotificationChannel;
import android.app.PendingIntent;
import android.graphics.Bitmap;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.paragon_software.dictionary_manager.downloader.DownloadInfoWrapper;
import com.paragon_software.dictionary_manager.downloader.DownloadLibraryBuilder;
import shdd.android.components.httpdownloader.DownloadClientHelper;
import shdd.android.components.httpdownloader.DownloadInfo;
import shdd.android.components.httpdownloader.DownloadListener;
import shdd.android.components.httpdownloader.NotificationSpecs;

//@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
public class DownloaderProvider implements DownloadClientHelper
{
  private DownloadLibraryBuilder builder;

  public DownloaderProvider( DownloadLibraryBuilder builder )
  {
    this.builder = builder;
  }

  @Override
  public NotificationSpecs getNotificationSpecs()
  {
    return new NotificationSpecs()
    {
      @RequiresApi(api = Build.VERSION_CODES.O)
      @Override
      public NotificationChannel getChannel()
      {
        return builder.getNotificationChannel();
      }

      @Override
      public String getChannelId()
      {
        return builder.getChannelId();
      }

      @Override
      public Bitmap getLargeIcon( DownloadInfo downloadInfo )
      {
        return builder.getLargeIcon();
      }

      @Override
      public int getSmallIcon( DownloadInfo downloadInfo )
      {
        DownloadLibraryBuilder.DownloadStatus status = convertDownloadStatus(downloadInfo);
        return builder.getSmallIcon(status);
      }

      @Override
      public PendingIntent createPendingIntent( DownloadInfo info, DownloadListener.Type type )
      {
        DownloadInfoWrapper infoWrapper = createInfoWrapper(info);
        return builder.createPendingIntent(infoWrapper);
      }
    };
  }

  @NonNull
  public static DownloadInfoWrapper createInfoWrapper( DownloadInfo info )
  {
    return new DownloadInfoWrapper(info.url, convertDownloadStatus(info), info.request.meta, info.currentBytes, info.request.fileSize, info.speed);
  }

  @NonNull
  static DownloadLibraryBuilder.DownloadStatus convertDownloadStatus( DownloadInfo downloadInfo )
  {
    DownloadLibraryBuilder.DownloadStatus status;
    switch ( downloadInfo.status )
    {
      case CONNECTING:
        status = DownloadLibraryBuilder.DownloadStatus.CONNECTING;
        break;
      case DOWNLOADING:
        status = DownloadLibraryBuilder.DownloadStatus.DOWNLOADING;
        break;
      case SUCCESSFULL:
        status = DownloadLibraryBuilder.DownloadStatus.SUCCESSFULL;
        break;
      default:
        status = DownloadLibraryBuilder.DownloadStatus.DEFAULT;
        break;
    }
    return status;
  }

  @Override
  public String getString( DownloadInfo info, STRINGS strings )
  {
    DownloadLibraryBuilder.StringRes stringRes = convertStringRes(strings);
    if (null == stringRes)
    {
      return "";
    }
    DownloadInfoWrapper infoWrapper = createInfoWrapper(info);
    return builder.getString(infoWrapper, stringRes);
  }

  private DownloadLibraryBuilder.StringRes convertStringRes( STRINGS strings )
  {
    switch ( strings )
    {
      case TITLE:
        return DownloadLibraryBuilder.StringRes.TITLE;
      case TEXT:
        return DownloadLibraryBuilder.StringRes.TEXT;
      case SUB_TEXT:
        return DownloadLibraryBuilder.StringRes.SUB_TEXT;
      case SIZE:
        return DownloadLibraryBuilder.StringRes.SIZE;
      case REMAINING:
        return DownloadLibraryBuilder.StringRes.REMAINING;
      case SPEED:
        return DownloadLibraryBuilder.StringRes.SPEED;
    }
    return DownloadLibraryBuilder.StringRes.TITLE;
  }

  @Override
  public int getLayout( DownloadInfo downloadInfo, LAYOUT layout )
  {
    // UNUSED for API >= 17
    return 0;
  }

  @Override
  public int getId( ID id )
  {
    // UNUSED for API >= 17
    return 0;
  }
}
