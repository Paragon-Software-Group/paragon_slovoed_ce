package com.paragon_software.news_manager.mock;

import com.paragon_software.news_manager.OnControllerNewsListChangedListener;

import static org.junit.Assert.assertTrue;

/**
 * Created by Ivan Kuzmin on 20.02.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public class MockOnControllerNewsListChangedListener extends MockBaseChangeListener
        implements OnControllerNewsListChangedListener
{
    public MockOnControllerNewsListChangedListener(int maxNumberOfCalls)
    {
        super(maxNumberOfCalls);
    }

    @Override
    public void onControllerNewsListChanged()
    {
        assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    }
}
