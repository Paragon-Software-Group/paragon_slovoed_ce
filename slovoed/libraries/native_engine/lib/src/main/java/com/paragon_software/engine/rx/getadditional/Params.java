package com.paragon_software.engine.rx.getadditional;

import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;

class Params {
    @Nullable
    final Dictionary dictionary;

    Params(@Nullable Dictionary _dictionary) {
        dictionary = _dictionary;
    }
}
