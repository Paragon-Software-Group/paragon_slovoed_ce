package com.paragon_software.native_engine;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.data.SoundData;

/**
 * Interface with methods for working with sound data in slovoed engine.
 */
public interface EngineSoundAPI
{
  /**
   * Registers a dictionary manager to handle dictionary updates.
   *
   * @param manager dictionary manager to register
   */
  void registerDictionaryManager( @NonNull DictionaryManagerAPI manager );

  /**
   * Synchronously check is there is a sound data for specified {@link ArticleItem} article item.
   *
   * @param item {@link ArticleItem} item for which need to check sound data
   *
   * @return True if there is a sound data for specified {@link ArticleItem} article item
   * (from internal or external base), otherwise false
   */
  boolean itemHasSound( @NonNull ArticleItem item );

  /**
   * Asynchronously get sound data for specified {@link ArticleItem} article item.
   *
   * @param item     {@link ArticleItem} item for which need to get sound data
   * @param callback callback that will receive the sound data
   */
  void getSoundData( @NonNull ArticleItem item, @NonNull OnSoundReadyCallback callback );

  void getSoundData(@NonNull ArticleItem item, @NonNull String soundBaseIdx, @NonNull String soundKey, @NonNull OnSoundReadyCallback callback);

  void getSoundData(@NonNull ArticleItem item, int soundIdx, @NonNull OnSoundReadyCallback callback);

  SoundData getSoundDataFromSpxSynch(Dictionary.DictionaryId dictionaryId, byte[] bytes);

  /**
   * Interface of callbacks receiving sound data from the engine.
   */
  interface OnSoundReadyCallback
  {
    /**
     * @param soundData sound data
     */
    void onSoundReady( @NonNull SoundData soundData );
  }
}
