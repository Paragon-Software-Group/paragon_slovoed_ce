package com.paragon_software.utils_slovoed.pdahpc;

import com.google.gson.annotations.SerializedName;

public final class LocaleData {
    @SerializedName("default")
    private String default_;

    @SerializedName("use_system")
    private String useSystem;

    @SerializedName("list")
    private String[] list;

    public String getDefault() {
        return default_;
    }

    public String getUseSystem() {
        return useSystem;
    }

    public String[] getList() { return list; }
}
