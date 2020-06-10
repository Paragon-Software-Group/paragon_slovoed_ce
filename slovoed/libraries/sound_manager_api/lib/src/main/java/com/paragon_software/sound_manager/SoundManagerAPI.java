package com.paragon_software.sound_manager;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;

/*
 * sound_manager_api
 *
 *  Created on: 16.05.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */
public interface SoundManagerAPI
{
  boolean playSound( @NonNull ArticleItem currentItem );

  boolean playSound( @NonNull ArticleItem currentItem, int soundIdx );

  boolean playSound( @NonNull ArticleItem currentItem, String language, String dictId, int listId, String key );

  boolean playSound(@NonNull ArticleItem currentItem, String soundBaseIdx, String soundKey);

  boolean playSound(@NonNull Dictionary.DictionaryId dictionaryId,
                    @NonNull Dictionary.Direction direction,
                    String soundBaseIdx,
                    String soundKey);

  boolean playSoundOnline(Dictionary.DictionaryId dictionaryId, String soundBaseIdx, String soundKey);

  void registerOnStateChangedListener( @NonNull OnStateChangedListener listener );

  void unregisterOnStateChangedListener( @NonNull OnStateChangedListener listener );

  boolean isPlaying();

  void setPlayChannel(@NonNull String playChannel);

  boolean itemHasSound( @NonNull ArticleItem item );

  interface OnStateChangedListener
  {
    void onSoundStateChanged();
  }
}
