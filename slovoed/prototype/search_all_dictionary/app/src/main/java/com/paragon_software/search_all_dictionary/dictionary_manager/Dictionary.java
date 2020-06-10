/*
 * search_all_dictionary
 *
 *  Created on: 18.06.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.search_all_dictionary.dictionary_manager;

import android.content.res.AssetFileDescriptor;
import android.os.ParcelFileDescriptor;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import java.io.IOException;
import java.io.InputStream;

public final class Dictionary
{
  private final InputStream          inputStream;
  private final AssetFileDescriptor  descriptor;
  private final ParcelFileDescriptor parcelFileDescriptor;
  private DictionaryOptions options = null;

  public Dictionary( @NonNull final ParcelFileDescriptor descriptor )
  {
    this.inputStream = null;
    this.descriptor = null;
    this.parcelFileDescriptor = descriptor;
  }

  public Dictionary( @NonNull final AssetFileDescriptor descriptor )
  {
    this.inputStream = null;
    this.descriptor = descriptor;
    this.parcelFileDescriptor = this.descriptor.getParcelFileDescriptor();
  }

  public Dictionary( @NonNull final InputStream stream )
  {
    this.inputStream = stream;
    this.descriptor = null;
    this.parcelFileDescriptor = null;
  }

  @NonNull
  public DictionaryOptions getOptions()
  {
    if ( null == options )
    {
      return new DictionaryOptions();
    }
    return options;
  }

  public void setOptions( @NonNull final DictionaryOptions options )
  {
    this.options = options;
  }

  public boolean isStream()
  {
    return null != inputStream;
  }

  public InputStream getInputStream()
  {
    return inputStream;
  }

  public int getFD()
  {
    return parcelFileDescriptor.getFd();
  }

  public void close()
  {
    if ( null != inputStream )
    {
      try
      {
        inputStream.close();
      }
      catch ( IOException ignore )
      {
      }
    }
    else
    {
      try
      {
        parcelFileDescriptor.close();
      }
      catch ( IOException ignore )
      {
      }
      if ( null != descriptor )
      {
        try
        {
          descriptor.close();
        }
        catch ( IOException ignore )
        {
        }
      }
    }
  }

  public static final class DictionaryOptions
  {
    private final long   id;
    @NonNull
    private final String productName;
    @NonNull
    private final String dictionaryName;
    @NonNull
    private final String dictionaryNameShort;
    @NonNull
    private final String dictionaryClass;
    @NonNull
    private final String dictionaryLanguagePair;
    @NonNull
    private final String dictionaryLanguagePairShort;
    @NonNull
    private final String authorName;

    DictionaryOptions()
    {
      this.id = -1;
      this.productName = "";
      this.dictionaryName = "";
      this.dictionaryNameShort = "";
      this.dictionaryClass = "";
      this.dictionaryLanguagePair = "";
      this.dictionaryLanguagePairShort = "";
      this.authorName = "";
    }

    public DictionaryOptions( long id, @Nullable String productName, @Nullable String dictionaryName,
                              @Nullable String dictionaryNameShort, @Nullable String dictionaryClass,
                              @Nullable String dictionaryLanguagePair, @Nullable String dictionaryLanguagePairShort,
                              @Nullable String authorName )
    {
      this.id = id;
      if ( null == productName )
      {
        this.productName = "";
      }
      else
      {
        this.productName = productName;
      }

      if ( null == dictionaryName )
      {
        this.dictionaryName = "";
      }
      else
      {
        this.dictionaryName = dictionaryName;
      }

      if ( null == dictionaryNameShort )
      {
        this.dictionaryNameShort = "";
      }
      else
      {
        this.dictionaryNameShort = dictionaryNameShort;
      }

      if ( null == dictionaryClass )
      {
        this.dictionaryClass = "";
      }
      else
      {
        this.dictionaryClass = dictionaryClass;
      }

      if ( null == dictionaryLanguagePair )
      {
        this.dictionaryLanguagePair = "";
      }
      else
      {
        this.dictionaryLanguagePair = dictionaryLanguagePair;
      }

      if ( null == dictionaryLanguagePairShort )
      {
        this.dictionaryLanguagePairShort = "";
      }
      else
      {
        this.dictionaryLanguagePairShort = dictionaryLanguagePairShort;
      }

      if ( null == authorName )
      {
        this.authorName = "";
      }
      else
      {
        this.authorName = authorName;
      }
    }

    public long getId()
    {
      return id;
    }

    @NonNull
    public String getProductName()
    {
      return productName;
    }

    @NonNull
    public String getDictionaryName()
    {
      return dictionaryName;
    }

    @NonNull
    public String getDictionaryNameShort()
    {
      return dictionaryNameShort;
    }

    @NonNull
    public String getDictionaryClass()
    {
      return dictionaryClass;
    }

    @NonNull
    public String getDictionaryLanguagePair()
    {
      return dictionaryLanguagePair;
    }

    @NonNull
    public String getDictionaryLanguagePairShort()
    {
      return dictionaryLanguagePairShort;
    }

    @NonNull
    public String getAuthorName()
    {
      return authorName;
    }
  }
}
