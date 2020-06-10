package com.paragon_software.native_engine;

import com.paragon_software.history_manager.mock.StaticConsts;

import java.io.Serializable;

import androidx.annotation.NonNull;

public abstract class MockEngineSerializer implements EngineSerializerAPI {

  @Override
  public void deserializeArticleItems(@NonNull Serializable[] serialized,
                                      @NonNull OnDeserializedArticlesReadyCallback callback,
                                      boolean openSoundBases) {

    callback.onDeserializedArticlesReady(StaticConsts.ARTICLE_ITEMS);
    callback.onDeserializationStatusChanged(false);
  }

}
