package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.collections.MutableCollectionView;
import com.paragon_software.utils_slovoed.collections.SortCollectionView;

import java.util.Comparator;

class SortCardsCv extends SortCollectionView<FlashcardBox,
                                             BaseQuizAndFlashcardMetadata,
                                             MutableCollectionView<FlashcardBox,
                                                                   BaseQuizAndFlashcardMetadata>>
                  implements MutableCollectionView<FlashcardBox,
                                                   BaseQuizAndFlashcardMetadata> {
    @NonNull
    final FlashcardMetadata.SortMode sortMode;

    public SortCardsCv(@NonNull MutableCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> _core, @NonNull FlashcardMetadata.SortMode _sortMode) {
        super(_core, createComparator(_sortMode));
        sortMode = _sortMode;
    }

    @Override
    public void update(int position, FlashcardBox flashcardBox) {
        getCore().update(getCoreIndex(position), flashcardBox);
    }

    @Override
    public void remove(int position) {
        getCore().remove(getCoreIndex(position));
    }

    @Override
    public void updateMetadata(@Nullable BaseQuizAndFlashcardMetadata metadata) {
        getCore().updateMetadata(metadata);
    }

    @NonNull
    private static Comparator<FlashcardBox> createComparator(@NonNull FlashcardMetadata.SortMode sortMode) {
        Comparator<FlashcardBox> res;
        if(sortMode.equals(FlashcardMetadata.SortMode.Alphabetical))
            res = new AlphabeticalFcBoxComparator();
        else
            res = new ProgressComparator();
        return res;
    }

    private static class ProgressComparator extends AlphabeticalFcBoxComparator {
        @Override
        public int compare(FlashcardBox o1, FlashcardBox o2) {
            int res = o1.getPercentSuccess() - o2.getPercentSuccess();
            if(res == 0)
                res = super.compare(o1, o2);
            return res;
        }
    }
}
