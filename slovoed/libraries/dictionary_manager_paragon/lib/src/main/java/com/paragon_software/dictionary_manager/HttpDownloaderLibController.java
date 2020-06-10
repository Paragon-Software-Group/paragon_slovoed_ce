package com.paragon_software.dictionary_manager;

import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI.DownloadFailureReason;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.download_manager.HttpHeaders;
import com.paragon_software.dictionary_manager.downloader.DictionaryMeta;
import com.paragon_software.dictionary_manager.downloader.DownloadInfoWrapper;
import com.paragon_software.dictionary_manager.downloader.DownloadLibraryBuilder;
import com.paragon_software.dictionary_manager.exceptions.DownloadDictionaryException;
import com.paragon_software.dictionary_manager.file_operations.DefaultFileOperations;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.http_downloader_library.DownloaderProvider;
import com.paragon_software.dictionary_manager.local_dictionary_library.ILocalDictionaryLibrary;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import shdd.android.components.httpdownloader.DownloadInfo;
import shdd.android.components.httpdownloader.DownloadListener;
import shdd.android.components.httpdownloader.Downloader;
import shdd.android.components.httpdownloader.Request;

import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.DownloadFailureReason.COMMON_ERROR;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.DownloadFailureReason.CONNECTION_LOST;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.DownloadFailureReason.CONNECTION_UNAVAILABLE;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.DownloadFailureReason.FILESYSTEM_ERROR;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.DownloadFailureReason.FILE_CORRUPTED;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.DownloadFailureReason.STORAGE_INSUFFICIENT_SPACE;

class HttpDownloaderLibController implements IDownloadController
{
  // Download to temporary file. On success - DictionaryLocationResolver.renameTmpToDownloadedComponent
  private static final String TMP_FILE_SUFFIX = ".tmp";
  @Nullable
  private final Downloader                 mDownloader;
  private final DictionaryLocationResolver mComponentLocationResolver;
  private final DictionaryManagerAPI       mDictionaryManager;
  private final DownloadLibraryBuilder     mBuilder;
  private final Map< DictionaryComponent, DownloadInfoWrapper >          mDownloadProgress          = new HashMap<>();
  private       List< DownloadController.WeakComponentProgressObserver > componentProgressObservers = new ArrayList<>();
  private       List< DictionaryManagerAPI.IOnDownloadFailureObserver >  downloadFailureObservers   = new ArrayList<>();

  HttpDownloaderLibController( DictionaryLocationResolver locationResolver, DictionaryManagerAPI dictionaryManager )
  {
    this.mComponentLocationResolver = locationResolver;
    this.mDictionaryManager = dictionaryManager;
    mBuilder = dictionaryManager.getDownloadLibraryBuilder();
    if ( null != mBuilder )
    {
      this.mDownloader = new Downloader(mBuilder.getAppContext(), new DownloaderProvider(mBuilder));
      this.mDownloader.setListener(new DownloadListener()
      {
        @Override
        public void onDownloadChanged( DownloadInfo downloadInfo, Type type )
        {
          DictionaryComponent component =
              (DictionaryComponent) downloadInfo.request.meta.get(DownloadLibraryBuilder.META_DICTIONARY_COMPONENT);
          if ( Type.STATE.equals(type) )
          {
            switch ( downloadInfo.status )
            {
              case PAUSED:
              case CANCELED:
              case CONNECTING:
              case DOWNLOADING:
                notifyComponentStatusChanged(component);
                break;
              case SUCCESSFULL:
                String tmpFilename = downloadInfo.request.destFilename;
                onSuccessfullyDownloaded(component, tmpFilename);
                break;
              case STORAGE_INSUFFICIENT_SPACE:
                notifyDownloadFailureObservers(DownloadFailureReason.STORAGE_INSUFFICIENT_SPACE);
                break;
              case NETWORK_UNAVAILABLE:
              case HTTP_DATA_ERROR:
                notifyDownloadFailureObservers(CONNECTION_UNAVAILABLE);
                break;
              case HTTP_CODE_500_INTERNAL_ERROR:
              case HTTP_CODE_503_UNAVAILABLE:
              case HTTP_CODE_from_300_to_400_UNHANDLED_REDIRECT:
              case HTTP_CODE_from_400_to_600_UNHANDLED_HTTP_OR_SERVER:
              case HTTP_CODE_all_UNHANDLED_OTHER:
              case HTTP_CANNOT_RESUME:
              case TOO_MANY_REDIRECTS:
                notifyDownloadFailureObservers(CONNECTION_LOST);
                break;
              case FILE_ERROR:
                notifyDownloadFailureObservers(FILESYSTEM_ERROR);
                break;
              case UNKNOWN_ERROR:
                notifyDownloadFailureObservers(COMMON_ERROR);
                break;
            }
          }
          else if ( Type.PROGRESS.equals(type) )
          {
            mDownloadProgress.put(component, DownloaderProvider.createInfoWrapper(downloadInfo));
            notifyComponentProgressObservers(component);
          }
        }
      });
    }
    else
    {
      this.mDownloader = null;
    }
  }

  private void onSuccessfullyDownloaded( DictionaryComponent component, String tmpFilename )
  {
    if (mComponentLocationResolver.renameTmpToDownloadedComponent(tmpFilename, component))
    {
      notifyComponentProgressObservers(component);
      mComponentLocationResolver.rescanDictionaries();
      notifyComponentStatusChanged(component);
      mDictionaryManager.updateDictionariesInBackground();
    }
    else
    {
      notifyDownloadFailureObservers(DownloadFailureReason.FILESYSTEM_ERROR);
    }
  }

  private void notifyComponentStatusChanged( @NonNull DictionaryComponent component )
  {
    for ( DownloadController.WeakComponentProgressObserver cpo : new ArrayList<>(componentProgressObservers) )
    {
      if ( component.equals(cpo.getComponent()) )
      {
        DictionaryManagerAPI.IComponentDownloadProgressObserver observer = cpo.getObserver().get();
        if ( null == observer )
        {
          componentProgressObservers.remove(cpo);
        }
        else
        {
          observer.onStatusChanged();
        }
      }
    }
  }

  private void notifyComponentProgressObservers( @NonNull DictionaryComponent component )
  {
    for ( DownloadController.WeakComponentProgressObserver cpo : new ArrayList<>(componentProgressObservers) )
    {
      if ( component.equals(cpo.getComponent()) )
      {
        DictionaryManagerAPI.IComponentDownloadProgressObserver observer = cpo.getObserver().get();
        if ( null == observer )
        {
          componentProgressObservers.remove(cpo);
        }
        else
        {
          observer.onProgressChanged();
        }
      }
    }
  }

  private void notifyDownloadFailureObservers( DownloadFailureReason reason )
  {
    for ( DictionaryManagerAPI.IOnDownloadFailureObserver observer : new ArrayList<>(downloadFailureObservers) )
    {
      observer.onFailed(reason);
    }
  }

  @Override
  public void download( DictionaryComponent component, Dictionary dictionary )
  {
    if ( null == mDownloader || null == mBuilder )
    {
      return;
    }

    if ( !Utils.isNetworkAvailable(mBuilder.getAppContext()) )
    {
      notifyDownloadFailureObservers(CONNECTION_UNAVAILABLE);
      return;
    }

    try
    {
      mDownloadProgress.remove(component);
      ILocalDictionaryLibrary mDictionaryLibrary = mComponentLocationResolver.getDictionaryLibrary();
      final String componentFileName = mComponentLocationResolver.getComponentFileName(component);
      if ( mDictionaryLibrary.isDictionaryPresent(componentFileName) )
      {
        throw new DownloadDictionaryException("already exist");
      }

      String path = mDictionaryLibrary.getDirectoryPath();
      File storePath = new File(path);
      if ( !storePath.exists() )
      {
        //noinspection ResultOfMethodCallIgnored
        storePath.mkdirs();
      }

      if ( !shdd.android.components.httpdownloader.Utils
          .hasEnoughFreeSpaceInFileSystemFor(storePath, component.getSize()) )
      {
        notifyDownloadFailureObservers(STORAGE_INSUFFICIENT_SPACE);
        return;
      }

      String tmpFileName = componentFileName + TMP_FILE_SUFFIX;
      long alreadyDownloadedBytes = getAlreadyDownloadedBytes(storePath, tmpFileName);
      if ( alreadyDownloadedBytes == component.getSize() )
      {
        onSuccessfullyDownloaded(component, tmpFileName);
        return;
      }
      else if ( alreadyDownloadedBytes > component.getSize() )
      {
        removeTmpFile(storePath, tmpFileName);
        notifyDownloadFailureObservers(FILE_CORRUPTED);
        return;
      }

      Request r = new Request();
      r.meta.put(DownloadLibraryBuilder.META_DICTIONARY, new DictionaryMeta(dictionary));
      r.meta.put(DownloadLibraryBuilder.META_DICTIONARY_COMPONENT, component);
      r.url = component.getUrl();
      r.destDir = storePath;
      r.destFilename = tmpFileName;
      r.fileSize = component.getSize();
      r.noIntegrity = true;
      r.showNotification = !component.isDemo();
      r.headers = new LinkedList<>();
      r.headers.add(new Pair<>(HttpHeaders.USER_AGENT_HEADER_NAME,
                               HttpHeaders.getUserAgentHeaderValue(mBuilder.getAppContext())));
      mDownloader.add(r);

    }
    catch ( DownloadDictionaryException e )
    {
      e.printStackTrace();
    }
  }

  private void removeTmpFile( @NonNull File storePath, @NonNull String tmpFileName )
  {
    if ( !storePath.exists() )
    {
      return;
    }

    File tmpFile = new File(storePath, tmpFileName);
    if ( tmpFile.exists() )
    {
      try
      {
        new DefaultFileOperations().delete(tmpFile.getAbsolutePath());
      }
      catch ( CantDeleteFileException ignored )
      {
      }
    }
  }

  private long getAlreadyDownloadedBytes( @NonNull File storePath, @NonNull String tmpFileName )
  {
    if ( !storePath.exists() )
    {
      return Long.MIN_VALUE;
    }

    File tmpFile = new File(storePath, tmpFileName);
    if ( tmpFile.exists() )
    {
      return tmpFile.length();
    }
    return Long.MIN_VALUE;
  }

  @Override
  public boolean isDownloaded( DictionaryComponent component )
  {
    return mComponentLocationResolver.isDownloaded(component);
  }

  @Override
  public boolean isInProgress( DictionaryComponent component )
  {
    return mDownloader != null && mDownloader.hasDownloading(Request.oneTime(component.getUrl()));
  }

  @Override
  public long getTransferredBytes( DictionaryComponent component )
  {
    DownloadInfoWrapper infoWrapper = mDownloadProgress.get(component);
    if (infoWrapper != null) {
      return infoWrapper.getCurrentBytes();
    } else {
//      Check bytes in file
      ILocalDictionaryLibrary mDictionaryLibrary = mComponentLocationResolver.getDictionaryLibrary();
      final String componentFileName = mComponentLocationResolver.getComponentFileName(component);
      String path = mDictionaryLibrary.getDirectoryPath();
      File storePath = new File(path);
      String tmpFileName = componentFileName + TMP_FILE_SUFFIX;
      long downloadedBytes = getAlreadyDownloadedBytes(storePath, tmpFileName);
      return downloadedBytes < 0 ? 0 : downloadedBytes;
    }
  }

  @Override
  public long getSizeBytes( DictionaryComponent component )
  {
    DownloadInfoWrapper infoWrapper = mDownloadProgress.get(component);
    return null != infoWrapper ? infoWrapper.getFileSize() : 0;
  }

  @Override
  public void unregisterComponentProgressObserver(
      @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer )
  {
    for ( DownloadController.WeakComponentProgressObserver cpo : new ArrayList<>(componentProgressObservers) )
    {
      if ( observer.equals(cpo.getObserver().get()) )
      {
        componentProgressObservers.remove(cpo);
      }
    }
  }

  @Override
  public void registerComponentProgressObserver( @NonNull DictionaryComponent component,
                                                 @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer )
  {
    componentProgressObservers.add(new DownloadController.WeakComponentProgressObserver(component, observer));
  }

  @Override
  public void registerDownloadFailureObserver( DictionaryManagerAPI.IOnDownloadFailureObserver observer )
  {
    downloadFailureObservers.add(observer);
  }

  @Override
  public void unregisterDownloadFailureObserver( DictionaryManagerAPI.IOnDownloadFailureObserver observer )
  {
    downloadFailureObservers.remove(observer);
  }

  @Override
  public void remove( @NonNull DictionaryComponent component )
  {
    try
    {
      mComponentLocationResolver.remove(component);
    }
    catch ( Exception ignore )
    {
    }
    notifyComponentStatusChanged(component);
    mDictionaryManager.updateDictionariesInBackground();
  }

  @Override
  public boolean isAnyDictionaryDownloaded()
  {
    for (Dictionary dictionary: mDictionaryManager.getDictionaries())
    {
      for (DictionaryComponent component: dictionary.getDictionaryComponents())
      {
        boolean downloaded = isDownloaded(component);
        if (!component.isDemo() && downloaded)
          return true;
      }
    }
    return false;
  }

  @Override
  public synchronized void removeAll()
  {
    for (Dictionary dictionary: mDictionaryManager.getDictionaries())
    {
      for (DictionaryComponent component: dictionary.getDictionaryComponents())
      {
        boolean downloaded = isDownloaded(component);
        if (!component.isDemo() && downloaded)
          remove(component);
      }
    }
  }

  @Override
  public void pauseDownload( DictionaryComponent component, Dictionary dictionary )
  {
    if ( mDownloader != null )
    {
      mDownloader.pause(Request.oneTime(component.getUrl()));
      notifyComponentStatusChanged(component);
      notifyComponentProgressObservers(component);
    }
  }
}
