package com.paragon_software.utils_slovoed.pdahpc;

import com.google.gson.annotations.SerializedName;

public class GenericData {

    static final String PROMISE = "promise";

    @SerializedName("demo_fts")
    private String demoFts;

    @SerializedName("promiseFTSinDemo")
    private String promiseFTSinDemo;

    public boolean isDemoFts()  {
        return PROMISE.equals(demoFts);
    }

    public boolean isPromiseFTSinDemo() {
        return Boolean.parseBoolean(promiseFTSinDemo);
    }
}
