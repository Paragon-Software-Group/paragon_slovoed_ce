package com.paragon_software.odapi_ui;

import android.graphics.Bitmap;
import android.os.Bundle;
import androidx.annotation.NonNull;

import java.io.ByteArrayOutputStream;

class ImageTranslateResult extends TranslateResult {
    private static final String EXTRA_VALUE = "value";
    private static final int IMAGE_QUALITY = 90;

    @NonNull
    private final Bitmap mImage;

    ImageTranslateResult(@NonNull String mode, @NonNull Bitmap image) {
        super(mode);
        mImage = image;
    }

    @Override
    void setResult(@NonNull Bundle responseBundle) {
        super.setResult(responseBundle);
        responseBundle.putByteArray(EXTRA_VALUE, getImageAsByteArray());
    }

    private byte[] getImageAsByteArray() {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        mImage.compress(Bitmap.CompressFormat.PNG, IMAGE_QUALITY, out);
        return  out.toByteArray();
    }
}
