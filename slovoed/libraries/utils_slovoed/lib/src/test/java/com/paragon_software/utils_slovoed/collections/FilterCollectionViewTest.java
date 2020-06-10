package com.paragon_software.utils_slovoed.collections;

import com.paragon_software.utils_slovoed.collections.util.MultiEventListener;
import com.paragon_software.utils_slovoed.collections.util.SingleEventListener;
import com.paragon_software.utils_slovoed.collections.util.Util;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Arrays;

public class FilterCollectionViewTest
{
    private ArrayCollectionView<Integer, Void> CORE;
    private FilterCollectionView<Integer, Void, ArrayCollectionView<Integer, Void>> COLLECTION;
    private SingleEventListener listener = Mockito.spy(SingleEventListener.class);

    @Before
    public void init()
    {
        CORE = new ArrayCollectionView<>(new Integer[] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
        COLLECTION = new FilterCollectionView<>(CORE, new FilterCollectionView.Predicate<Integer>()
        {
            @Override
            public boolean apply(Integer integer)
            {
                return (integer % 2) == 0;
            }
        });
        COLLECTION.registerListener(listener);
    }

    @Test
    public void testCount()
    {
        Assert.assertEquals(5, COLLECTION.getCount());
    }

    @Test
    public void testItems()
    {
        for(int i = 0; i < 5; i++)
            Assert.assertEquals(i * 2, (int) COLLECTION.getItem(i));
    }

    @Test
    public void testSelection()
    {
        CORE.updateSelection(2);
        Assert.assertEquals(1, COLLECTION.getSelection());
        CORE.updateSelection(7);
        Assert.assertEquals(-5, COLLECTION.getSelection());
    }

    @Test
    public void testPosition()
    {
        CORE.updatePosition(8);
        Assert.assertEquals(4, COLLECTION.getPosition());
        CORE.updatePosition(1);
        Assert.assertEquals(-2, COLLECTION.getPosition());
    }

    @Test
    public void testUpdateSelection()
    {
        COLLECTION.updateSelection(3);
        Assert.assertEquals(6, CORE.getSelection());
    }

    @Test
    public void testAdd1()
    {
        CORE.insert(2, Arrays.asList(1000, 10001, 1002, 1003));
        Util.assertCollection(COLLECTION, 0, 1000, 1002, 2, 4, 6, 8);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 1, 2);
    }

    @Test
    public void testAdd2()
    {
        CORE.insert(3, Arrays.asList(1003, 1002, 1001, 1000));
        Util.assertCollection(COLLECTION, 0, 2, 1002, 1000, 4, 6, 8);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 2);
    }

    @Test
    public void testAdd3()
    {
        CORE.add(1000);
        Util.assertCollection(COLLECTION, 0, 2, 4, 6, 8, 1000);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 5, 1);
    }

    @Test
    public void testAdd4()
    {
        CORE.insert(3, 1002);
        Util.assertCollection(COLLECTION, 0, 2, 1002, 4, 6, 8);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 1);
    }

    @Test
    public void testRemove1()
    {
        CORE.remove(2, 2);
        Util.assertCollection(COLLECTION, 0, 4, 6, 8);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 1, 1);
    }

    @Test
    public void testRemove2()
    {
        CORE.remove(4, 3);
        Util.assertCollection(COLLECTION, 0, 2, 8);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 2, 2);
    }

    @Test
    public void testRemove3()
    {
        CORE.remove(3, 7);
        Util.assertCollection(COLLECTION, 0, 2);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 2, 3);
    }

    @Test
    public void testRemove4()
    {
        CORE.remove(6, 2);
        Util.assertCollection(COLLECTION, 0, 2, 4, 8);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 3, 1);
    }

    @Test
    public void testClear()
    {
        CORE.clear();
        Util.assertCollection(COLLECTION);
        Util.assertAndClearListener(listener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, 5);
    }

    @Test
    public void testUpdate1()
    {
        COLLECTION.unregisterListener(listener);
        MultiEventListener multiEventListener = Mockito.spy(MultiEventListener.class);
        COLLECTION.registerListener(multiEventListener);
        CORE.update(2, 7);
        Util.assertCollection(COLLECTION, 0, 4, 6, 8);
        Util.assertAndClearListener(multiEventListener, CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 1, 1);
    }

    @Test
    public void testUpdate2()
    {
        COLLECTION.unregisterListener(listener);
        MultiEventListener multiEventListener = Mockito.spy(MultiEventListener.class);
        COLLECTION.registerListener(multiEventListener);
        CORE.update(0, Arrays.asList(11, 12, 13, 14, 15, 16, 17, 18, 19, 20));
        Util.assertCollection(COLLECTION, 12, 14, 16, 18, 20);
        Util.assertAndClearListener(multiEventListener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, 5,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, 5);
    }

    @Test
    public void testUpdate3()
    {
        COLLECTION.unregisterListener(listener);
        MultiEventListener multiEventListener = Mockito.spy(MultiEventListener.class);
        COLLECTION.registerListener(multiEventListener);
        CORE.update(3, Arrays.asList(78, 76, 74, 72));
        Util.assertCollection(COLLECTION, 0, 2, 78, 76, 74, 72, 8);
        Util.assertAndClearListener(multiEventListener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 2, 2,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 4);
    }

    @Test
    public void testUpdate4()
    {
        COLLECTION.unregisterListener(listener);
        MultiEventListener multiEventListener = Mockito.spy(MultiEventListener.class);
        COLLECTION.registerListener(multiEventListener);
        CORE.update(3, 88);
        Util.assertCollection(COLLECTION, 0, 2, 88, 4, 6, 8);
        Util.assertAndClearListener(multiEventListener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 1);
    }

    @Test
    public void testUpdate5()
    {
        COLLECTION.unregisterListener(listener);
        MultiEventListener multiEventListener = Mockito.spy(MultiEventListener.class);
        COLLECTION.registerListener(multiEventListener);
        CORE.update(4, 34);
        Util.assertCollection(COLLECTION, 0, 2, 34, 6, 8);
        Util.assertAndClearListener(multiEventListener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 2, 1);
    }
}
