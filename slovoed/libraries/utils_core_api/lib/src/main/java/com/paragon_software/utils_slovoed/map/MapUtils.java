package com.paragon_software.utils_slovoed.map;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.LinkedHashMap;
import java.util.Map;

public final class MapUtils {
    @NonNull
    public static Map<String, String> newMap(@NonNull String... values) {
        final int n = values.length;
        final Map<String, String> res = new LinkedHashMap<>(n / 2);
        if((n % 2) != 0)
            throw new IllegalArgumentException();
        for(int i = 0; i < n; i += 2)
            res.put(values[i], values[i + 1]);
        return res;
    }

    public static boolean isEmpty(@Nullable Map<?, ?> map) {
        boolean res = (map == null);
        if(!res)
            res = map.isEmpty();
        return res;
    }

    private MapUtils() {}
}
