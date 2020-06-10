package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

public class ArrayCollectionView<ITEM, METADATA> extends BasicCollectionView<ITEM, METADATA>
    implements MutableCollectionView< ITEM, METADATA >
{
    private static final int DEFAULT_CAPACITY = 20;

    @NonNull
    private final ArrayList<ITEM> mData;

    public ArrayCollectionView() {
        this(DEFAULT_CAPACITY);
    }

    public ArrayCollectionView(int capacity) {
        this(new ArrayList<ITEM>(capacity), false);
    }

    public ArrayCollectionView(@Nullable Collection<ITEM> collection) {
        this(collectionToArrayList(collection), false);
    }

    public ArrayCollectionView(@Nullable ITEM[] items){
        this((items != null) ? Arrays.asList(items) : null);
    }

    public ArrayCollectionView(@NonNull ArrayList<ITEM> list,  boolean copy) {
        if(copy)
            list = new ArrayList<>(list);
        mData = list;
    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public ITEM getItem(int position) {
        return mData.get(position);
    }

    public void add(ITEM item) {
        mData.add(item);
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED,
                               mData.size() - 1,
                               1);
    }

    public void add(@Nullable Collection<ITEM> items) {
        if(items != null) {
            mData.addAll(items);
            int n = items.size();
            callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED,
                                   mData.size() - n,
                                   n);
        }
    }

    public void add(@Nullable ITEM[] items) {
        if(items != null)
            add(Arrays.asList(items));
    }

    public void insert(int position, ITEM item) {
        mData.add(position, item);
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, position, 1);
    }

    public void insert(int position, @Nullable Collection<ITEM> items) {
        if(items != null) {
            mData.addAll(position, items);
            callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, position, items.size());
        }
    }

    public void insert(int position, @Nullable ITEM[] items) {
        if(items != null)
            insert(position, Arrays.asList(items));
    }

    @Override
    public void update( int position, ITEM item ) {
        mData.set(position, item);
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, position, 1);
    }

    public void update(int position, @Nullable Collection<ITEM> items) {
        if(items != null) {
            for(ITEM item : items)
                mData.set(position++, item);
            callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED,
                                   position - items.size(),
                                   items.size());
        }
    }

    public void update(int position, @Nullable ITEM[] items) {
        if(items != null)
            update(position, Arrays.asList(items));
    }

    public void clear() {
        int n = mData.size();
        mData.clear();
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, n);
    }

    @Override
    public void remove( int position ) {
        mData.remove(position);
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED, position, 1);
    }

    public void remove(int position, int count) {
        mData.subList(position, position + count).clear();
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED, position, count);
    }

    @NonNull
    protected ArrayList<ITEM> getData() {
        return mData;
    }

    @NonNull
    private static <ITEM>
    ArrayList<ITEM> collectionToArrayList(@Nullable Collection<ITEM> collection) {
        ArrayList<ITEM> res;
        if(collection instanceof ArrayList)
            res = (ArrayList<ITEM>) collection;
        else if(collection != null)
            res = new ArrayList<>(collection);
        else
            res = new ArrayList<>(DEFAULT_CAPACITY);
        return res;
    }
}
