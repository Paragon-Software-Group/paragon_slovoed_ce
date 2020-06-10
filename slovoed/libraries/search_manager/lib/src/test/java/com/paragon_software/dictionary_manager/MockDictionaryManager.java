package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.ArrayList;
import java.util.List;

import io.reactivex.Observable;

public abstract class MockDictionaryManager extends DictionaryManagerAPI
{
    public int mGetDictionariesCounter = 0;

    public List<Dictionary> dictionaries = new ArrayList<>();
    public List<Dictionary.Direction> directions = new ArrayList<>();

    public static Dictionary.DictionaryId dictId_1 = new Dictionary.DictionaryId("test_id_1");
    public static Dictionary.DictionaryId dictId_2 = new Dictionary.DictionaryId("test_id_2");

    public static Dictionary.Direction direction_1 = new Dictionary.Direction(0,1,null);
    public static Dictionary.Direction direction_2 = new Dictionary.Direction(1,0,null);

    public DictionaryAndDirection mDictionaryAndDirection = new DictionaryAndDirection(dictId_1, direction_1);

    public MockDictionaryManager()
    {
        directions.add(direction_1);
        directions.add(direction_2);

        dictionaries.add(new Dictionary.Builder(
                dictId_1,
                LocalizedString.from("test_title_1"),
                LocalizedString.from("test_desctiption_1"),
                                     null).
                setDirections(directions).
                build());

        dictionaries.add(new Dictionary.Builder(
                dictId_2,
                LocalizedString.from("test_title_2"),
                LocalizedString.from("test_desctiption_2"),
                null).
                setDirections(directions).
                build());
    }

    @NonNull
    @Override
    public List< Dictionary > getDictionaries()
    {
        ++mGetDictionariesCounter;
        return dictionaries;
    }

    @Override
    public Dictionary getDictionaryById(@NonNull Dictionary.DictionaryId dictionaryId)
    {
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
    public Observable<Boolean> getDictionaryAndDirectionChangeObservable() {
        return Observable.empty();
    }

    @Nullable
    @Override
    public DictionaryAndDirection getDictionaryAndDirectionSelectedByUser() {
        return mDictionaryAndDirection;
    }
}
