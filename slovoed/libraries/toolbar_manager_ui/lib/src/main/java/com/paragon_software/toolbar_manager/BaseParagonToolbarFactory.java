package com.paragon_software.toolbar_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;

import com.paragon_software.toolbar_manager_ui.R;

public class BaseParagonToolbarFactory implements ToolbarFactory {
    @Nullable
    @Override
    public Toolbar inflateToolbar(@NonNull View parent) {
        Toolbar res = null;
        ViewGroup viewGroup = null;
        if(parent instanceof ViewGroup)
            viewGroup = (ViewGroup) parent;
        else
            for(ViewParent p = parent.getParent(); p != null; p = p.getParent())
                if(p instanceof ViewGroup) {
                    viewGroup = (ViewGroup) p;
                    break;
                }
        View inflated = LayoutInflater.from(parent.getContext()).inflate(R.layout.toolbar, viewGroup, false);
        if(inflated instanceof Toolbar)
            res = (Toolbar) inflated;
        return res;
    }
}
