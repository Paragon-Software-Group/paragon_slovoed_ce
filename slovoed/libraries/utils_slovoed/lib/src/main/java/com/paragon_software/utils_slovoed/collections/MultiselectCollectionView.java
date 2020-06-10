package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

public abstract class MultiselectCollectionView<ITEM,
                                                TRANSFORM_FUNCTION
                                                        extends TransformCollectionView.TransformFunction<MultiselectCollectionView.Selectable<ITEM>,
                                                                                                          ITEM>,
                                                METADATA,
                                                CORE_COLLECTION extends CollectionView<ITEM, METADATA>>
            extends TransformCollectionView<MultiselectCollectionView.Selectable<ITEM>,
                                            ITEM,
                                            TRANSFORM_FUNCTION,
                                            METADATA,
                                            CORE_COLLECTION> {
    public static class Selectable<ITEM> {
        public final ITEM item;
        public final boolean isSelected;

        Selectable(ITEM _item, boolean _isSelected){
            item = _item;
            isSelected = _isSelected;
        }
    }

    public MultiselectCollectionView(@NonNull CORE_COLLECTION core, @NonNull TRANSFORM_FUNCTION transformFunction) {
        super(core, transformFunction);
    }

    public abstract boolean isSelected(int position);

    public abstract void select(int position);

    public abstract void unselect(int position);

    public abstract void select(int position, boolean checked);

    public abstract void selectAll(boolean checked);

    public abstract void flip(int position);

    public abstract int getSelectionCount();
}
