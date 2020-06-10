package com.paragon_software.license_manager_api;

import androidx.annotation.Nullable;

class TextUtils
{
  public static boolean isEmpty( @Nullable CharSequence str )
  {
    return str == null || str.length() == 0;
  }
}
