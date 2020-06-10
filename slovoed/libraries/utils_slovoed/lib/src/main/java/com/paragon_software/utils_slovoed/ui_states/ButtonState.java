package com.paragon_software.utils_slovoed.ui_states;

import androidx.annotation.IdRes;
import androidx.annotation.NonNull;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.Checkable;

public class ButtonState
{
  private @NonNull
  final VisibilityState mVisibilityState;
  private @NonNull
  final CheckState      mCheckState;

  public ButtonState( @NonNull VisibilityState _visibilityState, @NonNull CheckState _checkState )
  {
    mVisibilityState = _visibilityState;
    mCheckState = _checkState;
  }

  public @NonNull
  VisibilityState getVisibility()
  {
    return mVisibilityState;
  }

  public @NonNull
  CheckState getCheckState()
  {
    return mCheckState;
  }

  public void applyTo(@NonNull Button button) {
    getVisibility().applyTo(button);
    if((button instanceof Checkable) && (!CheckState.uncheckable.equals(getCheckState())))
      ((Checkable) button).setChecked(CheckState.checked.equals(getCheckState()));
  }

  public void applyTo(@NonNull Menu menu, @IdRes int id) {
    MenuItem menuItem = menu.findItem(id);
    menuItem.setVisible(!VisibilityState.gone.equals(getVisibility()));
    menuItem.setEnabled(!VisibilityState.disabled.equals(getVisibility()));
    menuItem.setCheckable(!CheckState.uncheckable.equals(getCheckState()));
    menuItem.setChecked(CheckState.checked.equals(getCheckState()));
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
    ButtonState that = (ButtonState) o;
    return mVisibilityState == that.mVisibilityState && mCheckState == that.mCheckState;
  }

  @Override
  public int hashCode()
  {
    int result = mVisibilityState.hashCode();
    result = 31 * result + mCheckState.hashCode();
    return result;
  }

  @Override
  public String toString()
  {
    return getVisibility().toString() + "/" + getCheckState().toString();
  }
}
