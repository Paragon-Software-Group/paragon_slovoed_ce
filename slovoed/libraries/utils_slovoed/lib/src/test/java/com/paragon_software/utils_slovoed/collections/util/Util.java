package com.paragon_software.utils_slovoed.collections.util;

import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.junit.Assert;

public final class Util
{
   public static <ITEM> void assertCollection(CollectionView<ITEM, ?> collection,
                                              ITEM ... content)
   {
        Assert.assertEquals(content.length, collection.getCount());
        for(int i = 0; i < content.length; i++)
            Assert.assertEquals(content[i], collection.getItem(i));
    }

    public static void assertAndClearListener(SingleEventListener listener,
                                              CollectionView.OPERATION_TYPE type,
                                              int position,
                                              int count)
    {
        listener.assertValid(type, position, count);
        listener.clear();
    }

    public static void assertAndClearListener(MultiEventListener listener,
                                              Object ... eventParams)
    {
       listener.assertValid(eventParams);
       listener.clear();
    }

    public static String collectionToString(CollectionView<Integer, ?> collectionView)
    {
        StringBuilder res = new StringBuilder();
        int n = collectionView.getCount();
        for(int i = 0; i < n; i++)
        {
            res.append(collectionView.getItem(i));
            if(i < (n - 1))
                res.append(',');
        }
        return res.toString();
    }
}
