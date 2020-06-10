package com.paragon_software.dictionary_manager_ui;

import android.content.res.Resources;
import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.components.PictureComponent;
import com.paragon_software.dictionary_manager.components.SoundComponent;
import com.paragon_software.dictionary_manager.downloader.DictionaryMeta;
import com.paragon_software.dictionary_manager.downloader.DownloadInfoWrapper;
import com.paragon_software.dictionary_manager.downloader.DownloadLibraryBuilder;
import com.paragon_software.dictionary_manager.downloader.IDownloadStrings;

public class DownloadStrings implements IDownloadStrings
{

  @Override
  public String getString( @NonNull DownloadInfoWrapper infoWrapper,
                           @NonNull DownloadLibraryBuilder.StringRes stringRes,
                           @NonNull Resources res)
  {
    DictionaryMeta dictionaryMeta = DictionaryMeta.class
        .cast(infoWrapper.getMeta().get(DownloadLibraryBuilder.META_DICTIONARY));
    DictionaryComponent component =
        DictionaryComponent.class.cast(infoWrapper.getMeta().get(DownloadLibraryBuilder.META_DICTIONARY_COMPONENT));

    switch ( stringRes )
    {
      case TITLE:
        switch ( infoWrapper.getDownloadStatus() )
        {
          case DOWNLOADING:
          case CONNECTING:
            return res.getString(R.string.dictionary_manager_ui_downloading);
          case SUCCESSFULL:
            return res.getString(R.string.dictionary_manager_ui_download_success);
          default:
            return res.getString(R.string.dictionary_manager_ui_download_error);
        }
      case TEXT:
        switch ( component.getType() )
        {
          case SOUND:
            if ( component instanceof SoundComponent )
            {
              return res.getString(R.string.dictionary_manager_ui_my_dictionaries_download_sound_base,
                      ( (SoundComponent) component ).getName()).toUpperCase();
            }
          case PICT:
            if ( component instanceof PictureComponent )
            {
              return res.getString(R.string.dictionary_manager_ui_my_dictionaries_download_picture_base).toUpperCase();
            }
          case WORD_BASE:
            return dictionaryMeta.getTitle().get();
          default:
            throw new IllegalArgumentException();
        }
      case SUB_TEXT:
        return null;
      case SIZE:
        return res.getString(R.string.dictionary_manager_ui_download_size_of, Utils.formatSize(infoWrapper.getCurrentBytes()),
                             Utils.formatSize(infoWrapper.getFileSize()));
      case REMAINING:
        return res.getString(R.string.dictionary_manager_ui_download_remaining, Utils.formatDuration(
            ( ( infoWrapper.getFileSize() - infoWrapper.getCurrentBytes() ) * 1000 ) / infoWrapper.getSpeed()));
      case SPEED:
        return Utils.formatSize(infoWrapper.getSpeed()) + "/s";
      default:
        throw new IllegalArgumentException();
    }
  }

  @Override
  public CharSequence getNotificationChannelName(@NonNull Resources res)
  {
    return res.getString(R.string.utils_slovoed_ui_common_download);
  }
}
