package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.List;

public class ReorderCollectionView<ITEM,
                                   METADATA,
                                   CORE_COLLECTION extends CollectionView<ITEM, METADATA>>
        extends AbstractWrapperCollectionView<ITEM, ITEM, METADATA, CORE_COLLECTION> {
    public interface Order<ITEM> {
        void getInitialIndices(@NonNull CollectionView<ITEM, ?> core,
                               @NonNull List<Integer> indices);
        int getWrapperIndex(@NonNull CollectionView<ITEM, ?> core,
                            @NonNull List<Integer> indices,
                            int coreIndex);
        int getWrapperInsertPoint(@NonNull CollectionView<ITEM, ?> core,
                                  @NonNull List<Integer> indices,
                                  int coreIndex);
    }

    @NonNull
    private final Order<ITEM> mOrder;

    @NonNull
    private final List<Integer> mIndices;

    ReorderCollectionView(@NonNull CORE_COLLECTION core,
                          @NonNull Order<ITEM> order) {
        super(core);
        mOrder = order;
        order.getInitialIndices(core, mIndices = new ArrayList<>(core.getCount()));
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
                for(int i = 0; i < mIndices.size(); i++) {
                    int index = mIndices.get(i);
                    if (index >= startPosition)
                        mIndices.set(i, index + itemCount);
                }
                for(int i = 0; i < itemCount; i++) {
                    int index = getWrapperInsertPoint(startPosition + i);
                    mIndices.add(index, startPosition + i);
                    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, index, 1);
                }
            }
            else if(OPERATION_TYPE.ITEM_RANGE_REMOVED.equals(type)) {
                int minAffectedWrapperIndex = -1, maxAffectedWrapperIndex = -1;
                for(int i = 0; i < mIndices.size(); i++) {
                    int index = mIndices.get(i);
                    if((index >= startPosition) && (index < (startPosition + itemCount))) {
                        if ((minAffectedWrapperIndex < 0) || (minAffectedWrapperIndex > i))
                            minAffectedWrapperIndex = i;
                        if (maxAffectedWrapperIndex < i)
                            maxAffectedWrapperIndex = i;
                    }
                }
                List<Integer> subList =
                        new ArrayList<>(maxAffectedWrapperIndex -
                                                minAffectedWrapperIndex + 1 - itemCount);
                for(int i = minAffectedWrapperIndex; i <= maxAffectedWrapperIndex; i++) {
                    int index = mIndices.get(i);
                    if(index < startPosition)
                        subList.add(index);
                    else if(index >= (startPosition + itemCount))
                        subList.add(index - itemCount);
                }
                mIndices.subList(minAffectedWrapperIndex, maxAffectedWrapperIndex + 1).clear();
                for(int i = 0; i < mIndices.size(); i++) {
                    int index = mIndices.get(i);
                    if(index >= (startPosition + itemCount))
                        mIndices.set(i, index - itemCount);
                }
                callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED,
                                       minAffectedWrapperIndex,
                                       maxAffectedWrapperIndex -
                                               minAffectedWrapperIndex + 1);
                if(!subList.isEmpty()) {
                    mIndices.addAll(minAffectedWrapperIndex, subList);
                    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED,
                                           minAffectedWrapperIndex,
                                           subList.size());
                }
            }
            else if(OPERATION_TYPE.ITEM_RANGE_CHANGED.equals(type)) {
                int minAffectedWrapperIndex = -1, maxAffectedWrapperIndex = -1;
                for(int i = 0; i < mIndices.size(); i++) {
                    int coreIndex = mIndices.get(i);
                    if((coreIndex >= startPosition) && (coreIndex < (startPosition + itemCount))) {
                        if ((minAffectedWrapperIndex < 0) || (minAffectedWrapperIndex > i))
                            minAffectedWrapperIndex = i;
                        if (maxAffectedWrapperIndex < i)
                            maxAffectedWrapperIndex = i;
                    }
                }
                List<Integer> subList = new ArrayList<>(maxAffectedWrapperIndex -
                                                                minAffectedWrapperIndex + 1 - itemCount);
                for(int i = minAffectedWrapperIndex; i <= maxAffectedWrapperIndex; i++) {
                    int coreIndex = mIndices.get(i);
                    if((coreIndex < startPosition) || (coreIndex >= (startPosition + itemCount)))
                        subList.add(coreIndex);
                }
                mIndices.subList(minAffectedWrapperIndex, maxAffectedWrapperIndex + 1).clear();
                boolean canCallChanged = false;
                if(subList.isEmpty())
                    canCallChanged = true;
                    for(int i = 0; i < itemCount; i++) {
                        int coreIndex = startPosition + i;
                        int wrapperIndex = getWrapperInsertPoint(coreIndex);
                        if(wrapperIndex != minAffectedWrapperIndex) {
                            canCallChanged = false;
                            break;
                        }
                    }
                if(canCallChanged) {
                    for (int i = 0; i < itemCount; i++) {
                        int coreIndex = startPosition + i;
                        int wrapperIndex = getWrapperInsertPoint(coreIndex);
                        mIndices.add(wrapperIndex, coreIndex);
                    }
                    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED,
                                           minAffectedWrapperIndex,
                                           maxAffectedWrapperIndex - minAffectedWrapperIndex + 1);
                }
                else {
                    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED,
                                           minAffectedWrapperIndex,
                                           maxAffectedWrapperIndex - minAffectedWrapperIndex + 1);
                    if (!subList.isEmpty()) {
                        mIndices.addAll(minAffectedWrapperIndex, subList);
                        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED,
                                               minAffectedWrapperIndex,
                                               subList.size());
                    }
                    for (int i = 0; i < itemCount; i++) {
                        int coreIndex = startPosition + i;
                        int wrapperIndex = getWrapperInsertPoint(coreIndex);
                        mIndices.add(wrapperIndex, coreIndex);
                        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, wrapperIndex, 1);
                    }
                }
            }
    }

    protected int getCoreIndex(int wrapperIndex) {
        return mIndices.get(wrapperIndex);
    }

    protected int getWrapperIndex(int coreIndex) {
        return mOrder.getWrapperIndex(getCore(), mIndices, coreIndex);
    }

    protected int getWrapperInsertPoint(int coreIndex) {
        return mOrder.getWrapperInsertPoint(getCore(), mIndices, coreIndex);
    }
}
