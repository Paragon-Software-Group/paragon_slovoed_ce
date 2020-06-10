package com.paragon_software.utils_slovoed.collections.util;

import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.junit.Assert;

public abstract class SingleEventListener implements CollectionView.OnItemRangeChanged
{
    private ChangedEvent event = null;
    private int fired = 0;

    @Override
    public void onItemRangeChanged(CollectionView.OPERATION_TYPE type, int startPosition, int itemCount)
    {
        if(event == null)
            event = new ChangedEvent(type, startPosition, itemCount);
        fired++;
    }

    public void clear()
    {
        event = null;
        fired = 0;
    }

    public void assertValid(CollectionView.OPERATION_TYPE type,  int position, int count)
    {
        Assert.assertNotNull(event);
        Assert.assertEquals(1, fired);
        Assert.assertEquals(new ChangedEvent(type, position, count), event);
    }
}
