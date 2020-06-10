package com.paragon_software.information_manager.mock;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;

import java.util.LinkedList;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public abstract class MockDictionaryManager extends DictionaryManagerAPI {


    private final static Dictionary.DictionaryId DICTID_1 = new Dictionary.DictionaryId("test_id_1");
    private final static Dictionary.DictionaryId DICTID_2 = new Dictionary.DictionaryId("test_id_2");
    private final static Dictionary.Direction DIRECTION_1 = new Dictionary.Direction(0, 1, null);
    private final static Dictionary.Direction DIRECTION_2 = new Dictionary.Direction(1, 0, null);

    private final List<IDictionaryListObserver> observers = new LinkedList<>();

    public final static DictionaryAndDirection dictionaryAndDirection = new DictionaryAndDirection(DICTID_1, DIRECTION_1);
    public final static DictionaryAndDirection dictionaryAndDirection2 = new DictionaryAndDirection(DICTID_2, DIRECTION_2);
    private DictionaryAndDirection mSelectedDictionary = dictionaryAndDirection;

    public MockDictionaryManager() {
    }

    @Nullable
    @Override
    public DictionaryAndDirection getDictionaryAndDirectionSelectedByUser() {
        return mSelectedDictionary;
    }

    @Override
    public void registerDictionaryListObserver(@NonNull IDictionaryListObserver observer) {
        if (!observers.contains(observer)) {
            observers.add(observer);
        }
    }

    public void onDictionaryListChanged(DictionaryAndDirection dictionaryAndDirection) {
        mSelectedDictionary = dictionaryAndDirection;
        for (IDictionaryListObserver observer : observers) {
            observer.onDictionaryListChanged();
        }
    }
}
