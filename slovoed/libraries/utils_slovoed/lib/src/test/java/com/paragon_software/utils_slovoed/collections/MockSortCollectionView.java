package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.util.Util;

import java.util.Collections;

public abstract class MockSortCollectionView
        extends SortCollectionView<Integer, Void, CollectionView<Integer, Void>>
{
    public MockSortCollectionView(@NonNull CollectionView<Integer, Void> core)
    {
        super(core, Collections.reverseOrder(Collections.<Integer>reverseOrder()));
    }

    @Override
    public String toString()
    {
        return Util.collectionToString(this);
    }
}