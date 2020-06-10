package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

abstract class AbstractWrapperCollectionView<ITEM,
                                            CORE_ITEM,
                                            METADATA,
                                            CORE_COLLECTION extends CollectionView<CORE_ITEM,
                                                                                   METADATA>>
        extends AbstractCollectionView<ITEM, METADATA>
        implements FullNotifier {
    @NonNull
    private CORE_COLLECTION mCore;

    AbstractWrapperCollectionView(@NonNull CORE_COLLECTION core) {
        mCore = core;
        mCore.registerListener(this);
    }

    @Override
    public int getCount() {
        return mCore.getCount();
    }

    @Override
    public METADATA getMetadata() {
        return mCore.getMetadata();
    }

    @Override
    public int getSelection() {
        return mCore.getSelection();
    }

    @Override
    public boolean isInProgress() {
        return mCore.isInProgress();
    }

    @Override
    public int getPosition() {
        return mCore.getPosition();
    }

    @Override
    public void updateSelection( int selection ) {
        mCore.updateSelection(selection);
    }

    @Override
    public void onItemRangeChanged(OPERATION_TYPE type, int startPosition, int itemCount) {
        callOnItemRangeChanged(type, startPosition, itemCount);
    }

    @Override
    public void onMetadataChanged() {
        callOnMetadataChanged();
    }

    @Override
    public void onSelectionChanged() {
        callOnSelectionChanged();
    }

    @Override
    public void onProgressChanged() {
        callOnProgressChanged();
    }

    @Override
    public void onScrollToPosition(@NonNull CollectionView collectionView) {
        callOnPositionChanged();
    }

    @NonNull
    protected final CORE_COLLECTION getCore() {
        return mCore;
    }

    protected final void setCore(@NonNull CORE_COLLECTION core) {
        mCore = core;
    }
}
