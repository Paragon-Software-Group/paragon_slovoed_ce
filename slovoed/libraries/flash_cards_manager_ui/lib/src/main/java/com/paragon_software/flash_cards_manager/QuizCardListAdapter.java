package com.paragon_software.flash_cards_manager;

import android.content.res.ColorStateList;
import androidx.annotation.ColorRes;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.paragon_software.flash_cards_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionViewRecyclerAdapter;

class QuizCardListAdapter extends CollectionViewRecyclerAdapter<AbstractQuizCollectionView,
                                                                QuizCardListAdapter.ViewHolder> {
    static QuizCardListAdapter create(@NonNull AbstractQuizCollectionView collection) {
        QuizCardListAdapter res = new QuizCardListAdapter();
        res.setData(collection);
        return res;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        ImageView view = new ImageView(parent.getContext());
        view.setImageResource(R.drawable.card_circle);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        AbstractQuizCollectionView collection = getData();
        if(collection != null)
            holder.setColor(getColorForItem(collection, position));
    }

    @ColorRes
    private static int getColorForItem(@NonNull AbstractQuizCollectionView collection,
                                       int position) {
        @ColorRes int res = 0;
        switch (collection.getItem(position).answerType) {
            case Right:
                res = R.color.fc_green;
                break;
            case Wrong:
                res = R.color.fc_red;
                break;
            case Current:
                res = R.color.fc_blue;
                break;
            case Future:
                res = R.color.fc_gray2;
                break;
        }
        return res;
    }

    static class ViewHolder extends RecyclerView.ViewHolder {
        private ImageView view;

        ViewHolder(View itemView) {
            super(itemView);
            view = (ImageView) itemView;
        }

        void setColor(@ColorRes int colorId) {
            int color = view.getResources().getColor(colorId);
            view.setImageTintList(ColorStateList.valueOf(color));
        }
    }
}
