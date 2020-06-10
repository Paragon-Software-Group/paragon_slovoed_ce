package com.paragon_software.mock;

import com.paragon_software.utils_slovoed.collections.CollectionView;

public abstract class MockCollectionView<ITEM_TYPE,LABEL_TYPE> implements CollectionView<ITEM_TYPE,LABEL_TYPE>
{
    @Override
    public int getCount() {
        return 0;
    }

    @Override
    public ITEM_TYPE getItem(int position) {
        return null;
    }

    @Override
    public LABEL_TYPE getMetadata() {
        return null;
    }

    @Override
    public int getSelection()
    {
        return -1;
    }

    @Override
    public boolean isInProgress() {
        return false;
    }

    @Override
    public int getPosition()
    {
        return 0;
    }
}
