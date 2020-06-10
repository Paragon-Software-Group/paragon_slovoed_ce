package com.paragon_software.flash_cards_manager;

public interface CardsChangedListener extends FlashcardManagerAPI.Notifier {
     void onCardsChanged();
}
