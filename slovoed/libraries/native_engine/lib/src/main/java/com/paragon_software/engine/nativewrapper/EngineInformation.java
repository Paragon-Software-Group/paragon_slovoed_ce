package com.paragon_software.engine.nativewrapper;

import androidx.annotation.NonNull;

import com.paragon_software.native_engine.EngineVersion;

public final class EngineInformation implements EngineVersion {
  @NonNull
  public static final EngineInformation INSTANCE = new EngineInformation();

  private EngineInformation() { }

  public int getVersion() {
    return NativeFunctions.ENGINE_VERSION;
  }

  public int getBuild() {
    return NativeFunctions.ENGINE_BUILD;
  }
}
