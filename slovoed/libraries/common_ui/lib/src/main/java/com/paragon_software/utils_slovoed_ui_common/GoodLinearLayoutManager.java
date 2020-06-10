package com.paragon_software.utils_slovoed_ui_common;

import android.content.Context;
import androidx.recyclerview.widget.LinearLayoutManager;
import android.util.AttributeSet;

//
//    https://stackoverflow.com/questions/30220771/recyclerview-inconsistency-detected-invalid-item-position
//
public class GoodLinearLayoutManager extends LinearLayoutManager {
    public GoodLinearLayoutManager(Context context) {
        super(context);
    }

    public GoodLinearLayoutManager(Context context, int orientation, boolean reverseLayout) {
        super(context, orientation, reverseLayout);
    }

    public GoodLinearLayoutManager(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    public boolean supportsPredictiveItemAnimations() {
        return false;
    }
}
