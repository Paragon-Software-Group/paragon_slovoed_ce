package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.util.MultiEventListener;
import com.paragon_software.utils_slovoed.collections.util.Util;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.withSettings;

public class SortCollectionViewTest
{
    private MockArrayCollectionView CORE;
    private MockSortCollectionView COLLECTION;
    private MultiEventListener listener;

    @Before
    public void init() 
    {
        CORE = Mockito.mock(MockArrayCollectionView.class, withSettings()
                .useConstructor((Object) new Integer[] {70, 10, 30, 20, 0, 90, 50, 60, 80, 40})
                .defaultAnswer(CALLS_REAL_METHODS));
        COLLECTION = Mockito.mock(MockSortCollectionView.class, withSettings()
                .useConstructor(CORE).defaultAnswer(CALLS_REAL_METHODS));
        listener = Mockito.spy(MultiEventListener.class);
        COLLECTION.registerListener(listener);
    }

    @Test
    public void testSort()
    {
        assertSorted(CORE, COLLECTION);
        Random random = new Random();
        for(int i = 0; i < 100; i++) 
        {
            int n = random.nextInt(100) + 1;
            List<Integer> list = new ArrayList<>(n);
            for(int j = 0; j < n; j++)
                list.add(random.nextInt());
            MockArrayCollectionView core = Mockito.mock(MockArrayCollectionView.class
                    , withSettings().useConstructor(list).defaultAnswer(CALLS_REAL_METHODS));
            MockSortCollectionView sorted = Mockito.mock(MockSortCollectionView.class
                    , withSettings().useConstructor(core).defaultAnswer(CALLS_REAL_METHODS));
            assertSorted(core, sorted);
        }
    }

    @Test
    public void testSelection() 
    {
        CORE.updateSelection(2);
        Assert.assertEquals(3, COLLECTION.getSelection());
        CORE.updateSelection(7);
        Assert.assertEquals(6, COLLECTION.getSelection());
    }

    @Test
    public void testPosition() 
    {
        CORE.updatePosition(8);
        Assert.assertEquals(8, COLLECTION.getPosition());
        CORE.updatePosition(1);
        Assert.assertEquals(1, COLLECTION.getPosition());
    }

    @Test
    public void testUpdateSelection() 
    {
        COLLECTION.updateSelection(3);
        Assert.assertEquals(2, CORE.getSelection());
    }

    @Test
    public void testInsert1() 
    {
        CORE.insert(4, new Integer[] {81, 21, 51});
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 9, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 3, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 7, 1);
    }

    @Test
    public void testInsert2() 
    {
        CORE.add(100);
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 10, 1);
    }

    @Test
    public void testRemove1() 
    {
        CORE.remove(1, 4);
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, 4);
    }

    @Test
    public void testRemove2() 
    {
        CORE.remove(5, 5);
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 4, 6,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 4, 1);
    }

    @Test
    public void testUpdate1() 
    {
        CORE.update(1, new Integer[] {11, 31, 21});
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 1, 3);
    }

    @Test
    public void testUpdate2() 
    {
        CORE.update(1, new Integer[] {11, 41, 21});
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 1, 3,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 1, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 3, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 1);
    }

    @Test
    public void testUpdate3() 
    {
        CORE.update(5, 91);
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 9, 1);
    }

    @Test
    public void testUpdate4() 
    {
        CORE.update(5, 11);
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 9, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 2, 1);
    }

    @Test
    public void testUpdate5() 
    {
        CORE = Mockito.mock(MockArrayCollectionView.class, withSettings()
                .useConstructor((Object) new Integer[] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
                .defaultAnswer(CALLS_REAL_METHODS));
        COLLECTION = Mockito.mock(MockSortCollectionView.class, withSettings()
                .useConstructor(CORE).defaultAnswer(CALLS_REAL_METHODS));
        listener = Mockito.spy(MultiEventListener.class);
        COLLECTION.registerListener(listener);
        for(int i = 1; i < CORE.getCount(); i += 2) 
        {
            CORE.update(i, 100 + CORE.getItem(i));
            assertSorted(CORE, COLLECTION);
            Util.assertAndClearListener(listener,
                                        CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, i / 2 + 1, 1,
                                        CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 9, 1);
        }
    }

    @Test
    public void testUpdate6() 
    {
        CORE.update(0, new Integer[] {11, 81, 53, 17, 91});
        assertSorted(CORE, COLLECTION);
        Util.assertAndClearListener(listener,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, 8,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, 3,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 5, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 3, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 1, 1,
                                    CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 9, 1);
    }

    private void assertSorted(@NonNull CollectionView<Integer, Void> core,
                              @NonNull CollectionView<Integer, Void> sorted) 
    {
        int n = core.getCount();
        Assert.assertEquals(n, sorted.getCount());
        Map<Integer, Integer> map1 = new HashMap<>(n), map2 = new HashMap<>(n);
        for(int i = 0; i < n; i++) 
        {
            registerInt(map1, core.getItem(i));
            registerInt(map2, sorted.getItem(i));
        }
        Assert.assertEquals(map1, map2);
        for(int i = 1; i < n; i++)
            Assert.assertTrue(sorted.getItem(i) > sorted.getItem(i - 1));
    }

    private void registerInt(@NonNull Map<Integer, Integer> map, int n) 
    {
        int count = 0;
        if(map.containsKey(n))
            count = map.get(n);
        count++;
        map.put(n, count);
    }
}
