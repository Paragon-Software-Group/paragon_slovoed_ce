package com.paragon_software.dictionary_manager.downloader;

import android.app.PendingIntent;

public interface IDownloadNotification
{
  int getSmallIcon( DownloadLibraryBuilder.DownloadStatus status );

  PendingIntent createPendingIntent( DownloadInfoWrapper infoWrapper );
}
