package com.paragon_software.odapi_ui;

import android.os.Bundle;
import androidx.annotation.NonNull;

abstract class TranslateResult extends Result {
    private static final String RESULT_TRANSLATE = "translate";
    private static final String EXTRA_MODE = "mode";

    @NonNull
    private final String mMode;

    TranslateResult(@NonNull String mode) {
        super(RESULT_TRANSLATE);
        mMode = mode;
    }

    @Override
    void setResult(@NonNull Bundle responseBundle) {
        super.setResult(responseBundle);
        responseBundle.putString(EXTRA_MODE, mMode);
    }
}
