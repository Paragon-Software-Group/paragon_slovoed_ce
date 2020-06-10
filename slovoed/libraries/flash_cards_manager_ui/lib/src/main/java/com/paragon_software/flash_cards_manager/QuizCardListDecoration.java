package com.paragon_software.flash_cards_manager;

import android.graphics.Rect;
import androidx.recyclerview.widget.RecyclerView;
import android.view.View;

import com.paragon_software.flash_cards_manager_ui.R;

class QuizCardListDecoration extends RecyclerView.ItemDecoration {
    private int margin = -1;

    @Override
    public void getItemOffsets(Rect outRect, View view, RecyclerView parent, RecyclerView.State state) {
        if(margin < 0)
            margin = parent.getResources().getDimensionPixelSize(R.dimen.card_circe_margin);
        outRect.set(margin, margin, margin, margin);
    }
}
