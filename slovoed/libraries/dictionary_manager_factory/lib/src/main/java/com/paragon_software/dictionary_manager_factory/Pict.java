package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

public class Pict
{
  @SerializedName("prc_id")
  private String prcId;

  @SerializedName("from_lang_short")
  private String langFromShort;

  @SerializedName("size")
  private String size;

  @SerializedName("version")
  private String version;

  @SerializedName("url")
  private String url;


  public String getPrcId()
  {
    return prcId;
  }

  public String getLangFromShort()
  {
    return langFromShort;
  }

  public String getSize()
  {
    return size;
  }

  public String getVersion()
  {
    return version;
  }

  public String getUrl()
  {
    return url;
  }
}
