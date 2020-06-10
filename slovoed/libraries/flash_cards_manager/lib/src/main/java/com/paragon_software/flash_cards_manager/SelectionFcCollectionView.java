package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.HashSetMultiselectCollectionView;
import com.paragon_software.utils_slovoed.collections.TransformCollectionView;
import com.paragon_software.utils_slovoed.collections.MultiselectCollectionView;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

class SelectionFcCollectionView
        extends TransformCollectionView<FlashcardView,
                                        MultiselectCollectionView.Selectable<FlashcardView>,
                                        SelectionFcCollectionView.FcSelectionTransform,
                                        BaseQuizAndFlashcardMetadata,
                                        HashSetMultiselectCollectionView<FlashcardView,
                                                                         BaseQuizAndFlashcardMetadata,
                                                                         AbstractFlashcardCollectionView>>
        implements AbstractFlashcardCollectionView {
    @NonNull
    private final AbstractFlashcardCollectionView mCore;

    SelectionFcCollectionView(@NonNull AbstractFlashcardCollectionView core, int initialSelection) {
        super(new HashSetMultiselectCollectionView<>(core, false),
              new FcSelectionTransform());
        getCore().select(initialSelection);
        mCore = core;
    }

    @Override
    public FlashcardMetadata getMetadata() {
        FlashcardMetadata res = (FlashcardMetadata) super.getMetadata();
        return new FlashcardMetadata(res.sortMode,
                                     FlashcardMetadata.Mode.Selection,
                                     new ButtonState(VisibilityState.enabled,
                                                     CheckState.uncheckable));
    }

    @Override
    public void checkCard(int position, boolean checked) {
        getCore().select(position, checked);
    }

    @Override
    public void checkAll(boolean checked) {
        getCore().selectAll(checked);
    }

    @Override
    public void remove(int position) {
        mCore.remove(position);
    }

    @Override
    public void setSortMode(@NonNull FlashcardMetadata.SortMode sortMode) {
        mCore.setSortMode(sortMode);
    }

    @Override
    public void activateRegularMode() {
        throw new UnsupportedOperationException();
    }

    @Override
    public void activateSelectionMode(int position) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int getCheckedCount() {
        return getCore().getSelectionCount();
    }

    static class FcSelectionTransform
            implements TransformCollectionView.TransformFunction<FlashcardView,
                                                                 MultiselectCollectionView.Selectable<FlashcardView>> {
        @Override
        public FlashcardView transform(int position, MultiselectCollectionView.Selectable<FlashcardView> argument) {
            return new FlashcardView(argument.item.title,
                                     argument.isSelected,
                                     argument.item.percentSuccess);
        }

        @Override
        public void onItemRangeChanged(OPERATION_TYPE type, int startPosition, int itemCount) { }
    }
}
