package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.StrictProxyCollectionView;

class FlashcardCollectionView
        extends StrictProxyCollectionView<FlashcardView,
                                          BaseQuizAndFlashcardMetadata,
                                          AbstractFlashcardCollectionView>
        implements AbstractFlashcardCollectionView {
    @NonNull
    private final AbstractFlashcardCollectionView mMainCollection;

    FlashcardCollectionView( @NonNull AbstractFlashcardCollectionView core ) {
        super(core);
        mMainCollection = core;
    }

    @Override
    public void checkCard(int position, boolean checked) {
        getCore().checkCard(position, checked);
    }

    @Override
    public void checkAll(boolean checked) {
        getCore().checkAll(checked);
    }

    @Override
    public void remove(int position) {
        getCore().remove(position);
    }

    @Override
    public void setSortMode(@NonNull FlashcardMetadata.SortMode sortMode) {
        getCore().setSortMode(sortMode);
        FlashcardManagerHolder.get().saveSortMode(sortMode);
    }

    @Override
    public void activateRegularMode() {
        set(mMainCollection);
        callOnMetadataChanged();
    }

    @Override
    public void activateSelectionMode(int position) {
        set(new SelectionFcCollectionView(mMainCollection, position));
        callOnMetadataChanged();
    }

    @Override
    public int getCheckedCount() {
        return getCore().getCheckedCount();
    }
}
