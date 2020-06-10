package com.paragon_software.dictionary_manager;


import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.text.LocalizedString;

public interface LanguageStrings {
    @NonNull
    LocalizedString getSingle();
    @NonNull LocalizedString getPlural();
    @NonNull LocalizedString getDictionaries();
    @NonNull LocalizedString getDirectionFrom();
    @NonNull LocalizedString getDirectionTo();
}
