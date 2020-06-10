package com.paragon_software.flash_cards_manager;

import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.fragment.app.Fragment;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.style.ImageSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.paragon_software.flash_cards_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed_ui_common.fragments.SimpleDialog;

public class FlashCardsFragment extends Fragment
                                implements CollectionView.OnItemRangeChanged,
                                           SimpleDialog.Target {
    private static final String DELETE_ALL_DIALOG_TAG = "com.paragon_software.flash_cards_manager.FlashCardsFragment.DELETE_ALL_DIALOG_TAG";
    private static final String ALL_CARDS_DESELECTED_TAG = "com.paragon_software.flash_cards_manager.FlashCardsFragment.ALL_CARDS_DESELECTED_TAG";

    private RecyclerView mRecyclerView;
    private TextView mEmptyView;

    @Nullable
    private AbstractFlashcardCollectionView mCollection = null;

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_flashcards, container, false);
        mRecyclerView = view.findViewById(R.id.information_list);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(getContext()));
        mRecyclerView.setAdapter(new Adapter());
        mEmptyView = view.findViewById(R.id.empty_view);
        mEmptyView.setText(getDeleteAllText());
        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        if(mCollection != null)
            mCollection.unregisterListener(this);
        mCollection = FlashcardManagerHolder.get().getFlashcardCollectionView();
        mCollection.registerListener(this);
        onItemRangeChanged(CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 0, mCollection.getCount());
    }

    @Override
    public void onPause() {
        super.onPause();
        if(mCollection != null)
            mCollection.unregisterListener(this);
        mCollection = null;
        onItemRangeChanged(CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 0, 0);
    }

    @Override
    public void onItemRangeChanged(CollectionView.OPERATION_TYPE type, int startPosition, int itemCount) {
        boolean empty = false;
        if(mCollection != null)
            empty = (mCollection.getCount() <= 0);
        mRecyclerView.setVisibility(empty ? View.GONE : View.VISIBLE);
        mEmptyView.setVisibility(empty ? View.VISIBLE : View.GONE);
    }

    @Override
    public void onCreateSimpleDialog(@Nullable String tag, @NonNull Dialog dialog, @Nullable Bundle extra) { }

    @Override
    public void onSimpleDialogButtonPressed(@Nullable String tag, int n, @Nullable Bundle extra) {
        if((mCollection != null) && (DELETE_ALL_DIALOG_TAG.equals(tag)) && (n == DialogInterface.BUTTON_POSITIVE))
            while (mCollection.getCount() > 0)
                mCollection.remove(mCollection.getCount() - 1);
    }

    @NonNull
    private CharSequence getDeleteAllText() {
        CharSequence res = "";
        Context context = getContext();
        if(context != null) {
            final String placeholder = "###";
            String string = getString(R.string.flash_cards_manager_ui_no_flashcards, getString(R.string.utils_slovoed_ui_add_to_flashcards_hint));
            Spannable spannable = new SpannableString(string);
            Drawable drawable = AppCompatResources.getDrawable(context, R.drawable.icn_flashcards_greystroke);
            if(drawable != null) {
                int imageWidth = drawable.getIntrinsicWidth();
                int imageHeight = drawable.getIntrinsicHeight();
                int height = getResources().getDimensionPixelOffset(R.dimen.empty_view_font_size);
                int width = height;
                if((imageWidth > 0) && (imageHeight > 0))
                    width = imageWidth * height / imageHeight;
                drawable.setBounds(0, 0, width, height);
                ImageSpan span = new ImageSpan(drawable, ImageSpan.ALIGN_BASELINE);
                int index1 = string.indexOf(placeholder);
                int index2 = index1 + placeholder.length();
                spannable.setSpan(span, index1, index2, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                res = spannable;
            }
        }
        return res;
    }

    private class StartQuizListener implements View.OnClickListener {
        @Override
        public void onClick(View v) {
            FlashcardManagerAPI flashcardManager = FlashcardManagerHolder.get();
            flashcardManager.randomizeQuiz();
            AbstractQuizCollectionView quizCollectionView = flashcardManager.getQuizCollectionView();
            if(quizCollectionView.getCount() > 0) {
                Context context = v.getContext();
                context.startActivity(new Intent(context, FcQuizActivity.class));
            }
            else {
                SimpleDialog.show(FlashCardsFragment.this, ALL_CARDS_DESELECTED_TAG,
                        new SimpleDialog.Builder()
                            .setMessage( getString(R.string.flash_cards_manager_ui_all_flashcards_deselected, getString(R.string.flash_cards_manager_ui_select_cards_title)))
                            .setPositiveText(getString(R.string.utils_slovoed_ui_common_ok)));
            }
        }
    }

    private class DeleteAllCardsListener implements View.OnClickListener {
        @Override
        public void onClick(View v) {
            SimpleDialog.show(FlashCardsFragment.this, DELETE_ALL_DIALOG_TAG,
                    new SimpleDialog.Builder()
                            .setMessage(getString(R.string.flash_cards_manager_ui_really_want_to_delete_all_flashcards))
                            .setPositiveText(getString(R.string.flash_cards_manager_ui_delete))
                            .setNegativeText(getString(R.string.utils_slovoed_ui_common_cancel)));
        }
    }

    private class Adapter extends RecyclerView.Adapter<ViewHolder> {
        @NonNull
        @Override
        public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.recycler_item, parent, false);
            return new ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
            switch (position) {
                case 0:
                    holder.setText(R.string.flash_cards_manager_ui_start_quiz_title,
                                   R.string.flash_cards_manager_ui_start_quiz_subtitle);
                    holder.setClickListener(new StartQuizListener());
                    break;

                case 1:
                    holder.setText(R.string.flash_cards_manager_ui_select_cards_title,
                                   R.string.flash_cards_manager_ui_select_cards_subtitle);
                    holder.setClickListener(new SelectCardsListener());
                    break;

                case 2:
                    holder.setText(R.string.flash_cards_manager_ui_delete_all_title,
                                   R.string.flash_cards_manager_ui_delete_all_subtitle);
                    holder.setClickListener(new DeleteAllCardsListener());
                    break;
            }
        }

        @Override
        public int getItemCount() {
            return 3;
        }
    }

    private static class ViewHolder extends RecyclerView.ViewHolder {
        private final View view;
        private final TextView title, subtitle;

        ViewHolder( View itemView ) {
            super(itemView);
            view = itemView;
            title = itemView.findViewById(android.R.id.text1);
            subtitle = itemView.findViewById(android.R.id.text2);
        }

        void setText(@StringRes int titleId, @StringRes int subtitleId) {
            title.setText(titleId);
            subtitle.setText(subtitleId);
        }

        void setClickListener(View.OnClickListener listener) {
            view.setOnClickListener(listener);
        }
    }

    private static class SelectCardsListener implements View.OnClickListener {
        @Override
        public void onClick(View v) {
            Context context = v.getContext();
            context.startActivity(new Intent(context, FcSelectActivity.class));
        }
    }
}
