package com.paragon_software.navigation_manager.mock.dictionary_manager;


import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.dictionary_manager.Language;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public abstract class MockDictionaryManager extends DictionaryManagerAPI {
    private @NonNull
    final List<Dictionary> dictionaries = new ArrayList<>();
    private @NonNull
    final List<IDictionaryListObserver> observers = new LinkedList<>();
    private DictionaryAndDirection dictionaryAndDirection;


    public void registerDictionary(@NonNull Dictionary.DictionaryId id, @NonNull LocalizedString title,
                                   @NonNull LocalizedString description, @NonNull Dictionary.DICTIONARY_STATUS edition, @NonNull IDictionaryIcon icon) {
        Dictionary.Builder b = new Dictionary.Builder(id, title, description, icon);
        dictionaries.add(b.setStatus(edition).build());
        int russ = Language.getLangCodeFromShortForm("russ");
        int germ = Language.getLangCodeFromShortForm("germ");
        setDictionaryAndDirectionSelectedByUser(new DictionaryAndDirection(id, new Dictionary.Direction(russ, germ, null)));
        sendEventDictionaryListChanged();
    }

    @NonNull
    @Override
    public List<Dictionary> getDictionaries() {
        return new ArrayList<>(dictionaries);
    }

    @Override
    public Dictionary getDictionaryById(@Nullable Dictionary.DictionaryId dictionaryId) {
        Dictionary dictionary;
        for (int i = 0; i < dictionaries.size(); i++) {
            dictionary = dictionaries.get(i);
            if (dictionary.getId().equals(dictionaryId)) {
                return dictionary;
            }
        }
        return null;
    }

    @Override
    public void registerDictionaryListObserver(@NonNull IDictionaryListObserver observer) {
        if (!observers.contains(observer)) {
            observers.add(observer);
        }
    }

    @Override
    public void unRegisterDictionaryListObserver(@NonNull IDictionaryListObserver observer) {
        observers.remove(observer);
    }

    public void sendEventDictionaryListChanged() {
        for (IDictionaryListObserver observer : observers) {
            observer.onDictionaryListChanged();
        }
    }

    @Nullable
    @Override
    public DictionaryAndDirection getDictionaryAndDirectionSelectedByUser() {
        return dictionaryAndDirection;
    }

    @Override
    public void setDictionaryAndDirectionSelectedByUser(@NonNull DictionaryAndDirection dictionaryAndDirection) {
        this.dictionaryAndDirection = dictionaryAndDirection;
    }
}
