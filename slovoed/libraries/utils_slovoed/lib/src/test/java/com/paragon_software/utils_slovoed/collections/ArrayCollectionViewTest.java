package com.paragon_software.utils_slovoed.collections;

import com.paragon_software.utils_slovoed.collections.util.SingleEventListener;
import com.paragon_software.utils_slovoed.collections.util.Util;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Arrays;

public class ArrayCollectionViewTest
{
    private static final Integer[] ARRAY = {1, 2, 3, 4, 5};

    private ArrayCollectionView<Integer, Void> collection;
    private SingleEventListener listener = Mockito.spy(SingleEventListener.class);

    @Before
    public void init()
    {
        listener.clear();
        collection = new ArrayCollectionView<>(ARRAY);
        collection.registerListener(listener);
    }

    @Test
    public void testGetCount()
    {
        Assert.assertEquals(ARRAY.length, collection.getCount());
    }

    @Test
    public void testGetItem()
    {
        for(int i = 0; i < ARRAY.length; i++)
            Assert.assertEquals(ARRAY[i], collection.getItem(i));
    }

    @Test
    public void testAdd1()
    {
        collection.add(6);
        Util.assertCollection(collection, 1, 2, 3, 4, 5, 6);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 5, 1);
    }

    @Test
    public void testAdd2()
    {
        collection.add(Arrays.asList(6, 7, 8));
        Util.assertCollection(collection, 1, 2, 3, 4, 5, 6, 7, 8);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 5, 3);
    }

    @Test
    public void testAdd3()
    {
        collection.add(new Integer[] {6, 7, 8});
        Util.assertCollection(collection, 1, 2, 3, 4, 5, 6, 7, 8);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 5, 3);
    }

    @Test
    public void testInsert1()
    {
        collection.insert(2, 99);
        Util.assertCollection(collection, 1, 2, 99, 3, 4, 5);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 1);
    }

    @Test
    public void testInsert2()
    {
        collection.insert(2, Arrays.asList(99, 77, 55));
        Util.assertCollection(collection, 1, 2, 99, 77, 55, 3, 4, 5);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 3);
    }

    @Test
    public void testInsert3()
    {
        collection.insert(2, new Integer[] {99, 77, 55});
        Util.assertCollection(collection, 1, 2, 99, 77, 55, 3, 4, 5);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 3);
    }

    @Test
    public void testUpdate1()
    {
        collection.update(2, 99);
        Util.assertCollection(collection, 1, 2, 99, 4, 5);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 2, 1);
    }

    @Test
    public void testUpdate2()
    {
        collection.update(2, Arrays.asList(99, 77, 55));
        Util.assertCollection(collection, 1, 2, 99, 77, 55);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 2, 3);
    }

    @Test
    public void testUpdate3()
    {
        collection.update(2, new Integer[] {99, 77, 55});
        Util.assertCollection(collection, 1, 2, 99, 77, 55);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 2, 3);
    }

    @Test
    public void testClear()
    {
        collection.clear();
        Util.assertCollection(collection);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, 5);
    }

    @Test
    public void testRemove1()
    {
        collection.remove(2);
        Util.assertCollection(collection, 1, 2, 4, 5);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 2, 1);
    }

    @Test
    public void testRemove2()
    {
        collection.remove(2, 2);
        Util.assertCollection(collection, 1, 2, 5);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 2, 2);
    }
}
