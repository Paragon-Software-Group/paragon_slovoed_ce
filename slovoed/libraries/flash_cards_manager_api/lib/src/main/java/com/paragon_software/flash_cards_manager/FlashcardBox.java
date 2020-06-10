package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

class FlashcardBox
{
  @NonNull
  private final ArticleItem       parent;
  @NonNull
  private final List< Flashcard > cards;

  static FlashcardBox create( @NonNull ArticleItem articleItem )
  {
    List< Flashcard > cardList = Collections.singletonList(new Flashcard(articleItem, articleItem));
    return new FlashcardBox(articleItem, cardList);
  }

  FlashcardBox( @NonNull ArticleItem parent, @NonNull List< Flashcard > cards )
  {
    if ( cards.isEmpty() )
    {
      throw new IllegalStateException("Cards in card box cant be empty");
    }
    this.parent = parent;
    this.cards = new ArrayList<>(cards);
  }

  boolean isChecked()
  {
    for ( Flashcard card : cards )
    {
      if ( card.isChecked() )
      {
        return true;
      }
    }
    return false;
  }

  int getPercentSuccess()
  {
    Integer min = null;
    for ( Flashcard card : cards )
    {
      if ( null == min )
      {
        min = card.getPercentSuccess();
      }
      min = Math.min(card.getPercentSuccess(), min);
    }
    return null == min ? 0 : min;
  }

  @NonNull
  ArticleItem getParent()
  {
    return parent;
  }

  @NonNull
  List< Flashcard > getCards()
  {
    return cards;
  }

  @Override
  public boolean equals( Object o )
  {
    if ( this == o )
    {
      return true;
    }
    if ( o == null || getClass() != o.getClass() )
    {
      return false;
    }

    FlashcardBox that = (FlashcardBox) o;

    if ( !parent.equals(that.parent) )
    {
      return false;
    }
    return cards.equals(that.cards);
  }

  @Override
  public int hashCode()
  {
    int result = parent.hashCode();
    result = 31 * result + cards.hashCode();
    return result;
  }

  @Override
  public String toString()
  {
    return "FlashcardBox{" + "parent=" + parent + ", cards=" + cards + '}';
  }

  static FlashcardBox setChecked( FlashcardBox flashcardBox, boolean checked )
  {
    List< Flashcard > resultCards = new ArrayList<>(flashcardBox.cards.size());
    for ( Flashcard card : flashcardBox.cards )
    {
      resultCards.add(Flashcard.setChecked(card, checked));
    }
    return new FlashcardBox(flashcardBox.parent, resultCards);
  }

  @NonNull
  static FlashcardBox increaseRightAnswerCount( @NonNull FlashcardBox flashcardBox )
  {
    List< Flashcard > resultCards = new ArrayList<>(flashcardBox.cards.size());
    int i = 0;
    for ( Flashcard card : flashcardBox.cards )
    {
      resultCards.add(0 == i++ ? Flashcard.increaseRightAnswerCount(card) : card);
    }
    return new FlashcardBox(flashcardBox.parent, resultCards);
  }

  @NonNull
  static FlashcardBox increaseShowCount( @NonNull FlashcardBox flashcardBox )
  {
    List< Flashcard > resultCards = new ArrayList<>(flashcardBox.cards.size());
    int i = 0;
    for ( Flashcard card : flashcardBox.cards )
    {
      resultCards.add(0 == i++ ? Flashcard.increaseShowCount(card) : card);
    }
    return new FlashcardBox(flashcardBox.parent, resultCards);
  }
}
