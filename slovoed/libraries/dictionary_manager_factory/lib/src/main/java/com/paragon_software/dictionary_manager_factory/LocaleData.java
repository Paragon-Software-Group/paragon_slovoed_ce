package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

final class LocaleData {
    @SerializedName("default")
    private String default_;

    @SerializedName("use_system")
    private String useSystem;

    String getDefault() {
        return default_;
    }

    String getUseSystem() {
        return useSystem;
    }
}
