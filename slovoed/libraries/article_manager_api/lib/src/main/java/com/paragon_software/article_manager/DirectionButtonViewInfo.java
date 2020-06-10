package com.paragon_software.article_manager;

import android.graphics.Bitmap;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Language;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;

public class DirectionButtonViewInfo
{
  private final ButtonState buttonState;
  @Nullable
  private final Bitmap      bitmap;
  @Nullable
  private final Language    languageFrom;
  @Nullable
  private final Language    languageTo;

  DirectionButtonViewInfo( @NonNull ButtonState buttonState, @Nullable Bitmap bitmap, @Nullable Language languageFrom,
                           @Nullable Language languageTo )
  {
    this.buttonState = buttonState;
    this.bitmap = bitmap;
    this.languageFrom = languageFrom;
    this.languageTo = languageTo;
  }

  @NonNull
  public ButtonState getButtonState()
  {
    return buttonState;
  }

  @Nullable
  public Bitmap getBitmap()
  {
    return bitmap;
  }

  @Nullable
  public Language getLanguageFrom()
  {
    return languageFrom;
  }

  @Nullable
  public Language getLanguageTo()
  {
    return languageTo;
  }

  @SuppressWarnings( "ConstantConditions" )
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

    DirectionButtonViewInfo that = (DirectionButtonViewInfo) o;

    if ( buttonState != null ? !buttonState.equals(that.buttonState) : that.buttonState != null )
    {
      return false;
    }
    if ( bitmap != null ? !bitmap.equals(that.bitmap) : that.bitmap != null )
    {
      return false;
    }
    //noinspection SimplifiableIfStatement
    if ( languageFrom != that.languageFrom )
    {
      return false;
    }
    return languageTo == that.languageTo;
  }

  @Override
  public int hashCode()
  {
    int result = buttonState.hashCode();
    result = 31 * result + ( bitmap != null ? bitmap.hashCode() : 0 );
    result = 31 * result + ( languageFrom != null ? languageFrom.hashCode() : 0 );
    result = 31 * result + ( languageTo != null ? languageTo.hashCode() : 0 );
    return result;
  }
}
