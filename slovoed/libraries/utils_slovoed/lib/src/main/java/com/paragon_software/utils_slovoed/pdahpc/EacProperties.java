package com.paragon_software.utils_slovoed.pdahpc;

import com.google.gson.annotations.SerializedName;

import java.util.Map;

public final class EacProperties {
    @SerializedName("licenseMap")
    private Map<String, String> licenseMap;

    public Map<String, String> getLicenseMap() { return licenseMap; }
}
