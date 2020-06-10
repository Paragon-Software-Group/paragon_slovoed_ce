package com.paragon_software.navigation_manager.mock;

import com.paragon_software.navigation_manager.NavigationControllerApi;

import static org.junit.Assert.assertTrue;

public abstract class MockForController_NewsListChangedListener extends BaseChangeListener
        implements NavigationControllerApi.OnNewsListChangedListener {

    public MockForController_NewsListChangedListener(int maxNumberOfCalls) {
        super(maxNumberOfCalls);
    }

    @Override
    public void onNewsListChanged() {
        assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    }
}