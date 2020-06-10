package com.paragon_software.utils_slovoed.ui_states;

public enum CheckState
{
  uncheckable,
  unchecked,
  checked;

  @Override
  public String toString()
  {
    String res = "";
    switch ( this )
    {
      case uncheckable:
        res = "UNCHECKABLE";
        break;
      case unchecked:
        res = "UNCHECKED";
        break;
      case checked:
        res = "CHECKED";
        break;
    }
    return res;
  }
}
