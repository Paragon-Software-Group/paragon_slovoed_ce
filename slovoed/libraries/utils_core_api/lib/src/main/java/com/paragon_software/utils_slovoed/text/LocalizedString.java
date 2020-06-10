package com.paragon_software.utils_slovoed.text;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;

import java.util.concurrent.atomic.AtomicReference;

public abstract class LocalizedString {
    private static final AtomicReference<Context> CONTEXT_REF = new AtomicReference<>(null);
    private static final LocalizedString EMPTY = from("");

    public static LocalizedString empty() {
        return EMPTY;
    }

    public static LocalizedString from(@NonNull final String string) {
        return new LocalizedString() {
            @NonNull
            @Override
            public String get() {
                return string;
            }
        };
    }

    public static LocalizedString from(Context context, @StringRes final int stringId) {
        CONTEXT_REF.compareAndSet(null, context.getApplicationContext());
        return new LocalizedString() {
            @NonNull
            @Override
            public String get() {
                return CONTEXT_REF.get().getString(stringId);
            }
        };
    }

    @NonNull
    public static LocalizedString nonNull(@Nullable LocalizedString string) {
        return (string != null) ? string : empty();
    }

    @NonNull
    public abstract String get();
}
