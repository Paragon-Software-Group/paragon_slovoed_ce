package com.paragon_software.native_engine;


import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.location.DictionaryLocation;

public interface EngineInformationAPI {
  /**
   * Get version of native engine.
   * @return version of native engine
   */
  @NonNull
  EngineVersion getEngineVersion();

  @Nullable
  DictionaryInfoAPI getDictionaryInfo( DictionaryLocation location );
}
