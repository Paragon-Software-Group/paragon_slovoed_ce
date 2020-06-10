package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.io.Serializable;
import java.util.ArrayList;

class FlashcardPersistLoader
{
  private static final String                                                 FLASHCARDS_SAVE_KEY =
      "FLASHCARDS_SAVE_KEY";
  @NonNull
  private final        FlashcardPersistConverter                              flashcardPersistConverter;
  @NonNull
  private final        SettingsManagerAPI                                     settingsManager;

  FlashcardPersistLoader( @NonNull FlashcardPersistConverter flashcardPersistConverter,
                          @NonNull SettingsManagerAPI settingsManager )
  {
    this.flashcardPersistConverter = flashcardPersistConverter;
    this.settingsManager = settingsManager;
  }

  @NonNull
  ArrayList< FlashcardBox > load()
  {
    return loadFromSettingsManager(new PersistFlashcardBox[0]);
  }

  @NonNull
  private < T extends Serializable > ArrayList< FlashcardBox > loadFromSettingsManager( @NonNull T[] defSerializable )
  {
    try
    {
      T[] loadedData = settingsManager.load(FLASHCARDS_SAVE_KEY, defSerializable);
      if ( loadedData != null && loadedData.length != 0 )
      {
        return flashcardPersistConverter.convertFromPersist(loadedData);
      }
    }
    catch ( Exception e )
    {
      onError(e);
    }
    return new ArrayList<>();
  }

  void save( @NonNull CollectionView< FlashcardBox, BaseQuizAndFlashcardMetadata > flashcards )
  {
    try
    {
      Serializable[] persistData = flashcardPersistConverter.convertToPersist(flashcards);
      settingsManager.save(FLASHCARDS_SAVE_KEY, persistData, true);
    }
    catch ( Exception e )
    {
      onError(e);
    }
  }

  private void onError( Exception exception )
  {
//    Log.e("shdd", getClass().getCanonicalName() + ".onError() called with: exception = [" + exception + "]");
  }
}
