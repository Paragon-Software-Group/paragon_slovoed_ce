package com.paragon_software.article_manager;

import android.content.Context;
import androidx.annotation.Nullable;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

final class KeyboardHelper
{
  static void showKeyboard( @Nullable View view )
  {
    InputMethodManager inputMethodManager = getManager(view);
    if ( ( inputMethodManager != null ) && ( view != null ) )
    {
      inputMethodManager.showSoftInput(view, InputMethodManager.SHOW_IMPLICIT);
    }
  }

  static void hideKeyboard( @Nullable View view )
  {
    InputMethodManager inputMethodManager = getManager(view);
    if ( ( inputMethodManager != null ) && ( view != null ) )
    {
      inputMethodManager.hideSoftInputFromWindow(view.getWindowToken(), 0);
    }
  }

  @Nullable
  private static InputMethodManager getManager( @Nullable View view )
  {
    InputMethodManager res = null;
    if ( view != null )
    {
      Context context = view.getContext();
      if ( context != null )
      {
        Object service = context.getSystemService(Context.INPUT_METHOD_SERVICE);
        if ( service instanceof InputMethodManager )
        {
          res = (InputMethodManager) service;
        }
      }
    }
    return res;
  }
}
