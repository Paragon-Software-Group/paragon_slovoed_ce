package com.paragon_software.utils_slovoed.pdahpc;

import com.google.gson.annotations.SerializedName;

import java.util.Map;

public class About {
  @SerializedName("app_about")
  private Map<String, String> appAboutMap;

  @SerializedName("product_abouts")
  private Map<String, Map<String, String>> productAbout;

  public Map<String, String> getAppAboutMap() {
    return appAboutMap;
  }

  public Map<String, Map<String, String>> getProductAbout() {
    return productAbout;
  }
}
