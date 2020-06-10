package com.paragon_software.utils_slovoed.collections.util;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.junit.Assert;

import java.util.ArrayList;
import java.util.List;

public abstract class MultiEventListener implements CollectionView.OnItemRangeChanged
{
    @NonNull
    private final List<ChangedEvent> mEvents = new ArrayList<>();

    @Override
    public void onItemRangeChanged(CollectionView.OPERATION_TYPE type, int startPosition, int itemCount)
    {
        mEvents.add(new ChangedEvent(type, startPosition, itemCount));
    }

    public void clear()
    {
        mEvents.clear();
    }

    public void assertValid(Object ... eventParams)
    {
        Assert.assertEquals(eventParams.length / 3, mEvents.size());
        for(int i = 0; i < mEvents.size(); i++)
            Assert.assertEquals(new ChangedEvent((CollectionView.OPERATION_TYPE) eventParams[i * 3],
                                                 (Integer) eventParams[i * 3 + 1],
                                                 (Integer) eventParams[i * 3 + 2]),
                                mEvents.get(i));
    }
}
