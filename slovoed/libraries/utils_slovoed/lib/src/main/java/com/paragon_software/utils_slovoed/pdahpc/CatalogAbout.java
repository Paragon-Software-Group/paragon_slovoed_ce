package com.paragon_software.utils_slovoed.pdahpc;

import java.util.Map;

import com.google.gson.annotations.SerializedName;


public class CatalogAbout
{
  @SerializedName("copyright")
  private Map< String, String > copyrightMap;

  @SerializedName("name")
  private Map< String, String > nameMap;

  @SerializedName("web")
  private Map< String, String > webMap;

  @SerializedName("provided")
  private Map< String, String > providedMap;

  @SerializedName("faq")
  private Map< String, String > faqMap;

  @SerializedName("support")
  private Map<String, String> supportEmailMap;

  public Map< String, String > getCopyrightMap()
  {
    return copyrightMap;
  }

  public Map<String, String> getNameMap() {
    return nameMap;
  }

  public Map<String, String> getWebMap() {
    return webMap;
  }

  public Map<String, String> getFaqMap() {
    return faqMap;
  }

  public Map<String, String> getSupportEmailMap() {
    return supportEmailMap;
  }

  public Map<String, String> getProvidedMap() {
    return providedMap;
  }
}
