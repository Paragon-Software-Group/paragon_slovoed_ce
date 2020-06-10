package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

import java.util.List;
import java.util.regex.Pattern;

public final class GenericData {
    static final String PROMISE = "promise";
    static final String FREE = "free";
    static final Pattern FULLPACK_PATTERN
            = Pattern.compile("\\s*((\\d+\\s*-\\s*\\d+)|(\\d+))\\s+bought\\s+((\\d+,\\s*)*\\d+)\\s*");

    @SerializedName("content")
    private List<String> content;

    @SerializedName("fullpack")
    private String fullpack;

    @SerializedName("demo_fts")
    private String demoFts;

    @SerializedName("promiseFTSinDemo")
    private String promiseFTSinDemo;

    List<String> getContent() {
        return content;
    }

    String getFullpack() { return fullpack; }

    boolean isDemoFts()  {
        return PROMISE.equals(demoFts);
    }

    boolean isPromiseFTSinDemo() {
        return Boolean.parseBoolean(promiseFTSinDemo);
    }
}
