package com.paragon_software.utils_slovoed.collections;

import com.paragon_software.utils_slovoed.collections.util.SingleEventListener;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;

public class HashSetMultiselectCollectionViewTest 
{
    private static final Integer ARRAY[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    private ArrayCollectionView<Integer, Void> source;
    private MultiselectCollectionView<Integer, HashSetMultiselectCollectionView.SelectionTransform<Integer>, Void, ArrayCollectionView<Integer, Void>> collection;

    @Before
    public void init() 
    {
        source = new ArrayCollectionView<>(ARRAY);
        collection = new HashSetMultiselectCollectionView<>(source, false);
    }

    @Test
    public void testInitial() 
    {
        assertCollection(0, false, 1, false, 2, false, 3, false, 4, false,
                         5, false, 6, false, 7, false, 8, false, 9, false);
    }

    @Test
    public void testInitialSelected() 
    {
        collection = new HashSetMultiselectCollectionView<>(source, true);
        assertCollection(0, true, 1, true, 2, true, 3, true, 4, true,
                         5, true, 6, true, 7, true, 8, true, 9, true);
    }

    @Test
    public void testSelect() 
    {
        collection.select(3);
        assertCollection(0, false, 1, false, 2, false, 3, true, 4, false,
                         5, false, 6, false, 7, false, 8, false, 9, false);
    }

    @Test
    public void testUnselect() 
    {
        collection = new HashSetMultiselectCollectionView<>(source, true);
        collection.unselect(7);
        collection.unselect(9);
        assertCollection(0, true, 1, true, 2, true, 3, true, 4, true,
                         5, true, 6, true, 7, false, 8, true, 9, false);
    }

    @Test
    public void testFlip() 
    {
        collection.flip(0);
        collection.flip(3);
        collection.flip(4);
        collection.flip(5);
        collection.flip(9);
        collection.flip(5);
        assertCollection(0, true, 1, false, 2, false, 3, true, 4, true,
                         5, false, 6, false, 7, false, 8, false, 9, true);
    }

    @Test
    public void testAdd1() 
    {
        source.insert(0, Arrays.asList(1000, 2000, 3000));
        assertCollection(1000, false, 2000, false, 3000, false,
                0, false, 1, false, 2, false, 3, false, 4, false,
                         5, false, 6, false, 7, false, 8, false, 9, false);
    }

    @Test
    public void testAdd2() 
    {
        collection = new HashSetMultiselectCollectionView<>(source, true);
        source.insert(10, Arrays.asList(1000, 2000, 3000));
        assertCollection(0, true, 1, true, 2, true, 3, true, 4, true,
                         5, true, 6, true, 7, true, 8, true, 9, true,
                         1000, true, 2000, true, 3000, true);
    }

    @Test
    public void testAdd3() 
    {
        source.insert(5, Arrays.asList(1000, 2000, 3000));
        assertCollection(0, false, 1, false, 2, false, 3, false, 4, false,
                         1000, false, 2000, false, 3000, false,
                         5, false, 6, false, 7, false, 8, false, 9, false);
    }

    @Test
    public void testRemove1() 
    {
        Random random = new Random();
        for(int i = 0; i < 10; i++)
        {
            source = new ArrayCollectionView<>(ARRAY);
            collection = new HashSetMultiselectCollectionView<>(source, false);
            int position = random.nextInt(ARRAY.length);
            source.remove(position);
            ArrayList<Integer> list = new ArrayList<>(Arrays.asList(ARRAY));
            list.remove(position);
            Object[] assertArray = new Object[(ARRAY.length - 1) * 2];
            for(int j = 0; j < (ARRAY.length - 1); j++)
            {
                assertArray[j * 2] = list.get(j);
                assertArray[j * 2 + 1] = false;
            }
            assertCollection(assertArray);
        }
    }

    @Test
    public void testRemove2() 
    {
        Random random = new Random();
        for(int i = 0; i < 10; i++)
        {
            source = new ArrayCollectionView<>(ARRAY);
            collection = new HashSetMultiselectCollectionView<>(source, true);
            int position = random.nextInt(ARRAY.length);
            source.remove(position);
            ArrayList<Integer> list = new ArrayList<>(Arrays.asList(ARRAY));
            list.remove(position);
            Object[] assertArray = new Object[(ARRAY.length - 1) * 2];
            for(int j = 0; j < (ARRAY.length - 1); j++)
            {
                assertArray[j * 2] = list.get(j);
                assertArray[j * 2 + 1] = true;
            }
            assertCollection(assertArray);
        }
    }

    @Test
    public void testRemove3() 
    {
        Random random = new Random();
        for(int i = 0; i < 10; i++)
        {
            source = new ArrayCollectionView<>(ARRAY);
            collection = new HashSetMultiselectCollectionView<>(source, false);
            ArrayList<Boolean> initialState = new ArrayList<>(ARRAY.length);
            for(int j = 0; j < ARRAY.length; j++)
            {
                initialState.add(random.nextBoolean());
                collection.select(j, initialState.get(j));
            }
            int position = random.nextInt(ARRAY.length);
            source.remove(position);
            initialState.remove(position);
            ArrayList<Integer> list = new ArrayList<>(Arrays.asList(ARRAY));
            list.remove(position);
            Object[] assertArray = new Object[(ARRAY.length - 1) * 2];
            for(int j = 0; j < (ARRAY.length - 1); j++)
            {
                assertArray[j * 2] = list.get(j);
                assertArray[j * 2 + 1] = initialState.get(j);
            }
            assertCollection(assertArray);
        }
    }

    @Test
    public void testInsertEvent() 
    {
        SingleEventListener listener = Mockito.spy(SingleEventListener.class);
        collection.registerListener(listener);
        source.insert(5, 1000);
        listener.assertValid(CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, 5, 1);
    }

    @Test
    public void testRemoveEvent() 
    {
        SingleEventListener listener = Mockito.spy(SingleEventListener.class);
        collection.registerListener(listener);
        source.remove(0);
        listener.assertValid(CollectionView.OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, 1);
    }

    @Test
    public void testUpdateEvent() 
    {
        SingleEventListener listener = Mockito.spy(SingleEventListener.class);
        collection.registerListener(listener);
        source.update(9, 1000);
        listener.assertValid(CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 9, 1);
    }

    @Test
    public void testSelectEvent() 
    {
        SingleEventListener listener = Mockito.spy(SingleEventListener.class);
        collection.registerListener(listener);
        collection.select(7);
        listener.assertValid(CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 7, 1);
    }

    @Test
    public void testCardinality() 
    {
        Assert.assertEquals(0, collection.getSelectionCount());
        collection.select(0);
        collection.select(7);
        Assert.assertEquals(2, collection.getSelectionCount());
        collection.flip(7);
        Assert.assertEquals(1, collection.getSelectionCount());
        collection.select(0);
        Assert.assertEquals(1, collection.getSelectionCount());
        for(int i = 0; i < ARRAY.length; i++)
            collection.select(i, true);
        Assert.assertEquals(ARRAY.length, collection.getSelectionCount());
    }

    @Test
    public void testSelectAll() 
    {
        SingleEventListener listener = Mockito.spy(SingleEventListener.class);
        collection.registerListener(listener);
        collection.selectAll(true);
        listener.assertValid(CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 0, ARRAY.length);
    }

    @Test
    public void testUnselectAll() 
    {
        collection = new HashSetMultiselectCollectionView<>(source, true);
        collection.unselect(7);
        SingleEventListener listener = Mockito.spy(SingleEventListener.class);
        collection.registerListener(listener);
        collection.selectAll(false);
        listener.assertValid(CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, 0, ARRAY.length);
    }

    private void assertCollection(Object ... values)
    {
        int n = values.length;
        Assert.assertEquals(0, n % 2);
        Assert.assertEquals(values.length, collection.getCount() * 2);
        for(int i = 0; i < n ; i += 2)
        {
            Object o1 = values[i];
            Object o2 = values[i + 1];
            Assert.assertTrue(o1 instanceof Integer);
            Assert.assertTrue(o2 instanceof Boolean);
            MultiselectCollectionView.Selectable<Integer> selectable =
                    collection.getItem(i / 2);
            Assert.assertEquals(o1, selectable.item);
            Assert.assertEquals(o2, selectable.isSelected);
        }
    }
}
