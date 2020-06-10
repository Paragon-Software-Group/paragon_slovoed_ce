package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

import java.util.List;
import java.util.Map;

class Product
{
  static final String COLLECTION = "collection";

  @SerializedName("components")
  private Components components;

  @SerializedName("lang_from")
  private String langFrom;

  @SerializedName("lang_to")
  private String langTo;

  @SerializedName("from_lang_short")
  private String langFromShort;

  @SerializedName("to_lang_short")
  private String langToShort;

  @SerializedName("names")
  private Map< String, String > namesMap;

  @SerializedName("type")
  private String type;

  @SerializedName("bestseller_for")
  private List<String> bestsellerLanguages;

  @SerializedName("new")
  private boolean isNew = false;

  @SerializedName("featured")
  private boolean isFeatured = false;

  @SerializedName("shoppings")
  private List<Shopping> shoppings;

  @SerializedName("generic_data")
  private GenericData genericData;

  @SerializedName("removed_from_sale")
  private Boolean removedFromSale;

  Components getComponents()
  {
    return components;
  }

  String getLangFrom()
  {
    return langFrom;
  }

  String getLangTo() {
    return langTo;
  }

  String getLangFromShort()
  {
    return langFromShort;
  }

  String getLangToShort()
  {
    return langToShort;
  }

  Map< String, String > getNamesMap()
  {
    return namesMap;
  }

  String getType()
  {
    return type;
  }

  List<String> getBestsellerLanguages()
  {
    return bestsellerLanguages;
  }

  boolean isNew()
  {
    return isNew;
  }

  boolean isFeatured()
  {
    return isFeatured;
  }

  List< Shopping > getShoppings()
  {
    return shoppings;
  }

  GenericData getGenericData() {
    return genericData;
  }

  Boolean getRemovedFromSale() {
    return removedFromSale;
  }
}
