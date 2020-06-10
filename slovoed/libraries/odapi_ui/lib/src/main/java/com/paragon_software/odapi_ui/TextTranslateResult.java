package com.paragon_software.odapi_ui;

import android.os.Bundle;
import androidx.annotation.NonNull;

class TextTranslateResult extends TranslateResult {
    private static final String EXTRA_VALUE = "value";

    @NonNull
    private final String mHtml;

    @NonNull
    private final String mText;

    TextTranslateResult(@NonNull String mode, @NonNull String html, @NonNull String text) {
        super(mode);
        mHtml = html;
        mText = text;
    }

    @Override
    void setResult(@NonNull Bundle responseBundle) {
        super.setResult(responseBundle);
        responseBundle.putStringArray(EXTRA_VALUE, new String[] {mHtml, mText});
    }
}
