package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

public class FlashcardView {
    @NonNull
    public final CharSequence title;

    public final boolean checked;

    public final int percentSuccess;

    public FlashcardView(@NonNull CharSequence _title, boolean _checked, int _percentSuccess) {
        title = _title;
        checked = _checked;
        percentSuccess = _percentSuccess;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        FlashcardView that = (FlashcardView) o;

        if (checked != that.checked) return false;
        if (percentSuccess != that.percentSuccess) return false;
        return title.equals(that.title);
    }

    @Override
    public int hashCode() {
        int result = title.hashCode();
        result = 31 * result + (checked ? 1 : 0);
        result = 31 * result + percentSuccess;
        return result;
    }
}
