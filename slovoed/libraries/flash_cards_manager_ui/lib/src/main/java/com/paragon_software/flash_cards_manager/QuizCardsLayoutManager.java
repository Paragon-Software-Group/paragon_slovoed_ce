package com.paragon_software.flash_cards_manager;

import android.content.Context;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.paragon_software.flash_cards_manager_ui.R;

class QuizCardsLayoutManager extends GridLayoutManager {
    private final int mInitialMaxCards;

    QuizCardsLayoutManager(Context context) {
        this(context, context.getResources().getInteger(R.integer.max_cards_in_row));
    }

    private QuizCardsLayoutManager(Context context, int maxCards) {
        super(context, maxCards);
        mInitialMaxCards = maxCards;
    }

    @Override
    public void onAdapterChanged(RecyclerView.Adapter oldAdapter, RecyclerView.Adapter newAdapter) {
        super.onAdapterChanged(oldAdapter, newAdapter);
        updateColumnCount();
    }

    @Override
    public void onItemsAdded(RecyclerView recyclerView, int positionStart, int itemCount) {
        super.onItemsAdded(recyclerView, positionStart, itemCount);
        updateColumnCount();
    }

    @Override
    public void onItemsRemoved(RecyclerView recyclerView, int positionStart, int itemCount) {
        super.onItemsRemoved(recyclerView, positionStart, itemCount);
        updateColumnCount();
    }

    private void updateColumnCount() {
        if ( getItemCount() > 0 ) {
            setSpanCount(Math.min(mInitialMaxCards, getItemCount()));
        }
    }
}
