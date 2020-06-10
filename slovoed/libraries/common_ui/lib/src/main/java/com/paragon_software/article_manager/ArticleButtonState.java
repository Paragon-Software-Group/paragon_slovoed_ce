package com.paragon_software.article_manager;

import android.graphics.drawable.Drawable;
import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

class ArticleButtonState extends ButtonState
{
  private @NonNull
  final Drawable icon;
  private @NonNull
  final String   text;

  ArticleButtonState(@NonNull VisibilityState _visibilityState, @NonNull CheckState _checkState,
                     @NonNull Drawable _icon, @NonNull String _text )
  {
    super(_visibilityState, _checkState);
    icon = _icon;
    text = _text;
  }

  @NonNull
  Drawable getIcon()
  {
    return icon;
  }

  @NonNull
  String getText()
  {
    return text;
  }
}
