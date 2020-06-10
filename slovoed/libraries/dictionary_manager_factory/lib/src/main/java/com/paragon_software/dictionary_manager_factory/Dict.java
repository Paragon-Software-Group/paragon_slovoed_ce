package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

class Dict
{
  @SerializedName("prc_id")
  private String prcId;

  @SerializedName("from_lang")
  private String langFrom;

  @SerializedName("to_lang")
  private String langTo;

  @SerializedName("from_lang_short")
  private String langFromShort;

  @SerializedName("to_lang_short")
  private String langToShort;

  @SerializedName("size")
  private String size;

  @SerializedName("version")
  private String version;

  @SerializedName("url")
  private String url;

  @SerializedName("words_count")
  private String wordsCount;


  public String getPrcId()
  {
    return prcId;
  }

  public String getLangTo() {
    return langTo;
  }

  public String getLangFrom()
  {
    return langFrom;
  }

  public String getLangToShort()
  {
    return langToShort;
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

  public String getWordsCount() {
    return wordsCount;
  }
}
