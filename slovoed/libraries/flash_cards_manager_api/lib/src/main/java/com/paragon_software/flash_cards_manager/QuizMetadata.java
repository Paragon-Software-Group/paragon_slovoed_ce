package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Map;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

public class QuizMetadata implements BaseQuizAndFlashcardMetadata
{
  public enum ButtonType
  {
    ShowMeaning,
    Right,
    Wrong,
    StartAgain,
    Pronunciation,
    Delete
  }

  public enum QuizStateType
  {
    ARTICLE_FRAGMENT_VISIBLE,
    USER_TRY_TO_RECALL,
    RESULT_OF_QUIZ,
    RESULT_OF_QUIZ_CONGRATULATIONS
  }

  @NonNull
  QuizStateType quizStateType;

  @Nullable
  ArticleItem articleItem;

  int correctCounter = Integer.MIN_VALUE;

  int incorrectCounter = Integer.MIN_VALUE;

  @NonNull
  final VisibilityState htmlVisibility;

  @NonNull
  final VisibilityState articleVisibility;

  @NonNull
  final Map< ButtonType, ButtonState > buttonState;

  QuizMetadata( @NonNull VisibilityState _htmlVisibility, @NonNull VisibilityState _articleVisibility,
                @NonNull Map< ButtonType, ButtonState > _buttonState )
  {
    quizStateType = QuizStateType.ARTICLE_FRAGMENT_VISIBLE;
    htmlVisibility = _htmlVisibility;
    articleVisibility = _articleVisibility;
    buttonState = _buttonState;
  }

  QuizMetadata( @NonNull ArticleItem _articleItem, @NonNull VisibilityState _htmlVisibility,
                @NonNull VisibilityState _articleVisibility, @NonNull Map< ButtonType, ButtonState > _buttonState )
  {
    quizStateType = QuizStateType.USER_TRY_TO_RECALL;
    articleItem = _articleItem;
    htmlVisibility = _htmlVisibility;
    articleVisibility = _articleVisibility;
    buttonState = _buttonState;
  }

  QuizMetadata( int _correctCounter, int _incorrectCounter, @NonNull VisibilityState _htmlVisibility,
                @NonNull VisibilityState _articleVisibility, @NonNull Map< ButtonType, ButtonState > _buttonState )
  {
    quizStateType = isCorrectCounterGreater50Percentage(_correctCounter, _incorrectCounter)
                    ? QuizStateType.RESULT_OF_QUIZ_CONGRATULATIONS : QuizStateType.RESULT_OF_QUIZ;
    correctCounter = _correctCounter;
    incorrectCounter = _incorrectCounter;
    htmlVisibility = _htmlVisibility;
    articleVisibility = _articleVisibility;
    buttonState = _buttonState;
  }

  private static boolean isCorrectCounterGreater50Percentage( int correctCounter, int incorrectCounter )
  {
    return correctCounter != 0 && correctCounter >= incorrectCounter;
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

    QuizMetadata that = (QuizMetadata) o;

    if ( correctCounter != that.correctCounter )
    {
      return false;
    }
    if ( incorrectCounter != that.incorrectCounter )
    {
      return false;
    }
    if ( quizStateType != that.quizStateType )
    {
      return false;
    }
    if ( articleItem != null ? !articleItem.equals(that.articleItem) : that.articleItem != null )
    {
      return false;
    }
    if ( htmlVisibility != that.htmlVisibility )
    {
      return false;
    }
    if ( articleVisibility != that.articleVisibility )
    {
      return false;
    }
    return buttonState.equals(that.buttonState);
  }

  @Override
  public int hashCode()
  {
    int result = quizStateType.hashCode();
    result = 31 * result + ( articleItem != null ? articleItem.hashCode() : 0 );
    result = 31 * result + correctCounter;
    result = 31 * result + incorrectCounter;
    result = 31 * result + htmlVisibility.hashCode();
    result = 31 * result + articleVisibility.hashCode();
    result = 31 * result + buttonState.hashCode();
    return result;
  }
}
