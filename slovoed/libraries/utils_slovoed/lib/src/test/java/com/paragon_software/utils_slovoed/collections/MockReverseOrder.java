package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import java.util.Collections;
import java.util.List;

public abstract class MockReverseOrder implements ReorderCollectionView.Order<Integer>
{
    @Override
    public void getInitialIndices(@NonNull CollectionView<Integer, ?> core
            , @NonNull List<Integer> indices)
    {
        int n = core.getCount();
        indices.clear();
        for(int i = 0; i < n; i++)
            indices.add(n - i - 1);
    }

    @Override
    public int getWrapperIndex(@NonNull CollectionView<Integer, ?> core
            , @NonNull List<Integer> indices, int coreIndex) 
    {
        return core.getCount() - coreIndex - 1;
    }

    @Override
    public int getWrapperInsertPoint(@NonNull CollectionView<Integer, ?> core
            , @NonNull List<Integer> indices, int coreIndex) 
    {
        int res = Collections.binarySearch(indices, coreIndex, Collections.<Integer>reverseOrder());
        if(res < 0)
            res = -res - 1;
        return res;
    }
}