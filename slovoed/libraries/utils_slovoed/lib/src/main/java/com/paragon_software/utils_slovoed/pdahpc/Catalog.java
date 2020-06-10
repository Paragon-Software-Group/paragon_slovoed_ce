package com.paragon_software.utils_slovoed.pdahpc;

import com.google.gson.annotations.SerializedName;

public final class Catalog
{

  @SerializedName("about")
  private CatalogAbout about;

  @SerializedName("id")
  private String id;

  @SerializedName("extra")
  private Extra extra;

  @SerializedName("locale")
  private LocaleData locale;

  @SerializedName("generic_data")
  private GenericData genericData;

  public CatalogAbout getAbout()
  {
    return about;
  }

  public void setAbout( CatalogAbout about )
  {
    this.about = about;
  }

  public String getId() {
    return id;
  }

  public Extra getExtra() { return extra; }

  public LocaleData getLocale() { return locale; }

  public GenericData getGenericData() {
    return genericData;
  }
}
