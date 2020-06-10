package com.paragon_software.dictionary_manager;

import android.os.Handler;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.download_manager.DownloadManager;
import com.paragon_software.dictionary_manager.download_manager.DownloadTaskId;
import com.paragon_software.dictionary_manager.download_manager.DownloadTaskStatus;
import com.paragon_software.dictionary_manager.download_manager.HttpUrlDownloadSource;
import com.paragon_software.dictionary_manager.download_manager.IDownloadDestination;
import com.paragon_software.dictionary_manager.download_manager.exceptions.BadUrlException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadManagerException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.NoSuchDownloadTaskException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.WriteToDestinationException;
import com.paragon_software.dictionary_manager.exceptions.DownloadDictionaryException;
import com.paragon_software.dictionary_manager.local_dictionary_library.IDictionaryWriter;
import com.paragon_software.dictionary_manager.local_dictionary_library.ILocalDictionaryLibrary;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.WriteToDictionaryLibraryException;

import java.lang.ref.WeakReference;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.DownloadFailureReason;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.IComponentDownloadProgressObserver;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.IOnDownloadFailureObserver;


/**
 * @deprecated use {@link HttpDownloaderLibController} instead.
 */
@Deprecated
public class DownloadController implements IDownloadController
{
  private List< WeakComponentProgressObserver > componentProgressObservers = new ArrayList<>();
  private List< IOnDownloadFailureObserver >    downloadFailureObservers   = new ArrayList<>();
  private DownloadManager            mDownloadManager;
  private DictionaryLocationResolver mComponentLocationResolver;
  private DictionaryManagerAPI       mDictionaryManager;
  private Map< DictionaryComponent, DownloadTaskId > downloadTasks = new HashMap<>();

  @SuppressWarnings( "WeakerAccess" )
  public DownloadController( DownloadManager downloadManager, DictionaryLocationResolver locationResolver,
                             DictionaryManagerAPI dictionaryManager )
  {
    this.mDownloadManager = downloadManager;
    this.mComponentLocationResolver = locationResolver;
    this.mDictionaryManager = dictionaryManager;
  }

  @Override
  public void download( final DictionaryComponent component, Dictionary dictionary )
  {
    Handler handlerForNotify = new Handler();
    try
    {
      ILocalDictionaryLibrary mDictionaryLibrary = mComponentLocationResolver.getDictionaryLibrary();
      final String componentFileName = mComponentLocationResolver.getComponentFileName(component);
      if ( mDictionaryLibrary.isDictionaryPresent(componentFileName) )
      {
        throw new DownloadDictionaryException("already exist");
      }
      HttpUrlDownloadSource source = getDownloadSourceForDictionary(component);
      IDownloadDestination destination =
          downloadDestinationFromIDictionaryWriter(mDictionaryLibrary.getDictionaryWriter(componentFileName, true));
      DownloadListenerInternal downloadStatus = new DownloadListenerInternal(component, handlerForNotify);
      DownloadTaskId taskId = mDownloadManager.downloadNow(source, destination, downloadStatus);
      downloadTasks.put(component, taskId);
    }
    catch ( DownloadDictionaryException e )
    {
      e.printStackTrace();
    }
    catch ( BadUrlException e )
    {
      e.printStackTrace();
    }
    catch ( WriteToDictionaryLibraryException e )
    {
      e.printStackTrace();
    }
  }

  @NonNull
  private HttpUrlDownloadSource getDownloadSourceForDictionary( @NonNull final DictionaryComponent component )
      throws BadUrlException
  {
    return new HttpUrlDownloadSource(component.getUrl(), mDownloadManager.getUserAgent());
  }

  @NonNull
  private IDownloadDestination downloadDestinationFromIDictionaryWriter(
      @NonNull final IDictionaryWriter dictionaryWriter )
  {
    return new IDownloadDestination()
    {
      @Override
      public void create() throws WriteToDestinationException
      {
        //do nothing
      }

      @Override
      public void write( byte[] data, int size ) throws WriteToDestinationException
      {
        try
        {
          dictionaryWriter.write(data, size);
        }
        catch ( WriteToDictionaryLibraryException e )
        {
          throw new WriteToDestinationException(e);
        }
      }

      @Override
      public void remove() throws WriteToDestinationException
      {
        try
        {
          dictionaryWriter.remove();
        }
        catch ( WriteToDictionaryLibraryException e )
        {
          throw new WriteToDestinationException(e);
        }
      }

      @Override
      public void finish() throws WriteToDestinationException
      {
        try
        {
          dictionaryWriter.finish();
        }
        catch ( WriteToDictionaryLibraryException e )
        {
          throw new WriteToDestinationException(e);
        }
      }
    };
  }

  @Override
  public boolean isDownloaded( DictionaryComponent component )
  {
    return mComponentLocationResolver.isDownloaded(component);
  }

  @Override
  @SuppressWarnings( "SimplifiableIfStatement" )
  public boolean isInProgress( DictionaryComponent component )
  {
    DownloadTaskStatus downloadTaskStatus = getDownloadTaskStatus(component);
    if ( downloadTaskStatus == null )
    {
      return false;
    }
    return DownloadTaskStatus.DOWNLOAD_TASK_STATUS.IN_PROCESS == downloadTaskStatus.getStatus();
  }

  @Nullable
  private DownloadTaskStatus getDownloadTaskStatus( DictionaryComponent component )
  {
    DownloadTaskStatus downloadTaskStatus = null;
    try
    {
      DownloadTaskId taskId = getTaskIdForComponent(component);
      if ( null != taskId )
      {
        downloadTaskStatus = mDownloadManager.getDownloadTaskStatus(taskId);
      }
    }
    catch ( NoSuchDownloadTaskException e )
    {
      removeTaskIdForComponent(component);
      return null;
    }
    return downloadTaskStatus;
  }

  private DownloadTaskId getTaskIdForComponent( DictionaryComponent component )
  {
    return downloadTasks.get(component);
  }

  private void removeTaskIdForComponent( DictionaryComponent component )
  {
    downloadTasks.remove(component);
  }

  @Override
  public long getTransferredBytes( DictionaryComponent component )
  {
    DownloadTaskStatus downloadTaskStatus = getDownloadTaskStatus(component);
    if ( downloadTaskStatus == null )
    {
      return 0;
    }
    return downloadTaskStatus.getCurrentSize();
  }

  @Override
  public long getSizeBytes( DictionaryComponent component )
  {
    DownloadTaskStatus downloadTaskStatus = getDownloadTaskStatus(component);
    if ( downloadTaskStatus == null )
    {
      return 0;
    }
    return downloadTaskStatus.getTotalSize();
  }

  @Override
  public void unregisterComponentProgressObserver( @NonNull IComponentDownloadProgressObserver observer )
  {
    for ( WeakComponentProgressObserver cpo : new ArrayList<>(componentProgressObservers) )
    {
      if ( observer.equals(cpo.getObserver().get()) )
      {
        componentProgressObservers.remove(cpo);
      }
    }
  }

  @Override
  public void registerComponentProgressObserver( @NonNull DictionaryComponent component,
                                                 @NonNull IComponentDownloadProgressObserver observer )
  {
    componentProgressObservers.add(new WeakComponentProgressObserver(component, observer));
  }

  @Override
  public void registerDownloadFailureObserver( IOnDownloadFailureObserver observer )
  {
    downloadFailureObservers.add(observer);
  }

  @Override
  public void unregisterDownloadFailureObserver( IOnDownloadFailureObserver observer )
  {
    downloadFailureObservers.remove(observer);
  }

  private void notifyDownloadFailureObservers( DownloadFailureReason reason )
  {
    for ( IOnDownloadFailureObserver observer : new ArrayList<>(downloadFailureObservers) )
    {
      observer.onFailed(reason);
    }
  }

  private void notifyComponentStatusChanged( @NonNull DictionaryComponent component )
  {
    for ( WeakComponentProgressObserver cpo : new ArrayList<>(componentProgressObservers) )
    {
      if ( component.equals(cpo.getComponent()) )
      {
        IComponentDownloadProgressObserver observer = cpo.getObserver().get();
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
    for ( WeakComponentProgressObserver cpo : new ArrayList<>(componentProgressObservers) )
    {
      if ( component.equals(cpo.getComponent()) )
      {
        IComponentDownloadProgressObserver observer = cpo.getObserver().get();
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

  @Override
  public void remove( @NonNull final DictionaryComponent component )
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
  public boolean isAnyDictionaryDownloaded() {
    // Unsupported yet
    return false;
  }

  @Override
  public void removeAll() {
    // Unsupported yet
  }

  @Override
  public void pauseDownload( DictionaryComponent component, Dictionary dictionary )
  {
    // Unsupported yet
  }

  static class WeakComponentProgressObserver
  {
    private DictionaryComponent                                 component;
    private WeakReference< IComponentDownloadProgressObserver > observer;

    WeakComponentProgressObserver( DictionaryComponent component, IComponentDownloadProgressObserver observer )
    {
      this.component = component;
      this.observer = new WeakReference<>(observer);
    }

    DictionaryComponent getComponent()
    {
      return component;
    }

    WeakReference< IComponentDownloadProgressObserver > getObserver()
    {
      return observer;
    }
  }

  private class DownloadListenerInternal implements DownloadManager.IDownloadStatus
  {
    @NonNull
    private DictionaryComponent component;
    private Handler             handler;

    DownloadListenerInternal( @NonNull DictionaryComponent component, Handler handler )
    {
      this.handler = handler;
      this.component = component;
    }

    @Override
    public void onProgress( long currentSize, long totalSize )
    {
      handler.post(new Runnable()
      {
        @Override
        public void run()
        {
          notifyComponentProgressObservers(component);
        }
      });
    }

    @Override
    public void onFinished()
    {
      handler.post(new Runnable()
      {
        @Override
        public void run()
        {
          mDictionaryManager.updateDictionariesInBackground();
        }
      });
    }

    @Override
    public void onDownloadStatusChanged()
    {
      handler.post(new Runnable()
      {
        @Override
        public void run()
        {
          notifyComponentStatusChanged(component);
        }
      });
    }

    @Override
    public void onFailed( @NonNull final DownloadManagerException reason )
    {
      handler.post(new Runnable()
      {
        @Override
        public void run()
        {
          Throwable cause = reason.getCause();
          if ( UnknownHostException.class.isInstance(cause) )
          {
            notifyDownloadFailureObservers(DownloadFailureReason.CONNECTION_UNAVAILABLE);
          }
          else if ( SocketTimeoutException.class.isInstance(cause) )
          {
            notifyDownloadFailureObservers(DownloadFailureReason.CONNECTION_LOST);
          }
          reason.printStackTrace();
        }
      });
    }
  }
}