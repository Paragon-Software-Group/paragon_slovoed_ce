/*
 * SearchAllDictionary
 *
 *  Created on: 28.05.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.search_all_dictionary;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.support.annotation.NonNull;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import com.paragon_software.engine.NativeEngine;
import com.paragon_software.search_all_dictionary.dictionary_manager.Dictionary;
import com.paragon_software.search_all_dictionary.search_manager.ParagonSearchManager;
import com.paragon_software.search_all_dictionary.search_manager.SearchManager;

public class ApplicationState
{
  @NonNull
  private final static ApplicationState state         = new ApplicationState();
  @NonNull
  private final        NativeEngine     engine        = new NativeEngine();
  @NonNull
  private final        SearchManager    searchManager = new ParagonSearchManager(engine);
  private              boolean          initiated     = false;

  @NonNull
  public static ApplicationState instance()
  {
    return state;
  }

  public synchronized void init( @NonNull final Context context )
  {
    if ( !initiated )
    {
      initiated = true;
      final Thread loader = new Thread("Dictionary Loader")
      {
        @Override
        public void run()
        {
          final AssetManager manager = context.getAssets();
          if ( null != manager )
          {
            try
            {
              final String[] dictionaries = manager.list("");
              for ( final String dictionary : dictionaries )
              {
                if ( dictionary.endsWith(".sdc") )
                {
                  try
                  {
                    engine.load(new Dictionary(manager.openFd(dictionary)));
                  }
                  catch ( IOException ignore )
                  {
                    engine.load(new Dictionary(manager.open(dictionary)));
                  }
                }
              }
            }
            catch ( IOException ignore )
            {
            }
          }
          final String state = android.os.Environment.getExternalStorageState();
          if ( android.os.Environment.MEDIA_MOUNTED.equals(state) || android.os.Environment.MEDIA_MOUNTED_READ_ONLY
              .equals(state) )
          {
            final File path = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS);
            if ( null != path )
            {
              final File[] dictionaries = path.listFiles();
              if ( null != dictionaries )
              {
                for ( final File dictionary : dictionaries )
                {
                  if ( dictionary.getName().endsWith(".sdc") )
                  {
                    try
                    {
                      engine.load(
                          new Dictionary(ParcelFileDescriptor.open(dictionary, ParcelFileDescriptor.MODE_READ_ONLY)));
                    }
                    catch ( FileNotFoundException ignore )
                    {
                    }
                  }
                }
              }
            }
          }
          engine.showDictionaries();
        }
      };
      //      loader.setPriority(Thread.MIN_PRIORITY);
      loader.start();
    }
  }

  @NonNull
  public final SearchManager getSearchManager()
  {
    return searchManager;
  }
}
