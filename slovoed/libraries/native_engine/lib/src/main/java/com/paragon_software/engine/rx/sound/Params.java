package com.paragon_software.engine.rx.sound;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.native_engine.EngineSoundAPI;

public class Params
{
  @NonNull
  final EngineSoundAPI.OnSoundReadyCallback callback;

  @Nullable
  final ArticleItem articleItem;

  @Nullable
  final NativeDictionary cachedSoundNativeDictionary;

  @Nullable
  final String soundKey;

  @Nullable
  final String soundBaseIdx;

  final int soundIdx;

  Params( @NonNull EngineSoundAPI.OnSoundReadyCallback _callback, @Nullable ArticleItem _articleItem,
          @Nullable NativeDictionary _cachedSoundNativeDictionary, @Nullable String _soundKey,
          @Nullable String _soundBaseIdx , int _soundIdx)
  {
    callback = _callback;
    articleItem = _articleItem;
    cachedSoundNativeDictionary = _cachedSoundNativeDictionary;
    soundKey = _soundKey;
    soundBaseIdx = _soundBaseIdx;
    soundIdx = _soundIdx;
  }
}
