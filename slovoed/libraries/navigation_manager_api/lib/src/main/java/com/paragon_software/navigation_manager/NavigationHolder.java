package com.paragon_software.navigation_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public final class NavigationHolder {
    @Nullable
    private static NavigationManagerApi mNavigationManager = null;

    @NonNull
    static NavigationManagerApi getNavigationManager() {
        if(mNavigationManager != null)
            return mNavigationManager;
        else
            throw new IllegalStateException("navigation manager not created");
    }

    public static void setNavigationManager(@NonNull NavigationManagerApi navigationManager) {
        if(mNavigationManager == null)
            mNavigationManager = navigationManager;
        else
            throw new IllegalStateException("navigation manager already set");
    }
}
