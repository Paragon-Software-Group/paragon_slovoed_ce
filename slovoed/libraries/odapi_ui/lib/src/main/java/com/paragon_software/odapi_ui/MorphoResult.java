package com.paragon_software.odapi_ui;

import android.os.Bundle;
import androidx.annotation.NonNull;

import java.io.Serializable;

class MorphoResult extends Result {
    private static final String RESULT_MORPHOLOGY = "morphology";
    private static final String EXTRA_VALUE = "value";

    @NonNull
    private final String[] mBaseForms;

    MorphoResult(@NonNull String[] baseForms) {
        super(RESULT_MORPHOLOGY);
        mBaseForms = baseForms;
    }

    @Override
    void setResult(@NonNull Bundle responseBundle) {
        super.setResult(responseBundle);
        responseBundle.putSerializable(EXTRA_VALUE, getWordsForBundle());
    }

    private Serializable getWordsForBundle() {
        String[][] words = new String[mBaseForms.length][];
        for(int i = 0; i < mBaseForms.length; i++)
            words[i] = new String[] { mBaseForms[i] };
        return words;
    }
}
