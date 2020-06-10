package com.paragon_software.navigation_manager.mock;

import com.paragon_software.word_of_day.WotDItem;
import com.paragon_software.word_of_day.WotDManagerAPI;

import java.util.ArrayList;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.subjects.BehaviorSubject;

public abstract class MockWotdManager extends WotDManagerAPI {

    private BehaviorSubject<List<WotDItem>> mWotdItemsSubject = BehaviorSubject.createDefault(new ArrayList<>());

    public void setWotDList(List<WotDItem> wotDItems) {
        mWotdItemsSubject.onNext(wotDItems);
    }

    @Override
    public Observable<Integer> getNewWotDCount() {
        return mWotdItemsSubject.map(this::getNewWotDItemsCount);
    }

    private int getNewWotDItemsCount(List<WotDItem> wotDItems) {
        int newItemsCount = 0;
        for (WotDItem wotDItem : wotDItems) {
            if (!wotDItem.getIsViewed()) {
                newItemsCount++;
            }
        }
        return newItemsCount;
    }
}