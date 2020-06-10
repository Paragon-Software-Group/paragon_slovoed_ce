package com.paragon_software.engine.rx.deserializearticle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Map;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.native_engine.EngineSerializerAPI;

public class Result
{
  @NonNull
  public final EngineSerializerAPI.OnDeserializedArticlesReadyCallback callback;
  @Nullable
  public final Map< Integer, ArticleItem >                             sortedItemsMap;

  Result( @NonNull EngineSerializerAPI.OnDeserializedArticlesReadyCallback _callback, @Nullable Map< Integer, ArticleItem > _sortedItemsMap )
  {
    callback = _callback;
    sortedItemsMap = _sortedItemsMap;
  }

  boolean isTerminator()
  {
    return sortedItemsMap == null;
  }

  public static Result createTermantor( @NonNull EngineSerializerAPI.OnDeserializedArticlesReadyCallback _callback )
  {
    return new Result(_callback, null);
  }
}
