package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;

public interface IDictionaryDiscount
{
  int getPercentage();

  @NonNull
  Long getDefaultPrice();

  @NonNull
  Long getActualPrice();
}
