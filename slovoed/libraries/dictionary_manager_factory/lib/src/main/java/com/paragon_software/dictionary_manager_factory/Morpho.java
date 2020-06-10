package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

class Morpho
{
  @SerializedName( "size" )
  private String size;

  @SerializedName( "id" )
  private String id;

  @SerializedName( "sdc_id" )
  private String sdcId;

  @SerializedName( "lang" )
  private String lang;

  @SerializedName( "lang_short" )
  private String langShort;

  @SerializedName( "url" )
  private String url;

  String getSize()
  {
    return size;
  }

  public String getSdcId()
  {
    return sdcId;
  }

  public String getLang()
  {
    return lang;
  }

  public String getLangShort()
  {
    return langShort;
  }

  public String getUrl()
  {
    return url;
  }

  public String getId() {
    return id;
  }
}
