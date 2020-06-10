package com.paragon_software.flash_cards_manager;

import android.content.Context;
import androidx.annotation.ColorRes;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.paragon_software.flash_cards_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionViewRecyclerAdapter;

class FlashCardListAdapter extends CollectionViewRecyclerAdapter<AbstractFlashcardCollectionView, FlashCardListAdapter.ViewHolder> {
    interface LongClickListener {
        boolean onLongClick(int position);
    }

    @NonNull
    private final LongClickListener mLongClickListener;

    FlashCardListAdapter(@NonNull LongClickListener longClickListener) {
        mLongClickListener = longClickListener;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View card = LayoutInflater.from(parent.getContext()).inflate(R.layout.flashcard_list_item, parent, false);
        return new ViewHolder(card);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        AbstractFlashcardCollectionView data = getData();
        if(data != null)
            holder.populate(data.getItem(position));
    }

    @ColorRes
    private static int fromPercentage(int percentage) {
        int color;
        if(percentage < 1)
            color = R.color.fc_gray1;
        else if(percentage < 50)
            color = R.color.fc_red;
        else if(percentage < 80)
            color = R.color.fc_orange;
        else
            color = R.color.fc_green;
        return color;
    }

    class ViewHolder extends RecyclerView.ViewHolder
                     implements View.OnClickListener, View.OnLongClickListener {
        private final ImageView checkBox;
        private final TextView text, percent;

        ViewHolder(View itemView) {
            super(itemView);
            checkBox = itemView.findViewById(R.id.checkbox);
            text = itemView.findViewById(R.id.text);
            percent = itemView.findViewById(R.id.percent);
            itemView.setOnClickListener(this);
            itemView.setOnLongClickListener(this);
        }

        void populate(FlashcardView flashcard) {
            AbstractFlashcardCollectionView data = getData();
            checkBox.setVisibility(flashcard.checked ? View.VISIBLE : View.INVISIBLE);
            text.setText(flashcard.title);
            Context context = percent.getContext();
            percent.setText(context.getString(R.string.flash_cards_manager_ui_percent, flashcard.percentSuccess));
            percent.setTextColor(
                    context.getResources().getColor(fromPercentage(flashcard.percentSuccess)));
            if(data != null)
                checkBox.setImageResource(
                        FlashcardMetadata.Mode.Regular.equals(((FlashcardMetadata)data.getMetadata()).mode)
                                ? R.drawable.ic_tick_with_margins : R.drawable.ic_tick_in_circle);
        }

        @Override
        public void onClick(View v) {
            AbstractFlashcardCollectionView data = getData();
            if(data != null) {
                int position = getAdapterPosition();
                FlashcardView card = data.getItem(position);
                if (card != null)
                    data.checkCard(position, !card.checked);
            }
        }

        @Override
        public boolean onLongClick(View v) {
            return mLongClickListener.onLongClick(getAdapterPosition());
        }
    }
}
