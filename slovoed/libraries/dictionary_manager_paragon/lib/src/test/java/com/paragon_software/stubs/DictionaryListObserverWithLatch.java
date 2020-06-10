package com.paragon_software.stubs;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;

import java.util.concurrent.CountDownLatch;

public abstract class DictionaryListObserverWithLatch implements DictionaryManagerAPI.IDictionaryListObserver
{
  private CountDownLatch latch;

  public DictionaryListObserverWithLatch( CountDownLatch latch )
  {
    this.latch = latch;
  }

  @Override
  public void onDictionaryListChanged()
  {
    if ( null != latch )
    {
      latch.countDown();
    }
  }

  public void setLatch( CountDownLatch latch )
  {
    this.latch = latch;
  }
}