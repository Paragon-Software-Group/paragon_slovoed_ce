package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.CollectionView;

public interface AbstractFlashcardCollectionView extends CollectionView<FlashcardView,
    BaseQuizAndFlashcardMetadata> {
    void checkCard(int position, boolean checked);
    void checkAll(boolean checked);
    void remove(int position);
    void setSortMode(@NonNull FlashcardMetadata.SortMode sortMode);
    void activateRegularMode();
    void activateSelectionMode(int position);
    int getCheckedCount();
}
