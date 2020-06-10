package com.paragon_software.navigation_manager.mock;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.navigation_manager.NavigationControllerApi;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_OnDictionariesChangedListener extends BaseChangeListener
        implements NavigationControllerApi.OnDictionariesChangedListener {
    private final NavigationControllerApi mNavigationController;
    private final DictionaryManagerAPI mDictionaryManager;

    public MockForController_OnDictionariesChangedListener(int maxNumberOfCalls, NavigationControllerApi navigationController, DictionaryManagerAPI dictionaryManager) {
        super(maxNumberOfCalls);
        mNavigationController = navigationController;
        mDictionaryManager = dictionaryManager;
    }

    @Override
    public void onDictionariesChanged() {
        assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);

        Dictionary dict = mDictionaryManager.getDictionaryById(mDictionaryManager.getDictionaryAndDirectionSelectedByUser().getDictionaryId());
        assertEquals(dict.getTitle().get(), mNavigationController.getDrawerTitle());
        assertEquals(dict.getIcon().getBitmap(), mNavigationController.getDrawerIcon());
    }
}