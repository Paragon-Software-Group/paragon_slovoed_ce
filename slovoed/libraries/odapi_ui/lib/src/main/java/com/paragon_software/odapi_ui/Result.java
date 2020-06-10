package com.paragon_software.odapi_ui;

import android.os.Bundle;
import androidx.annotation.NonNull;

abstract class Result {
    private static final String EXTRA_RESULT = "result";

    @NonNull
    private final String mResult;

    Result(@NonNull String result) {
        mResult = result;
    }

    void setResult(@NonNull Bundle responseBundle) {
        responseBundle.putString(EXTRA_RESULT, mResult);
    }
}
