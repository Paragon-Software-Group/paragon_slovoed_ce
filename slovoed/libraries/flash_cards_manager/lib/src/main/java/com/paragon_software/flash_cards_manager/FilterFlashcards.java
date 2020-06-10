package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.collections.ArrayCollectionView;
import com.paragon_software.utils_slovoed.collections.FilterCollectionView;
import com.paragon_software.utils_slovoed.collections.MutableCollectionView;

class FilterFlashcards extends
                               FilterCollectionView< FlashcardBox, BaseQuizAndFlashcardMetadata, ArrayCollectionView< FlashcardBox, BaseQuizAndFlashcardMetadata > >
    implements MutableCollectionView< FlashcardBox, BaseQuizAndFlashcardMetadata >
{
  FilterFlashcards( ArrayCollectionView< FlashcardBox, BaseQuizAndFlashcardMetadata > coreCollection,
                    @NonNull Predicate< FlashcardBox > predicate )
  {
    super(coreCollection, predicate);
  }

  @Override
  public void update( int position, FlashcardBox flashcardBox )
  {
    getCore().update(getCoreIndex(position), flashcardBox);
  }

  @Override
  public void remove( int position )
  {
    getCore().remove(getCoreIndex(position));
  }

  @Override
  public void updateMetadata( @Nullable BaseQuizAndFlashcardMetadata metadata )
  {
    getCore().updateMetadata(metadata);
  }

  static class IsFlashcardForFullDictionary implements Predicate< FlashcardBox >
  {
    private final Dictionary.DictionaryId dictionaryId;

    IsFlashcardForFullDictionary( @NonNull Dictionary.DictionaryId dictionaryId )
    {
      this.dictionaryId = dictionaryId;
    }

    @Override
    public boolean apply( FlashcardBox flashcardBox )
    {
      return flashcardBox.getParent().getDictId().equals(dictionaryId) && !flashcardBox.getParent().isLocked();
    }
  }

  static class IsFlashcardCheckedAndForFullDictionary extends IsFlashcardForFullDictionary
  {
    IsFlashcardCheckedAndForFullDictionary( @NonNull Dictionary.DictionaryId dictionaryId )
    {
      super(dictionaryId);
    }

    @Override
    public boolean apply( FlashcardBox flashcardBox )
    {
      return flashcardBox.isChecked() && super.apply(flashcardBox);
    }
  }
}
