package com.paragon_software.utils_slovoed.pdahpc;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.map.MapUtils;
import com.paragon_software.utils_slovoed.text.LocalizedString;
import com.paragon_software.utils_slovoed.text.LocalizedStringImpl;

import java.util.Locale;
import java.util.Map;

public class LocalizeStringHelper
{
    public static LocalizedString getLocalizedString(final Map<String, String> map, final String defaultLanguage )
    {
        return new LocalizedStringImpl<String>() {
            @NonNull
            @Override
            public String get() {
                return super.get();
            }

            @NonNull
            @Override
            protected String getCurrentLocale() {
                return Locale.getDefault().getLanguage();
            }

            @Nullable
            @Override
            protected String getDefaultLocale() {
                return defaultLanguage;
            }

            @Nullable
            @Override
            protected String getEnglishLocale() {
                return "en";
            }

            @Nullable
            @Override
            protected String getStringByKey(@NonNull String key) {
                String res = null;
                if(!MapUtils.isEmpty(map))
                    res = map.get(key);
                return res;
            }

            @Nullable
            @Override
            protected String getAnyString() {
                String res = null;
                if(!MapUtils.isEmpty(map))
                    res = map.values().iterator().next();
                return res;
            }
        };
    }
}
