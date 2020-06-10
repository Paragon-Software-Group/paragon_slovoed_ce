package com.paragon_software.navigation_manager.mock;


import com.paragon_software.news_manager.NewsManagerAPI;
import com.paragon_software.news_manager.OnNewsListChangesListener;

import java.util.ArrayList;
import java.util.List;

import androidx.annotation.NonNull;
import io.reactivex.Observable;
import io.reactivex.subjects.BehaviorSubject;

public abstract class MockNewsManager extends NewsManagerAPI {

    private int mUnreadCount;
    private final List<OnNewsListChangesListener> mOnNewsListChangesListeners = new ArrayList<>();
    private final BehaviorSubject<Integer> mUnreadNewsCountSubject = BehaviorSubject.createDefault(0);

    public void setUnreadCount(int unreadCount) {
        mUnreadCount = unreadCount;
        updateNews();
    }

    @Override
    public void registerNotifier(@NonNull Notifier notifier) {
        if (notifier instanceof OnNewsListChangesListener && !mOnNewsListChangesListeners.contains(notifier)) {
            mOnNewsListChangesListeners.add((OnNewsListChangesListener) notifier);
        }
    }

    @Override
    public Observable<Integer> getUnreadNewsCount() {
        return mUnreadNewsCountSubject;
    }

    private void updateNews() {
        for (OnNewsListChangesListener notifier : mOnNewsListChangesListeners) {
            notifier.onNewsListChanged();
        }
        mUnreadNewsCountSubject.onNext(mUnreadCount);
    }
}