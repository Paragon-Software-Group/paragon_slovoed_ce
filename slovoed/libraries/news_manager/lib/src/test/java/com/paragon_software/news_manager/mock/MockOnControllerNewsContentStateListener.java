package com.paragon_software.news_manager.mock;

import com.paragon_software.news_manager.BaseNewsController;
import com.paragon_software.news_manager.OnNewsContentStateListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Created by Ivan Kuzmin on 20.02.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public class MockOnControllerNewsContentStateListener extends MockBaseChangeListener
        implements OnNewsContentStateListener
{
    private BaseNewsController mNewsController;

    public MockOnControllerNewsContentStateListener(int maxNumberOfCalls, BaseNewsController newsController)
    {
        super(maxNumberOfCalls);
        mNewsController = newsController;
    }

    @Override
    public void onNewsContentStateListener(boolean visibility)
    {
        assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
        assertEquals(visibility, mNewsController.isContentVisibility());
    }
}
