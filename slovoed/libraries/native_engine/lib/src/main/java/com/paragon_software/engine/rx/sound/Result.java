package com.paragon_software.engine.rx.sound;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.native_engine.EngineSoundAPI;
import com.paragon_software.native_engine.data.SoundData;

public class Result
{
  @NonNull
  final EngineSoundAPI.OnSoundReadyCallback callback;

  @Nullable
  final SoundData soundData;

  Result( @NonNull EngineSoundAPI.OnSoundReadyCallback _callback, @Nullable SoundData _soundData )
  {
    callback = _callback;
    soundData = _soundData;
  }
}
