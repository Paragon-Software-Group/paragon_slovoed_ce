package com.paragon_software.utils_slovoed.collections;

import com.paragon_software.utils_slovoed.collections.util.TransformIntToString;
import com.paragon_software.utils_slovoed.collections.util.SingleEventListener;
import com.paragon_software.utils_slovoed.collections.util.Util;

import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

public class TransformCollectionViewTest
{
    private static final Integer[] ARRAY = {1, 2, 3, 4, 5};

    ArrayCollectionView<Integer, Void> source;
    TransformCollectionView<String,
                            Integer,
            TransformIntToString,
                            Void,
                            ArrayCollectionView<Integer, Void>> collection;
    TransformIntToString listener1 = Mockito.spy(TransformIntToString.class);
    SingleEventListener listener2 = Mockito.spy(SingleEventListener.class);

    @Before
    public void init() 
    {
        source = new ArrayCollectionView<>(ARRAY);
        listener1.clear();
        listener2.clear();
        collection = new TransformCollectionView<>(source, listener1);
        collection.registerListener(listener2);
    }

    @Test
    public void testInitialCollection() 
    {
        Util.assertCollection(collection, "1", "2", "3", "4", "5");
    }

    @Test
    public void testChangedCollection() 
    {
        source.insert(2, new Integer[]{99, 77, 55});
        Util.assertCollection(collection, "1", "2", "99", "77", "55", "3", "4", "5");
        Util.assertAndClearListener(listener1, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 3);
        Util.assertAndClearListener(listener2, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 3);

        source.update(3, 1000);
        Util.assertCollection(collection, "1", "2", "99", "1000", "55", "3", "4", "5");
        Util.assertAndClearListener(listener1, CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 3, 1);
        Util.assertAndClearListener(listener2, CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 3, 1);

        source.remove(5);
        Util.assertCollection(collection, "1", "2", "99", "1000", "55", "4", "5");
        Util.assertAndClearListener(listener1, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 5, 1);
        Util.assertAndClearListener(listener2, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 5, 1);
    }
}
