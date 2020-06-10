package com.paragon_software.flash_cards_manager;

import com.paragon_software.utils_slovoed.collections.CollectionView;

public interface AbstractQuizCollectionView extends CollectionView<QuizItemView, BaseQuizAndFlashcardMetadata> {
    void showMeaning();
    void answer( boolean right );
    void restart();
    void playSound();
    void remove();
    void setInQuizUI(boolean value);
}
