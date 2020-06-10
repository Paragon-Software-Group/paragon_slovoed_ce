package com.paragon_software.native_engine;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.text.LocalizedString;

public abstract class DictionaryInfoAPI
{
  //  static final int SHORT_NAME   = 0;
  //  static final int FULL_NAME    = 1;
  //  static final int PRODUCT_NAME = 2;
  //  static final int AUTHOR_WEB   = 3;
  public enum Name
  {
    Short,
    Full,
    Product,
    AuthorWeb
  }

  @NonNull
  public abstract LocalizedString getString(@NonNull Name name );
}
