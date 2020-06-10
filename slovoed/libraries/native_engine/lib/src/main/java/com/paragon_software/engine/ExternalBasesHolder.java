package com.paragon_software.engine;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Map;
import java.util.HashMap;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.extbase.SoundInfo;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.nativewrapper.NativeDictionary;

public final class ExternalBasesHolder
{
  private static Map< DictionaryLocation, NativeDictionary > mDictionaryCache = new HashMap<>();

  private ExternalBasesHolder()
  {
  }

  public synchronized static void openExternalBases( @Nullable Context context, @NonNull Dictionary dictionary )
  {
    if ( context != null )
    {
      openSoundBases(context, dictionary);
    }
  }

  private static void openSoundBases( @NonNull Context context, @NonNull Dictionary dictionary )
  {
  }

  static @Nullable
  public NativeDictionary get( @Nullable DictionaryLocation dictionaryLocation )
  {
    if ( dictionaryLocation != null )
    {
      return mDictionaryCache.get(dictionaryLocation);
    }
    return null;
  }
}
