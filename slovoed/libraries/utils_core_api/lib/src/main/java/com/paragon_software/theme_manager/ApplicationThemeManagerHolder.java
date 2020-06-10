package com.paragon_software.theme_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class ApplicationThemeManagerHolder {

    @Nullable
    private static ApplicationThemeManagerAPI mManager = null;

    @NonNull
    public static ApplicationThemeManagerAPI getManager() throws IllegalStateException {
        if (null == mManager) {
            throw new IllegalStateException("ApplicationThemeManagerHolder uninitialized");
        }
        return mManager;
    }

    public static void setManager(@NonNull ApplicationThemeManagerAPI manager) {
        mManager = manager;
    }

}
