package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class FilterCollectionView<ITEM,
                                  METADATA,
                                  CORE_COLLECTION extends CollectionView<ITEM, METADATA>>
        extends AbstractWrapperCollectionView<ITEM, ITEM, METADATA, CORE_COLLECTION> {
    public interface Predicate<ITEM> {
        boolean apply(ITEM item);
    }

    @NonNull
    private final List<Integer> mIndices;

    @NonNull
    private final Predicate<ITEM> mPredicate;

    public FilterCollectionView( @NonNull CORE_COLLECTION core, @NonNull Predicate< ITEM > predicate ) {
        super(core);
        mPredicate = predicate;
        int n = core.getCount();
        mIndices = new ArrayList<>(n);
        for(int i = 0; i < n; i++)
            if(mPredicate.apply(core.getItem(i)))
                mIndices.add(i);
    }

    @Override
    public int getCount() {
        return mIndices.size();
    }

    @Override
    public ITEM getItem(int position) {
        return getCore().getItem(getCoreIndex(position));
    }

    @Override
    public int getSelection() {
        return getWrapperIndex(super.getSelection());
    }

    @Override
    public int getPosition() {
        return getWrapperIndex(super.getPosition());
    }

    @Override
    public void updateSelection(int selection) {
        super.updateSelection(getCoreIndex(selection));
    }

    @Override
    public void onItemRangeChanged(OPERATION_TYPE type, int startPosition, int itemCount) {
        if(itemCount > 0)
            if(OPERATION_TYPE.ITEM_RANGE_INSERTED.equals(type)) {
                List<Integer> newIndices = new ArrayList<>(itemCount);
                for(int i = 0; i < itemCount; i++)
                    if(mPredicate.apply(getCore().getItem(startPosition + i)))
                        newIndices.add(startPosition + i);
                if(!newIndices.isEmpty()) {
                    int where = getWrapperInsertPoint(startPosition);
                    mIndices.addAll(where, newIndices);
                    for(int i = where + newIndices.size(); i < mIndices.size(); i++)
                        mIndices.set(i, mIndices.get(i) + itemCount);
                    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED,
                                           where,
                                           newIndices.size());
                }
            }
            else if(OPERATION_TYPE.ITEM_RANGE_REMOVED.equals(type)) {
                int where1 = getWrapperInsertPoint(startPosition);
                int where2 = getWrapperInsertPoint(startPosition + itemCount);
                if(where1 < where2) {
                    mIndices.subList(where1, where2).clear();
                    for(int i = where1; i < mIndices.size(); i++)
                        mIndices.set(i, mIndices.get(i) - itemCount);
                    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED,
                                           where1,
                                           where2 - where1);
                }
            }
            else if(OPERATION_TYPE.ITEM_RANGE_CHANGED.equals(type)) {
                List<Integer> newIndices = new ArrayList<>(itemCount);
                for(int i = 0; i < itemCount; i++)
                    if(mPredicate.apply(getCore().getItem(startPosition + i)))
                        newIndices.add(startPosition + i);
                int where1 = getWrapperInsertPoint(startPosition);
                int where2 = getWrapperInsertPoint(startPosition + itemCount);
                List<Integer> affectedSublist = mIndices.subList(where1, where2);
                if(newIndices.equals(affectedSublist) && (!affectedSublist.isEmpty()))
                    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED,
                                           where1,
                                           where2 - where1);
                else {
                    if (!affectedSublist.isEmpty()) {
                        affectedSublist.clear();
                        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED,
                                               where1,
                                               where2 - where1);
                    }
                    if (!newIndices.isEmpty()) {
                        mIndices.addAll(where1, newIndices);
                        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED,
                                               where1,
                                               newIndices.size());
                    }
                }
            }
    }

    protected int getCoreIndex(int wrapperIndex) {
        return mIndices.get(wrapperIndex);
    }

    protected int getWrapperIndex(int coreIndex) {
        return Collections.binarySearch(mIndices, coreIndex);
    }

    protected int getWrapperInsertPoint(int coreIndex) {
        int res = getWrapperIndex(coreIndex);
        if(res < 0)
            res = -res - 1;
        return res;
    }
}
