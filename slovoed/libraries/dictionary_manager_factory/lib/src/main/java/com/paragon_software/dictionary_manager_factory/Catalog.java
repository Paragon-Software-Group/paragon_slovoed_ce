package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

import java.util.Map;

class Catalog
{
  @SerializedName("products")
  private Map< String, Product > productsMap;

  @SerializedName("trialLengthInDays")
  private String trialLengthInDays;

  @SerializedName("locale")
  private LocaleData locale;

  Map< String, Product > getProductsMap() {
    return productsMap;
  }

  String getTrialLengthInDays() {
    return trialLengthInDays;
  }

  LocaleData getLocale() {
    return locale;
  }
}
