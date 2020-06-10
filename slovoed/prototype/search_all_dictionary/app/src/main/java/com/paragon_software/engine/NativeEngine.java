/*
 * SearchAllDictionary
 *
 *  Created on: 28.05.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.engine;

import android.support.annotation.NonNull;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

import com.paragon_software.search_all_dictionary.dictionary_manager.Dictionary;
import com.paragon_software.search_all_dictionary.search_manager.ParagonSearchManager;
import com.paragon_software.search_all_dictionary.search_manager.SearchResult;

public final class NativeEngine
{
  // Sets the amount of time an idle thread waits before terminating
  private static final int                     KEEP_ALIVE_TIME      = 1;
  // Sets the Time Unit to seconds
  private static final TimeUnit                KEEP_ALIVE_TIME_UNIT = TimeUnit.SECONDS;
  private static       int                     NUMBER_OF_CORES      = Runtime.getRuntime().availableProcessors();
  private final        Map< Long, Dictionary > m_Dictionaries       = new HashMap<>();

  private final BlockingQueue< Runnable > queue = new LinkedBlockingQueue<>();

  private final ThreadPoolExecutor m_searchThreads =
      new ThreadPoolExecutor(NUMBER_OF_CORES, 2 * NUMBER_OF_CORES, KEEP_ALIVE_TIME, KEEP_ALIVE_TIME_UNIT, queue);

  public void showDictionaries()
  {
    for ( final Dictionary dictionary : m_Dictionaries.values() )
    {
      final Dictionary.DictionaryOptions options = dictionary.getOptions();
      Log.i("!!!Dictionaries!!!",
            "ID: " + String.valueOf(options.getId()) + ", Name: " + options.getDictionaryName() + ", class: " + options
                .getDictionaryClass() + ", Product: " + options.getProductName() + ", LNG: " + options
                .getDictionaryLanguagePairShort());
    }
  }

  public void load( final Dictionary dictionary )
  {
    final Object object;
    if ( dictionary.isStream() )
    {
      object = NativeFunctions
          .native_1(NativeFunctions.LOAD_DICTIONARY, new InputStreamWrapper(dictionary.getInputStream()));
    }
    else
    {
      object = NativeFunctions.native_1(NativeFunctions.LOAD_DICTIONARY, dictionary.getFD());
    }
    if ( object instanceof Dictionary.DictionaryOptions )
    {
      Dictionary.DictionaryOptions options = (Dictionary.DictionaryOptions) object;
      dictionary.setOptions(options);
      m_Dictionaries.put(options.getId(), dictionary);
    }
    else
    {
      dictionary.close();
    }
  }

  public void close()
  {
    for ( final Map.Entry< Long, Dictionary > entry : m_Dictionaries.entrySet() )
    {
      NativeFunctions.native_1(NativeFunctions.CLOSE_DICTIONARY, entry.getKey());
      entry.getValue().close();
    }
    m_Dictionaries.clear();
  }

  public void search( @NonNull final String word, @NonNull final ParagonSearchManager.OnSearchStatus status )
  {
    m_searchThreads.purge();
    final AtomicBoolean allThreadNeedClean = new AtomicBoolean(true);
    for ( final Map.Entry< Long, Dictionary > entry : m_Dictionaries.entrySet() )
    {
      m_searchThreads.execute(new Runnable()
      {
        @Override
        public void run()
        {
          final boolean[] needClean = { allThreadNeedClean.get() };
          NativeFunctions.native_2(NativeFunctions.SEARCH_WORD, entry.getKey(), word, new NativeFunctions.Callback()
          {
            @Override
            public synchronized void function( final Object result )
            {
              if ( result instanceof SearchResult )
              {
                if ( needClean[0] )
                {
                  needClean[0] = allThreadNeedClean.get();
                }
                ( (SearchResult) result ).setDictionary(entry.getValue().getOptions());

                status.onSearchStatus(needClean[0], (SearchResult) result);
                if ( needClean[0] )
                {
                  allThreadNeedClean.compareAndSet(true, false);
                }
              }
            }
          });
        }
      });
    }
  }

  static final class InputStreamWrapper
  {
    final InputStream stream;
    byte[] tempBuffer = null;
    long   offset     = 0;

    InputStreamWrapper( final InputStream stream )
    {
      this.stream = stream;
      this.stream.mark(0);
    }

    int read( final long offset, final int size )
    {
      if ( null == tempBuffer || tempBuffer.length < size )
      {
        tempBuffer = new byte[size];
      }
      try
      {
        if ( offset < this.offset )
        {
          stream.reset();
          this.offset = 0;
          stream.mark(0);
          this.offset += stream.skip(offset);
        }
        else if ( offset > this.offset )
        {
          this.offset += stream.skip(offset - this.offset);
        }
        final int result = stream.read(tempBuffer, 0, size);
        this.offset += result;
        return result;
      }
      catch ( IOException ignore )
      {
      }
      return 0;
    }
  }
}
