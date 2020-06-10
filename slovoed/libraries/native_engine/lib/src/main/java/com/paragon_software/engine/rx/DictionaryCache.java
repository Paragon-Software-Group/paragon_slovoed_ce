package com.paragon_software.engine.rx;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Map;
import java.util.LinkedHashMap;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.ExternalBasesHolder;
import com.paragon_software.engine.nativewrapper.NativeDictionary;

public class DictionaryCache
{
  private final Context applicationContex;
  private final int     max;

  @NonNull
  private final Map< DictionaryLocation, NativeDictionary > map =
      new LinkedHashMap<>(4, 0.75f, true);

  public DictionaryCache( Context _context, int _max )
  {
    applicationContex = _context.getApplicationContext();
    max = _max > 0 ? _max : 0;
  }

  public NativeDictionary get( @Nullable Dictionary dictionary, boolean openMorpho, boolean openExtBases )
  {
    NativeDictionary res = null;
    if ( dictionary != null && dictionary.getDictionaryLocation() != null )
    {
      res = map.get(dictionary.getDictionaryLocation());
      if ( res == null )
      {
        res = NativeDictionary.open(applicationContex, dictionary, openMorpho, openExtBases);
        if ( res != null )
        {
          map.put(dictionary.getDictionaryLocation(), res);
          while ( map.size() > max )
          {
            map.remove(map.keySet().iterator().next());
          }
        }
      }
      else
      {
        if ( openExtBases )
        {
          ExternalBasesHolder.openExternalBases(applicationContex, dictionary);
        }
      }
    }
    return res;
  }
}
