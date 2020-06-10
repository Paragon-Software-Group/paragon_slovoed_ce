package com.paragon_software.odapi_ui;

import android.os.Bundle;
import androidx.annotation.NonNull;

abstract class ErrorResult extends Result {
    private static final String RESULT_ERROR = "error";
    private static final String EXTRA_VALUE = "value";

    @NonNull
    private final String mError;

    @NonNull
    private final String mMessage;

    ErrorResult(@NonNull String error, @NonNull String message) {
        super(RESULT_ERROR);
        mError = error;
        mMessage = message;
    }

    @Override
    void setResult(@NonNull Bundle responseBundle) {
        super.setResult(responseBundle);
        responseBundle.putStringArray(EXTRA_VALUE, new String[] {mError, mMessage});
    }

    @NonNull
    String getError() {
        return mError;
    }

    @NonNull
    String getMessage() {
        return mMessage;
    }
}
