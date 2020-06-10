package com.paragon_software.toolbar_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.Toolbar;
import android.view.View;

public interface ToolbarFactory {
    @Nullable
    Toolbar inflateToolbar(@NonNull View parent);
}
