package com.paragon_software.engine.rx.deserializearticle;

import androidx.annotation.NonNull;

import java.io.Serializable;

import com.paragon_software.native_engine.EngineSerializerAPI;

public class Params
{
  @NonNull
  final public Serializable[]                                          serialized;
  @NonNull
  final public EngineSerializerAPI.OnDeserializedArticlesReadyCallback callback;

  final public boolean openSoundBases;

  Params( @NonNull Serializable[] serialized,
          @NonNull EngineSerializerAPI.OnDeserializedArticlesReadyCallback callback,
          boolean openSoundBases )
  {
    this.serialized = serialized;
    this.callback = callback;
    this.openSoundBases = openSoundBases;
  }
}
