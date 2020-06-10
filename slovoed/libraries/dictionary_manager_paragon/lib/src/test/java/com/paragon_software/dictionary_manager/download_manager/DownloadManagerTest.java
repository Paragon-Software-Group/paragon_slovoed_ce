package com.paragon_software.dictionary_manager.download_manager;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadFailedException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadManagerException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.NoSuchCallbackException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.NoSuchDownloadTaskException;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.robolectric.RobolectricTestRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import static com.paragon_software.dictionary_manager.download_manager.DownloadTaskStatus.DOWNLOAD_TASK_STATUS;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.withSettings;

@RunWith( RobolectricTestRunner.class )
public class DownloadManagerTest
{
  @Rule
  public MockitoRule rule = MockitoJUnit.rule();

  @Test
  public void getDownloadTaskStatusForInvalidTaskId()
  {
    try
    {
      new DownloadManager().getDownloadTaskStatus(new DownloadTaskId(Long.MAX_VALUE));
      assertTrue("Dictionary manager return DownloadTaskStatus for unavailable TaskID", false);
    }
    catch ( NoSuchDownloadTaskException e )
    {
      assertTrue(true);
    }
  }

  @Test
  public void registerAndUnregisterForInvalidTaskId()
  {
    try
    {
      new DownloadManager().registerObserver(new DownloadTaskId(Long.MAX_VALUE), Utils.newEmptyDownloadStatusObserver());
      assertTrue("Dictionary manager register observer for unavailable TaskID", false);
    }
    catch ( NoSuchCallbackException e )
    {
      assertTrue(true);
    }
    try
    {
      new DownloadManager().unregisterObserver(new DownloadTaskId(Long.MAX_VALUE), Utils.newEmptyDownloadStatusObserver());
      assertTrue("Dictionary manager unregister observer for unavailable TaskID", false);
    }
    catch ( NoSuchCallbackException e )
    {
      assertTrue(true);
    }
  }

  @Test
  public void cancelAndPauseAndResumeForInvalidTaskId()
  {
    try
    {
      new DownloadManager().cancelDownloadTask(new DownloadTaskId(Long.MAX_VALUE));
      assertTrue("Dictionary manager cancel download task for unavailable TaskID", false);
    }
    catch ( NoSuchDownloadTaskException e )
    {
      assertTrue(true);
    }

    try
    {
      new DownloadManager().pauseDownloadTask(new DownloadTaskId(Long.MAX_VALUE));
      assertTrue("Dictionary manager pause download task for unavailable TaskID", false);
    }
    catch ( NoSuchDownloadTaskException e )
    {
      assertTrue(true);
    }

    try
    {
      new DownloadManager().resumeDownloadTask(new DownloadTaskId(Long.MAX_VALUE));
      assertTrue("Dictionary manager resume download task for unavailable TaskID", false);
    }
    catch ( NoSuchDownloadTaskException e )
    {
      assertTrue(true);
    }
  }

  @Test
  public void downloadNowWithCorrectData()
  {
    byte[] sourceBytes = Utils.getData();
    int sleepMillis = 50;
    downloadCorrectDataImpl(sourceBytes, sleepMillis);
  }

  @Test
  public void downloadNowWithCorrectDataZeroLength()
  {
    byte[] sourceBytes = new byte[] {};
    int sleepMillis = 50;
    downloadCorrectDataImpl(sourceBytes, sleepMillis);
  }

  @Test
  public void downloadNowWithCorrectDataOneByte()
  {
    byte[] sourceBytes = new byte[] { 42 };
    int sleepMillis = 50;
    downloadCorrectDataImpl(sourceBytes, sleepMillis);
  }

  private void downloadCorrectDataImpl( byte[] sourceBytes, int sleepMillis )
  {
    final CountDownLatch latch = new CountDownLatch(1);

    IDownloadSource source = Mockito.mock(Utils.MockSource.class, withSettings()
            .useConstructor(sourceBytes, sleepMillis).defaultAnswer(CALLS_REAL_METHODS));
    Utils.MockDestination dest = Mockito.mock(Utils.MockDestination.class, withSettings()
          .defaultAnswer(CALLS_REAL_METHODS));
    DownloadManager dm = new DownloadManager();
    DownloadTaskId downloadTaskId;
    try
    {
      downloadTaskId = dm.downloadNow(source, dest, new DownloadManager.IDownloadStatus()
        {
          @Override
          public void onProgress( long currentSize, long totalSize )
          {

          }

          @Override
          public void onFinished()
          {
            latch.countDown();
          }

          @Override
          public void onDownloadStatusChanged()
          {

          }

          @Override
          public void onFailed( @NonNull DownloadManagerException reason )
          {
            assertTrue("Exception while register observer for download : " + reason, false);
          }
        });
    }
    catch ( Exception e )
    {
      assertTrue("Exception while register observer for download : " + e, false);
    }

    try
    {
      latch.await(2, TimeUnit.SECONDS);
    }
    catch ( InterruptedException e )
    {
      assertTrue("Exception on wait download complete : " + e.getMessage(), false);
    }

    byte[] destinationBytes = dest.getBytes();
    assertTrue("While downloading source data and destination data does not equals", Utils.equalsBytes(sourceBytes, destinationBytes));
  }

  @Test
  public void downloadNowCheckStatusChanges()
  {
    byte[] sourceBytes = Utils.getData();
    int sleepMillis = 50;
    final CountDownLatch latch = new CountDownLatch(1);
    IDownloadSource source = Mockito.mock(Utils.MockSource.class, withSettings()
            .useConstructor(sourceBytes, sleepMillis).defaultAnswer(CALLS_REAL_METHODS));
    Utils.MockDestination dest = Mockito.mock(Utils.MockDestination.class, withSettings()
            .defaultAnswer(CALLS_REAL_METHODS));
    final DownloadManager dm = new DownloadManager();
    DownloadTaskId downloadTaskId = null;
    final List<DOWNLOAD_TASK_STATUS> statuses = new ArrayList<>();

    try
    {
      Utils.DownloadCallback callback = new Utils.DownloadCallback(dm)
      {

        @Override
        public void onProgress( long currentSize, long totalSize )
        {

        }

        @Override
        public void onFinished()
        {
          statuses.add(DOWNLOAD_TASK_STATUS.FINISHED);
        }

        @Override
        public void onDownloadStatusChanged()
        {
          try
          {
            statuses.add(getDownloadManager().getDownloadTaskStatus(getTaskId()).getStatus());
          }
          catch ( NoSuchDownloadTaskException e )
          {
            assertTrue("Exception on get download status : " + e, false);
          }
        }
      };
      downloadTaskId = dm.downloadNow(source, dest, callback);
      callback.setTaskId(downloadTaskId);
      statuses.add(dm.getDownloadTaskStatus(downloadTaskId).getStatus());
    }
    catch ( Exception e )
    {
      assertTrue("Exception while register observer for download : " + e, false);
    }

    try
    {
      latch.await(2, TimeUnit.SECONDS);
    }
    catch ( InterruptedException e )
    {
      assertTrue("Exception on wait download complete : " + e.getMessage(), false);
    }

    DOWNLOAD_TASK_STATUS [] expectedStatuses =  {DOWNLOAD_TASK_STATUS.IN_QUEUE, DOWNLOAD_TASK_STATUS.IN_PROCESS, DOWNLOAD_TASK_STATUS.FINISHED};
    assertTrue("Download process statuses changes does not equals expected  : " + Arrays.toString(expectedStatuses) + "; actual : " + statuses, Utils.equalsDownloadStatuses(Arrays.asList(expectedStatuses), statuses));
  }


  @Test
  public void downloadNowCheckStatusChangesOnPause()
  {
    byte[] sourceBytes = Utils.getData();
    int sleepMillis = 100;
    final CountDownLatch latch = new CountDownLatch(1);
    IDownloadSource source = Mockito.mock(Utils.MockSource.class, withSettings()
            .useConstructor(sourceBytes, sleepMillis).defaultAnswer(CALLS_REAL_METHODS));
    Utils.MockDestination dest = Mockito.mock(Utils.MockDestination.class, withSettings()
            .defaultAnswer(CALLS_REAL_METHODS));
    final DownloadManager dm = new DownloadManager();
    DownloadTaskId downloadTaskId = null;
    final List<DOWNLOAD_TASK_STATUS> statuses = new ArrayList<>();

    try
    {
      Utils.DownloadCallback callback = new Utils.DownloadCallback(dm)
      {

        @Override
        public void onProgress( long currentSize, long totalSize )
        {

        }

        @Override
        public void onFinished()
        {
          statuses.add(DOWNLOAD_TASK_STATUS.FINISHED);
        }

        @Override
        public void onDownloadStatusChanged()
        {
          try
          {
            statuses.add(getDownloadManager().getDownloadTaskStatus(getTaskId()).getStatus());
          }
          catch ( NoSuchDownloadTaskException e )
          {
            assertTrue("Exception on get download status : " + e, false);
          }
        }
      };
      downloadTaskId = dm.downloadNow(source, dest, callback);
      callback.setTaskId(downloadTaskId);
      statuses.add(dm.getDownloadTaskStatus(downloadTaskId).getStatus());
    }
    catch ( Exception e )
    {
      assertTrue("Exception while register observer for download : " + e, false);
    }

    Utils.sleep(200);
    try
    {
      dm.pauseDownloadTask(downloadTaskId);
    }
    catch ( NoSuchDownloadTaskException e )
    {
      assertTrue("Exception on try pause download: " + e.getMessage(), false);
    }

    Utils.sleep(300);

    try
    {
      dm.resumeDownloadTask(downloadTaskId);
    }
    catch ( NoSuchDownloadTaskException e )
    {
      assertTrue("Exception on try resume download: " + e.getMessage(), false);
    }

    try
    {
      latch.await(2, TimeUnit.SECONDS);
    }
    catch ( InterruptedException e )
    {
      assertTrue("Exception on wait download complete : " + e.getMessage(), false);
    }

    DOWNLOAD_TASK_STATUS [] expectedStatuses =  {DOWNLOAD_TASK_STATUS.IN_QUEUE, DOWNLOAD_TASK_STATUS.IN_PROCESS, DOWNLOAD_TASK_STATUS.PAUSED, DOWNLOAD_TASK_STATUS.IN_PROCESS, DOWNLOAD_TASK_STATUS.FINISHED};
    assertTrue("Download process statuses changes does not equals expected  : " + Arrays.toString(expectedStatuses) + "; actual : " + statuses, Utils.equalsDownloadStatuses(Arrays.asList(expectedStatuses), statuses));
  }


  @Test
  public void downloadNowCheckStatusChangesOnFailed()
  {
    byte[] sourceBytes = Utils.getData();
    int sleepMillis = 50;
    final CountDownLatch latch = new CountDownLatch(1);
    Utils.MockSource source = Mockito.mock(Utils.MockSource.class, withSettings()
            .useConstructor(sourceBytes, sleepMillis).defaultAnswer(CALLS_REAL_METHODS));
    source.setException(new DownloadFailedException());

    Utils.MockDestination dest = Mockito.mock(Utils.MockDestination.class, withSettings()
            .defaultAnswer(CALLS_REAL_METHODS));
    final DownloadManager dm = new DownloadManager();
    DownloadTaskId downloadTaskId = null;
    final List<DOWNLOAD_TASK_STATUS> statuses = new ArrayList<>();

    Utils.DownloadCallback callback = null;
    try
    {
      callback = new Utils.DownloadCallback(dm)
      {

        @Override
        public void onProgress( long currentSize, long totalSize )
        {

        }

        @Override
        public void onFinished()
        {
          statuses.add(DOWNLOAD_TASK_STATUS.FINISHED);
        }

        @Override
        public void onDownloadStatusChanged()
        {
          try
          {
            statuses.add(getDownloadManager().getDownloadTaskStatus(getTaskId()).getStatus());
          }
          catch ( NoSuchDownloadTaskException e )
          {
            assertTrue("Exception on get download status : " + e, false);
          }
        }
      };
      downloadTaskId = dm.downloadNow(source, dest, callback);
      callback.setTaskId(downloadTaskId);
      statuses.add(dm.getDownloadTaskStatus(downloadTaskId).getStatus());
    }
    catch ( Exception e )
    {
      assertTrue("Exception while register observer for download : " + e, false);
    }

    try
    {
      latch.await(2, TimeUnit.SECONDS);
    }
    catch ( InterruptedException e )
    {
      assertTrue("Exception on wait download complete : " + e.getMessage(), false);
    }

    DOWNLOAD_TASK_STATUS [] expectedStatuses =  {DOWNLOAD_TASK_STATUS.IN_QUEUE, DOWNLOAD_TASK_STATUS.IN_PROCESS};
    assertTrue("Download process statuses changes does not equals expected  : " + Arrays.toString(expectedStatuses) + "; actual : " + statuses, Utils.equalsDownloadStatuses(Arrays.asList(expectedStatuses), statuses));
    assertNotNull("Expected exception while downloading does no exist", callback.getException());
  }


}
