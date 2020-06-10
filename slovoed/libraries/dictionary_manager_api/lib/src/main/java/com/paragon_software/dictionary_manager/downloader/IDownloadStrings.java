package com.paragon_software.dictionary_manager.downloader;

import android.content.res.Resources;
import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.downloader.DownloadLibraryBuilder.StringRes;

public interface IDownloadStrings
{
  String getString( @NonNull DownloadInfoWrapper infoWrapper, @NonNull StringRes stringRes, @NonNull Resources res);

  CharSequence getNotificationChannelName(@NonNull Resources res);
}
