package com.paragon_software.article_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.flash_cards_manager.FlashcardManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

class FlashcardArticleController extends ArticleController {
    protected FlashcardArticleController(@NonNull ArticleManagerAPI _articleManager, @NonNull String _controllerId, @Nullable DictionaryManagerAPI _dictionaryManager, @Nullable SoundManagerAPI _soundManager, @Nullable FavoritesManagerAPI _favoritesManager, @Nullable HistoryManagerAPI _historyManager, @Nullable SettingsManagerAPI _settingsManager, @Nullable EngineArticleAPI _engine, @Nullable FlashcardManagerAPI _flashcardManager, @Nullable HintManagerAPI _hintManager ) {
        super(_articleManager, _controllerId, _dictionaryManager, _soundManager, _favoritesManager, _historyManager, _settingsManager, _engine, _flashcardManager, _hintManager);
    }

    private static final ButtonState GONE_BUTTON = new ButtonState(VisibilityState.gone, CheckState.uncheckable);

    static ArticleControllerAPI create( @NonNull ArticleManagerAPI _articleManager, @NonNull String _controllerId,
                                        @Nullable DictionaryManagerAPI _dictionaryManager,
                                        @Nullable SoundManagerAPI _soundManager,
                                        @Nullable FavoritesManagerAPI _favoritesManager,
                                        @Nullable HistoryManagerAPI _historyManager,
                                        @Nullable SettingsManagerAPI _settingsManager, @Nullable EngineArticleAPI _engine,
                                        @Nullable FlashcardManagerAPI _flashcardManager,
                                        @Nullable HintManagerAPI _hintManager )
    {
        ArticleController articleController =
                new FlashcardArticleController(_articleManager, _controllerId, _dictionaryManager, _soundManager, _favoritesManager,
                                               _historyManager, _settingsManager, _engine, _flashcardManager, _hintManager);
        initController(articleController);
        return articleController;
    }

    @NonNull
    @Override
    public VisibilityState getDemoBannerVisibility() {
        return VisibilityState.gone;
    }

    @NonNull
    @Override
    public VisibilityState getDictionaryIconVisibility() {
        return VisibilityState.gone;
    }

    @NonNull
    @Override
    public VisibilityState getDictionaryTitleVisibility() {
        return VisibilityState.gone;
    }

    @NonNull
    @Override
    public VisibilityState getSearchUIVisibility() {
        return VisibilityState.gone;
    }

    @NonNull
    @Override
    public VisibilityState getTrialStatusVisibility() {
        return VisibilityState.gone;
    }

    @NonNull
    @Override
    public VisibilityState getOpenMyDictionariesVisibility() {
        return VisibilityState.gone;
    }

    @NonNull
    @Override
    public ButtonState getBackButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getForwardButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getFavoritesButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getFlashcardButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getFindNextButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getSoundButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getFindPreviousButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getRunningHeadsButtonState(boolean leftToRight) {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getSearchUIButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getGoToHistoryButtonState() {
        return GONE_BUTTON;
    }

    @NonNull
    @Override
    public ButtonState getHideOrSwitchBlocksButtonState() {
        return GONE_BUTTON;
    }

    @Override
    public boolean isNeedCrossRef() {
        return false;
    }
}
