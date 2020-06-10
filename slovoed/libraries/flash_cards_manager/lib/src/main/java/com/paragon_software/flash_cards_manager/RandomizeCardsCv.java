package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.collections.FilterCollectionView;
import com.paragon_software.utils_slovoed.collections.MutableCollectionView;
import com.paragon_software.utils_slovoed.collections.SortCollectionView;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Random;
import java.util.Set;

class RandomizeCardsCv extends SortCollectionView<FlashcardBox,
                                                  BaseQuizAndFlashcardMetadata,
                                                  RandomizeCardsCv.PickedCv>
                       implements MutableCollectionView<FlashcardBox,
                                                        BaseQuizAndFlashcardMetadata> {
    private static final int MAX_CARDS = 20;

    RandomizeCardsCv(@NonNull MutableCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> flashcards) {
        this(flashcards, new CardPicker(randomize(flashcards)));
    }

    private RandomizeCardsCv(@NonNull MutableCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> flashcards,
                             @NonNull CardPicker cardPicker) {
        super(new PickedCv(flashcards, cardPicker), cardPicker);
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
    private static Map<FlashcardBox, Integer> randomize(@NonNull CollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> flashcards) {
        Random random = new Random();
        int n = Math.min(flashcards.getCount(), MAX_CARDS);
        Map<FlashcardBox, Integer> res = new HashMap<>(n);
        Set<Integer> generatedIndices = new HashSet<>(n);
        for(int i = 0; i < n; i++) {
            int index;
            do
                index = random.nextInt(flashcards.getCount());
            while (generatedIndices.contains(index));
            generatedIndices.add(index);
            res.put(flashcards.getItem(index), i);
        }
        return res;
    }

    static class PickedCv extends FilterCollectionView<FlashcardBox,
                                                       BaseQuizAndFlashcardMetadata,
                                                       MutableCollectionView<FlashcardBox,
                                                                             BaseQuizAndFlashcardMetadata>>
                          implements MutableCollectionView<FlashcardBox,
                                                           BaseQuizAndFlashcardMetadata> {

        private PickedCv(@NonNull MutableCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> core, @NonNull Predicate<FlashcardBox> predicate) {
            super(core, predicate);
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
    }

    private static class CardPicker extends AlphabeticalFcBoxComparator
                                    implements FilterCollectionView.Predicate<FlashcardBox> {
        @NonNull
        private final Map<FlashcardBox, Integer> selectedCards;

        CardPicker(@NonNull Map<FlashcardBox, Integer> _selectedCards) {
            selectedCards = _selectedCards;
        }

        @Override
        public int compare(FlashcardBox o1, FlashcardBox o2) {
            int res;
            Integer weight1 = selectedCards.get(o1), weight2 = selectedCards.get(o2);
            if((weight1 != null) && (weight2 != null))
                res = weight1 - weight2;
            else if((weight1 != null) || (weight2 != null))
                res = (weight1 != null) ? -1 : 1;
            else
                res = super.compare(o1, o2);
            return res;
        }

        @Override
        public boolean apply(FlashcardBox flashcardBox) {
            return selectedCards.containsKey(flashcardBox);
        }
    }
}
