package com.paragon_software.sound_manager;

import android.content.Context;
import androidx.annotation.Nullable;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.EngineSoundAPI;

public interface OnlineSoundApi {

  void play(Dictionary.DictionaryId dictionaryId, String soundBaseIdx, String soundKey, EngineSoundAPI.OnSoundReadyCallback callback);

  interface Factory
  {
    OnlineSoundApi create();

    Factory registerContext( @Nullable Context context );

    Factory registerEngineSound( @Nullable EngineSoundAPI engineSound );
  }
}
