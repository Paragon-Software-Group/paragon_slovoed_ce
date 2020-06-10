package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import java.util.HashSet;

public class HashSetMultiselectCollectionView<ITEM,
                                              METADATA,
                                              CORE_COLLECTION extends CollectionView<ITEM, METADATA>>
        extends MultiselectCollectionView<ITEM,
                                          HashSetMultiselectCollectionView.SelectionTransform<ITEM>,
                                          METADATA,
                                          CORE_COLLECTION> {
    public HashSetMultiselectCollectionView(@NonNull CORE_COLLECTION core, boolean defaultSelection) {
        super(core, new HashSetMultiselectCollectionView.SelectionTransform<>(core, defaultSelection));
    }

    @Override
    public boolean isSelected(int position) {
        return getTransformFunction().set.contains(getCore().getItem(position));
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
            selectHelper(position, checked);
            callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, position, 1);
        }
    }

    @Override
    public void selectAll(boolean checked) {
        int n = getCount();
        for(int i = 0; i < n; i++)
            selectHelper(i, checked);
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, 0, n);
    }

    @Override
    public void flip(int position) {
        selectHelper(position, !isSelected(position));
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, position, 1);
    }

    @Override
    public int getSelectionCount() {
        int n = getCount(), res = 0;
        for(int i = 0; i < n; i++)
            if(isSelected(i))
                res++;
        return res;
    }

    private void selectHelper(int position, boolean checked) {
        if(checked)
            getTransformFunction().set.add(getCore().getItem(position));
        else
            getTransformFunction().set.remove(getCore().getItem(position));
    }

    static class SelectionTransform<ITEM>
            implements TransformCollectionView.TransformFunction<MultiselectCollectionView.Selectable<ITEM>, ITEM> {
        private boolean defaultSelection;

        @NonNull
        private final CollectionView<ITEM, ?> collection;

        @NonNull
        private final HashSet<ITEM> set;

        SelectionTransform(@NonNull CollectionView<ITEM, ?> _collection, boolean _defaultSelection) {
            collection = _collection;
            int n = collection.getCount();
            set = new HashSet<>(n);
            defaultSelection = _defaultSelection;
            if(defaultSelection)
                for(int i = 0; i < n; i++)
                    set.add(collection.getItem(i));
        }

        @Override
        public Selectable<ITEM> transform(int position, ITEM argument) {
            return new Selectable<>(argument, set.contains(argument));
        }

        @Override
        public void onItemRangeChanged(OPERATION_TYPE type, int startPosition, int itemCount) {
            if((type.equals(OPERATION_TYPE.ITEM_RANGE_INSERTED) || type.equals(OPERATION_TYPE.ITEM_RANGE_CHANGED)) && defaultSelection)
                for(int i = 0; i < itemCount; i++)
                    set.add(collection.getItem(startPosition + i));
        }
    }
}
