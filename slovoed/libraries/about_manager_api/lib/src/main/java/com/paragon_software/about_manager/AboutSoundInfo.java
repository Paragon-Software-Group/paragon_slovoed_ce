package com.paragon_software.about_manager;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.text.LocalizedString;

class AboutSoundInfo
{
  @NonNull
  private LocalizedString productName;

  AboutSoundInfo( @NonNull LocalizedString productName )
  {
    this.productName = productName;
  }

  @NonNull
  LocalizedString getProductName()
  {
    return productName;
  }
}
