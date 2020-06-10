package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.PersistentArticle;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

class FlashcardPersistConverter
{
  @NonNull
  private final EngineSerializerAPI engineSerializer;

  FlashcardPersistConverter( @NonNull EngineSerializerAPI engineSerializer )
  {
    this.engineSerializer = engineSerializer;
  }

  @NonNull
  Serializable[] convertToPersist( @NonNull CollectionView< FlashcardBox, BaseQuizAndFlashcardMetadata > flashcards )
  {
    Serializable[] result = new Serializable[flashcards.getCount()];
    for ( int i = 0 ; i < flashcards.getCount() ; i++ )
    {
      FlashcardBox item = flashcards.getItem(i);
      if ( null != item )
      {
        result[i] = toPersist(item);
      }
    }
    return result;
  }

  ArrayList< FlashcardBox > convertFromPersist( @NonNull Serializable[] data )
  {
    ArrayList< FlashcardBox > flashcardBoxes = new ArrayList<>(data.length);
    for ( Serializable serializable : data )
    {
      if ( serializable instanceof PersistFlashcardBox )
      {
        try
        {
          flashcardBoxes.add(fromPersist((PersistFlashcardBox) serializable));
        }
        catch ( SerializationError ignore )
        {
        }
      }
    }
    return flashcardBoxes;
  }

  @NonNull
  private FlashcardBox fromPersist( @NonNull PersistFlashcardBox persistBox )
  {
    //noinspection ConstantConditions
    if ( null != persistBox.cards && null != persistBox.parent )
    {
      ArrayList< Flashcard > cards = new ArrayList<>(persistBox.cards.length);
      ArticleItem parentArticleItem = deserializeArticleItem(persistBox.parent);
      for ( PersistFlashcard persistCard : persistBox.cards )
      {
        cards.add(fromPersist(persistCard, parentArticleItem));
      }
      return new FlashcardBox(parentArticleItem, cards);
    }
    else
    {
      throw new SerializationError("Cards or parent is null in PersistFlashcardBox while deserialization");
    }
  }

  @NonNull
  private Flashcard fromPersist( @NonNull PersistFlashcard persistCard, @NonNull ArticleItem parentArticleItem )
  {
    ArticleItem front = null == persistCard.front ? parentArticleItem : deserializeArticleItem(persistCard.front);
    ArticleItem back = null == persistCard.back ? parentArticleItem : deserializeArticleItem(persistCard.back);
    return new Flashcard(persistCard.checked, persistCard.showCount, persistCard.rightAnsCount, front, back);
  }

  @NonNull
  private PersistFlashcardBox toPersist( @NonNull FlashcardBox flashcardBox )
  {
    PersistFlashcard[] cards = new PersistFlashcard[flashcardBox.getCards().size()];
    boolean optimizeSerialization = isOptimizationPossible(flashcardBox);
    int i = 0;
    for ( Flashcard card : flashcardBox.getCards() )
    {
      cards[i++] = toPersist(card, optimizeSerialization);
    }
    return new PersistFlashcardBox(serializeArticleItem(flashcardBox.getParent()), cards);
  }

  @NonNull
  private PersistFlashcard toPersist( @NonNull Flashcard card, boolean skipArticleSerialize )
  {
    return new PersistFlashcard(card.isChecked(), card.getShowCount(), card.getRightAnsCount(),
                                skipArticleSerialize ? null : serializeArticleItem(card.getFront()),
                                skipArticleSerialize ? null : serializeArticleItem(card.getBack()));
  }

  private boolean isOptimizationPossible( @NonNull FlashcardBox flashcardBox )
  {
    List< Flashcard > cards = flashcardBox.getCards();
    return 1 == cards.size() && flashcardBox.getParent().equals(cards.get(0).getFront());
  }

  @NonNull
  private ArticleItem deserializeArticleItem( @NonNull Serializable serializable )
  {
    ArticleItem articleItem = engineSerializer.deserializeArticleItem(serializable);
    if ( null == articleItem )
    {
      throw new SerializationError("Cant deserialize [" + serializable + "] with EngineSerializerAPI");
    }
    return articleItem;
  }

  @NonNull
  private PersistentArticle serializeArticleItem( @NonNull ArticleItem articleItem )
  {
    Serializable serializable = engineSerializer.serializeArticleItem(articleItem);
    if ( null == serializable )
    {
      throw new SerializationError("Cant serialize article item [" + articleItem + "] with EngineSerializerAPI");
    }
    return (PersistentArticle) serializable;
  }

  static class SerializationError extends RuntimeException
  {
    SerializationError( @NonNull String msg )
    {
      super(msg);
    }
  }
}
