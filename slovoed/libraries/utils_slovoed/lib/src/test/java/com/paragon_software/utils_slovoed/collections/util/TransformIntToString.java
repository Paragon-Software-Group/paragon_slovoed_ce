package com.paragon_software.utils_slovoed.collections.util;

import com.paragon_software.utils_slovoed.collections.TransformCollectionView;

public abstract class TransformIntToString extends SingleEventListener
            implements TransformCollectionView.TransformFunction<String, Integer>
{
    @Override
    public String transform(int position, Integer argument)
    {
        return argument.toString();
    }
}