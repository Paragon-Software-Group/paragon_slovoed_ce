package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.collections.util.Util;

import java.util.Collection;

public abstract class MockArrayCollectionView extends ArrayCollectionView<Integer, Void>
{
    public MockArrayCollectionView(@Nullable Collection<Integer> collection)
    {
        super(collection);
    }

    public MockArrayCollectionView(@Nullable Integer[] integers) 
    {
        super(integers);
    }

    @Override
    public String toString() 
    {
        return Util.collectionToString(this);
    }
}