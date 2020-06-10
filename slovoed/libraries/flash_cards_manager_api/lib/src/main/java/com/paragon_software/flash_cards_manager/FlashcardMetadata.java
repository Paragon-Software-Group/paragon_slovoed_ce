package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.ui_states.ButtonState;

public class FlashcardMetadata implements BaseQuizAndFlashcardMetadata {
    public enum SortMode {
        Alphabetical, ByProgress
    }

    public enum Mode {
        Regular, Selection
    }

    @NonNull
    public final SortMode sortMode;

    @NonNull
    public final Mode mode;

    @NonNull
    public final ButtonState deleteButtonVisibility;

    public FlashcardMetadata(@NonNull SortMode _sortMode,
                             @NonNull Mode _mode,
                             @NonNull ButtonState _deleteButtonVisibility) {
        sortMode = _sortMode;
        mode = _mode;
        deleteButtonVisibility = _deleteButtonVisibility;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        FlashcardMetadata that = (FlashcardMetadata) o;

        if (sortMode != that.sortMode) return false;
        if (mode != that.mode) return false;
        return deleteButtonVisibility.equals(that.deleteButtonVisibility);
    }

    @Override
    public int hashCode() {
        int result = sortMode.hashCode();
        result = 31 * result + mode.hashCode();
        result = 31 * result + deleteButtonVisibility.hashCode();
        return result;
    }
}
