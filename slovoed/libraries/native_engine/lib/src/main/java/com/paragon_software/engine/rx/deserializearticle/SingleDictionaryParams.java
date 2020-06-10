package com.paragon_software.engine.rx.deserializearticle;

import androidx.annotation.NonNull;

import java.io.Serializable;
import java.util.Map;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.EngineSerializerAPI;

public class SingleDictionaryParams
{
  @NonNull
  public final Map< Integer, Serializable >                            sortedSerializedMap;
  @NonNull
  public final Dictionary.DictionaryId                                 dictId;
  @NonNull
  public final EngineSerializerAPI.OnDeserializedArticlesReadyCallback callback;

  final public boolean openSoundBases;

  SingleDictionaryParams( @NonNull Map< Integer, Serializable > _sortedSerializedMap, @NonNull Dictionary.DictionaryId _dictId,
                          @NonNull EngineSerializerAPI.OnDeserializedArticlesReadyCallback _callback, boolean _openSoundBases )
  {
    sortedSerializedMap = _sortedSerializedMap;
    dictId = _dictId;
    callback = _callback;
    openSoundBases = _openSoundBases;
  }
}
