package com.paragon_software.utils_slovoed.ui_states;

import androidx.annotation.NonNull;
import android.view.View;

public enum VisibilityState
{
  gone,
  disabled,
  enabled;

  public void applyTo(@NonNull View view) {
    if(gone.equals(this))
      view.setVisibility(View.GONE);
    else {
      view.setVisibility(View.VISIBLE);
      view.setEnabled(enabled.equals(this));
    }
  }

  @Override
  public String toString()
  {
    String res = "";
    switch ( this )
    {
      case gone:
        res = "GONE";
        break;
      case disabled:
        res = "DISABLED";
        break;
      case enabled:
        res = "ENABLED";
        break;
    }
    return res;
  }
}
