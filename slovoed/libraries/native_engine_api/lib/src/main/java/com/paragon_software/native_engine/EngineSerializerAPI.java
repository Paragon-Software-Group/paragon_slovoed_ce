package com.paragon_software.native_engine;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.Serializable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;


public interface EngineSerializerAPI
{
  /**
   * Registers a dictionary manager to handle dictionary updates.
   *
   * @param manager dictionary manager to register
   */
  void registerDictionaryManager( @NonNull DictionaryManagerAPI manager );

  /**
   * Prepares article item for serialization.
   *
   * @param articleItem the item to serialize
   *
   * @return serialized object to write to external storage or null on error
   */
  @Nullable
  Serializable serializeArticleItem( @NonNull ArticleItem articleItem );

  /**
   * Decodes a previously serialized article item.
   *
   * @param serialized data read from external storage
   *
   * @return deserialized article item object or null on error
   */
  @Nullable
  ArticleItem deserializeArticleItem( @NonNull Serializable serialized );

  /**
   * Decodes a previously serialized array of article items.
   *
   * @param serialized data read from external storage
   * @param callback callback that will receive the deserialized articles
   * @param openSoundBases is need to open and cache corresponding sound bases during deserialization process
   */
  @Nullable
  void deserializeArticleItems( @NonNull Serializable[] serialized,
                                @NonNull OnDeserializedArticlesReadyCallback callback,
                                boolean openSoundBases );

  /**
   * Interface of callbacks the receive deserialization process status and deserialized articles from the engine.
   */
  interface OnDeserializedArticlesReadyCallback
  {
    /**
     * @param articles the deserialized article items
     */
    void onDeserializedArticlesReady( @NonNull ArticleItem[] articles );

    /**
     * @param inProgress the deserialization process status
     */
    void onDeserializationStatusChanged( boolean inProgress );
  }
}
