package com.paragon_software.navigation_manager.mock;

public abstract class BaseChangeListener {
    final int mMaxNumberOfCalls;

    int mNumberOfCalls;

    public int getNumberOfCalls() {
        return mNumberOfCalls;
    }

    BaseChangeListener(int maxNumberOfCalls) {
        mMaxNumberOfCalls = maxNumberOfCalls;
    }
}