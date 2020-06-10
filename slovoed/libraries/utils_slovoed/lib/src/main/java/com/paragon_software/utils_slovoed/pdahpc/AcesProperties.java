package com.paragon_software.utils_slovoed.pdahpc;

import com.google.gson.annotations.SerializedName;

import java.util.Map;

public final class AcesProperties {
    @SerializedName("environmentUrl")
    private String environmentUrl;

    @SerializedName("platformId")
    private String platformId;

    @SerializedName("xApiKey")
    private String xApiKey;

    @SerializedName("licenseMap")
    private Map<String, String> licenseMap;

    public String getEnvironmentUrl() { return environmentUrl; }
    public String getPlatformId() { return platformId; }
    public String getXApiKey() { return xApiKey; }
    public Map<String, String> getLicenseMap() { return licenseMap; }
}
