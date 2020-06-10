package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import java.util.BitSet;

public class StableIndexMultiselectCollectionView<ITEM,
                                       METADATA,
                                       CORE_COLLECTION extends CollectionView<ITEM, METADATA>>
        extends MultiselectCollectionView<ITEM,
                                          StableIndexMultiselectCollectionView.SelectionTransform<ITEM>,
                                          METADATA,
                                          CORE_COLLECTION> {
    public StableIndexMultiselectCollectionView(@NonNull CORE_COLLECTION core, boolean defaultSelection) {
        super(core, new StableIndexMultiselectCollectionView.SelectionTransform<ITEM>(core.getCount(), defaultSelection));
    }

    @Override
    public boolean isSelected(int position) {
        return getTransformFunction().bitSet.get(position);
    }

    @Override
    public void select(int position) {
        select(position, true);
    }

    @Override
    public void unselect(int position) {
        select(position, false);
    }

    @Override
    public void select(int position, boolean checked) {
        boolean isSelected = isSelected(position);
        if(isSelected != checked) {
            getTransformFunction().bitSet.set(position, checked);
            callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, position, 1);
        }
    }

    @Override
    public void selectAll(boolean checked) {
        int n = getCount();
        getTransformFunction().bitSet.set(0, n, checked);
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, 0, n);
    }

    @Override
    public void flip(int position) {
        getTransformFunction().bitSet.flip(position);
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, position, 1);
    }

    @Override
    public int getSelectionCount() {
        return getTransformFunction().bitSet.cardinality();
    }

    static class SelectionTransform<ITEM>
            implements TransformCollectionView.TransformFunction<MultiselectCollectionView.Selectable<ITEM>, ITEM> {
        private boolean defaultSelection;

        @NonNull
        private final BitSet bitSet;

        SelectionTransform(int _initialCount, boolean _defaultSelection) {
            bitSet = new BitSet(_initialCount);
            defaultSelection = _defaultSelection;
            if(defaultSelection)
                bitSet.set(0, _initialCount, true);
        }

        @Override
        public Selectable<ITEM> transform(int position, ITEM argument) {
            return new Selectable<>(argument, bitSet.get(position));
        }

        @Override
        public void onItemRangeChanged(OPERATION_TYPE type, int startPosition, int itemCount) {
            if(itemCount > 0) {
                int n = bitSet.length();
                if (OPERATION_TYPE.ITEM_RANGE_INSERTED.equals(type)) {
                    for (int i = n - 1; i >= startPosition; i--)
                        bitSet.set(i + itemCount, bitSet.get(i));
                    if(defaultSelection)
                        bitSet.set(startPosition, startPosition + itemCount, true);
                }
                else if (OPERATION_TYPE.ITEM_RANGE_REMOVED.equals(type) && (startPosition < n))
                    if ((startPosition + itemCount) >= n)
                        bitSet.clear(startPosition, n);
                    else {
                        for (int i = startPosition + itemCount; i < n; i++)
                            bitSet.set(i - itemCount, bitSet.get(i));
                        bitSet.clear(n - itemCount, n);
                    }
            }
        }
    }
}
