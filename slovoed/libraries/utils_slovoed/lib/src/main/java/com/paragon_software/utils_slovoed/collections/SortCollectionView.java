package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class SortCollectionView<ITEM,
                                METADATA,
                                CORE_COLLECTION extends CollectionView<ITEM, METADATA>>
        extends ReorderCollectionView<ITEM, METADATA, CORE_COLLECTION> {
    public SortCollectionView(@NonNull CORE_COLLECTION core, @NonNull Comparator<ITEM> comparator) {
        super(core, new SortedOrder<>(comparator));
    }

    private static class SortedOrder<ITEM> implements Order<ITEM> {
        @NonNull
        private final Comparator<ITEM> mComparator;

        SortedOrder(@NonNull Comparator<ITEM> comparator) {
            mComparator = comparator;
        }

        @Override
        public void getInitialIndices(@NonNull CollectionView<ITEM, ?> core, @NonNull List<Integer> indices) {
            indices.clear();
            for(int i = 0; i < core.getCount(); i++)
                indices.add(i);
            Collections.sort(indices, new IndexComparator<>(core, mComparator));
        }

        @Override
        public int getWrapperIndex(@NonNull CollectionView<ITEM, ?> core, @NonNull List<Integer> indices, int coreIndex) {
            int res = -1;
            if((coreIndex >= 0) && (coreIndex < core.getCount()))
                res = Collections.binarySearch(indices, coreIndex, new IndexComparator<>(core, mComparator));
            return res;
        }

        @Override
        public int getWrapperInsertPoint(@NonNull CollectionView<ITEM, ?> core, @NonNull List<Integer> indices, int coreIndex) {
            int res = getWrapperIndex(core, indices, coreIndex);
            if(res < 0)
                res = -res - 1;
            return res;
        }
    }

    private static class IndexComparator<ITEM> implements Comparator<Integer> {
        @NonNull
        private final CollectionView<ITEM, ?> mCore;

        @NonNull
        private final Comparator<ITEM> mComparator;

        IndexComparator(@NonNull CollectionView<ITEM, ?> core, @NonNull Comparator<ITEM> comparator) {
            mCore = core;
            mComparator = comparator;
        }

        @Override
        public int compare(Integer o1, Integer o2) {
            return mComparator.compare(mCore.getItem(o1), mCore.getItem(o2));
        }
    }
}
