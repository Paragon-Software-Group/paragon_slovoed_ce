package com.paragon_software.utils_slovoed.collections;

import com.paragon_software.utils_slovoed.collections.util.MultiEventListener;
import com.paragon_software.utils_slovoed.collections.util.Util;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class ReorderCollectionViewTest 
{
    private ArrayCollectionView<Integer, Void> CORE;
    private ReorderCollectionView<Integer, Void, ArrayCollectionView<Integer, Void>> COLLECTION;
    private MultiEventListener listener;

    @Before
    public void init() 
    {
        CORE = new ArrayCollectionView<Integer, Void>(new Integer[] {0, 10, 20, 30, 40, 50, 60, 70, 80, 90})
        {
            @Override
            public String toString()
            {
                return Util.collectionToString(this);
            }
        };
        COLLECTION = new ReorderCollectionView<Integer, Void, ArrayCollectionView<Integer, Void>>
                (CORE, Mockito.spy(MockReverseOrder.class)) 
        {
            @Override
            public String toString() 
            {
                return Util.collectionToString(this);
            }
        };
        listener = Mockito.spy(MultiEventListener.class);
        COLLECTION.registerListener(listener);
    }

    @Test
    public void testCount() 
    {
        Assert.assertEquals(10, COLLECTION.getCount());
    }

    @Test
    public void testItems() 
    {
        for(int i = 0; i < 10; i++)
            Assert.assertEquals((10 - i - 1) * 10, (int) COLLECTION.getItem(i));
        assertReverseOrder();
    }

    @Test
    public void testSelection() 
    {
        CORE.updateSelection(2);
        Assert.assertEquals(7, COLLECTION.getSelection());
        CORE.updateSelection(7);
        Assert.assertEquals(2, COLLECTION.getSelection());
    }

    @Test
    public void testPosition() 
    {
        CORE.updatePosition(8);
        Assert.assertEquals(1, COLLECTION.getPosition());
        CORE.updatePosition(1);
        Assert.assertEquals(8, COLLECTION.getPosition());
    }

    @Test
    public void testUpdateSelection() 
    {
        COLLECTION.updateSelection(3);
        Assert.assertEquals(6, CORE.getSelection());
    }

    @Test
    public void testRandomInserts() 
    {
        Random random = new Random();
        for(int i = 0; i < 100; i++) 
        {
            int index = random.nextInt(CORE.getCount() + 1);
            int len = random.nextInt(5) + 1;
            List<Integer> listToAdd = new ArrayList<>(len);
            Object[] updateEvents = new Object[len * 3];
            for(int j = 0; j < len; j++) 
            {
                listToAdd.add(random.nextInt());
                updateEvents[j * 3] = CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED;
                updateEvents[j * 3 + 1] = CORE.getCount() - index;
                updateEvents[j * 3 + 2] = 1;
            }
            CORE.insert(index, listToAdd);
            assertReverseOrder();
            Util.assertAndClearListener(listener, updateEvents);
        }
    }

    @Test
    public void testRemove1() 
    {
        CORE.remove(2);
        assertReverseOrder();
        Util.assertAndClearListener(listener
                , CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 7, 1);
    }

    @Test
    public void testRemove2() 
    {
        CORE.remove(2, 7);
        assertReverseOrder();
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 1, 7);
    }

    @Test
    public void testUpdate() 
    {
        CORE.update(5, new Integer[] {51, 61, 71});
        assertReverseOrder();
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 2, 3);
    }

    private void assertReverseOrder() 
    {
        Assert.assertEquals(CORE.getCount(), COLLECTION.getCount());
        for(int i = 0; i < COLLECTION.getCount(); i++)
            Assert.assertEquals(CORE.getItem(CORE.getCount() - i - 1), COLLECTION.getItem(i));
    }
}
