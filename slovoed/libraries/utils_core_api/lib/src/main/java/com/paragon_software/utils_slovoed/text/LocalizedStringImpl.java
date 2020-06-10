package com.paragon_software.utils_slovoed.text;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public abstract class LocalizedStringImpl<Key> extends LocalizedString {
    @NonNull
    @Override
    public String get() {
        String res = getStringByKeyHelper(getCurrentLocale());
        if (res == null)
            res = getStringByKeyHelper(getDefaultLocale());
        if (res == null)
            res = getStringByKeyHelper(getEnglishLocale());
        if (res == null)
            res = getAnyString();
        if (res == null)
            res = "";
        return res;
    }

    @Nullable
    protected abstract Key getCurrentLocale();

    @Nullable
    protected abstract Key getDefaultLocale();

    @Nullable
    protected abstract Key getEnglishLocale();

    @Nullable
    protected abstract String getStringByKey(@NonNull Key key);

    @Nullable
    protected abstract String getAnyString();

    @Nullable
    private String getStringByKeyHelper(@Nullable Key key) {
        String res = null;
        if (key != null)
            res = getStringByKey(key);
        return res;
    }
}
