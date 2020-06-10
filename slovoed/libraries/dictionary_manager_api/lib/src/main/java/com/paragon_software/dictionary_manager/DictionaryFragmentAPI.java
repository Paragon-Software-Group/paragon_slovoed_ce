package com.paragon_software.dictionary_manager;

import androidx.fragment.app.Fragment;

public abstract class DictionaryFragmentAPI extends Fragment {
    public void selectMyDictionariesTab( Dictionary.DictionaryId dictionaryId ) {}
    public void showUserCoreExpiredDialog() {}
    public void showExpiresTrialDialog() {}
}
