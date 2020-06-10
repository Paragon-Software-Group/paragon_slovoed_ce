package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

class Sound
{
  @SerializedName( "size" )
  private String size;

  @SerializedName( "prc_id" )
  private String prcId;

  @SerializedName( "lang" )
  private String lang;

  @SerializedName( "lang_short" )
  private String langShort;

  @SerializedName( "lang_iso" )
  private String langIso;

  @SerializedName( "version" )
  private String version;

  @SerializedName( "url" )
  private String url;

  String getSize()
  {
    return size;
  }

  String getPrcId()
  {
    return prcId;
  }

  String getLang()
  {
    return lang;
  }

  String getLangShort()
  {
    return langShort;
  }

  String getLangIso()
  {
    return langIso;
  }

  String getVersion()
  {
    return version;
  }

  String getUrl()
  {
    return url;
  }
}
