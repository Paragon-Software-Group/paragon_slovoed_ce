package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;

class Flashcard
{
  private final boolean     checked;
  private final long        showCount;
  private final long        rightAnsCount;
  @NonNull
  private final ArticleItem front, back;

  Flashcard( @NonNull ArticleItem front, @NonNull ArticleItem back )
  {
    this.checked = true;
    this.showCount = 0;
    this.rightAnsCount = 0;
    this.front = front;
    this.back = back;
  }

  Flashcard( boolean checked, long showCount, long rightAnsCount, @NonNull ArticleItem front, @NonNull ArticleItem back )
  {
    this.checked = checked;
    this.showCount = showCount;
    this.rightAnsCount = rightAnsCount;
    this.front = front;
    this.back = back;
  }

  boolean isChecked()
  {
    return checked;
  }

  @NonNull
  ArticleItem getFront()
  {
    return front;
  }

  @NonNull
  ArticleItem getBack()
  {
    return back;
  }

  int getPercentSuccess()
  {
    if ( showCount == 0 )
    {
      return 0;
    }
    return (int) ( rightAnsCount * 100 / showCount );
  }

  long getShowCount()
  {
    return showCount;
  }

  long getRightAnsCount()
  {
    return rightAnsCount;
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

    Flashcard flashcard = (Flashcard) o;

    if ( !front.equals(flashcard.front) )
    {
      return false;
    }
    return back.equals(flashcard.back);
  }

  @Override
  public int hashCode()
  {
    int result = front.hashCode();
    result = 31 * result + back.hashCode();
    return result;
  }

  @Override
  public String toString()
  {
    return "Flashcard{" + "checked=" + checked + ", showCount=" + showCount + ", rightAnsCount=" + rightAnsCount
        + ", front=" + front + ", back=" + back + '}';
  }

  static Flashcard increaseShowCount( Flashcard flashcard )
  {
    return new Flashcard(flashcard.checked, flashcard.showCount + 1, flashcard.rightAnsCount, flashcard.front,
                         flashcard.back);
  }

  static Flashcard increaseRightAnswerCount( Flashcard flashcard )
  {
    return new Flashcard(flashcard.checked, flashcard.showCount, flashcard.rightAnsCount + 1, flashcard.front,
                         flashcard.back);
  }

  static Flashcard setChecked( Flashcard flashcard, boolean checked )
  {
    return new Flashcard(checked, flashcard.showCount, flashcard.rightAnsCount, flashcard.front, flashcard.back);
  }

  static Flashcard toggleChecked( Flashcard flashcard )
  {
    return new Flashcard(!flashcard.checked, flashcard.showCount, flashcard.rightAnsCount, flashcard.front,
                         flashcard.back);
  }
}
