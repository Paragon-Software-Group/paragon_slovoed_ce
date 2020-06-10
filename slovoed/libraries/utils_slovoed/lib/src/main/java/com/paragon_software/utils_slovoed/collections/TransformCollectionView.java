package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

public class
TransformCollectionView<ITEM,
                        CORE_ITEM,
                        TRANSFORM_FUNCTION
                                extends TransformCollectionView.TransformFunction<ITEM,
                                                                                  CORE_ITEM>,
                        METADATA,
                        CORE_COLLECTION extends CollectionView<CORE_ITEM,
                                                               METADATA>>
        extends AbstractWrapperCollectionView<ITEM, CORE_ITEM, METADATA, CORE_COLLECTION> {
    public interface TransformFunction<ITEM, CORE_ITEM> extends CollectionView.OnItemRangeChanged {
        ITEM transform(int position, CORE_ITEM argument);
    }

    @NonNull
    private final TRANSFORM_FUNCTION mTransformFunction;

    public TransformCollectionView(@NonNull CORE_COLLECTION core,
                                   @NonNull TRANSFORM_FUNCTION transformFunction) {
        super(core);
        mTransformFunction = transformFunction;
    }

    @Override
    public ITEM getItem(int position) {
        return mTransformFunction.transform(position, getCore().getItem(position));
    }

    @Override
    public void onItemRangeChanged(OPERATION_TYPE type, int startPosition, int itemCount) {
        mTransformFunction.onItemRangeChanged(type, startPosition, itemCount);
        super.onItemRangeChanged(type, startPosition, itemCount);
    }

    @NonNull
    protected final TRANSFORM_FUNCTION getTransformFunction() {
        return mTransformFunction;
    }
}
