package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;

public abstract class CollectionViewRecyclerAdapter<COLLECTION_VIEW extends CollectionView<?, ?>,
                                                    VIEW_HOLDER extends RecyclerView.ViewHolder>
        extends RecyclerView.Adapter<VIEW_HOLDER>
        implements CollectionView.OnItemRangeChanged,
                   CollectionView.OnSelectionChange {
    @Nullable
    private COLLECTION_VIEW data = null;

    private Integer countCap = null, selected;

    public CollectionViewRecyclerAdapter() {
        this(false);
    }

    public CollectionViewRecyclerAdapter(boolean trackSelection) {
        selected = trackSelection ? -1 : null;
    }

    public final void setData(@Nullable COLLECTION_VIEW newData) {
        int oldCount = 0, newCount = 0;
        if(data != null) {
            oldCount = data.getCount();
            data.unregisterListener(this);
        }
        if(newData != null)
            newCount = newData.getCount();
        int minCount = Math.min(oldCount, newCount);
        countCap = minCount;
        if(oldCount > newCount)
            notifyItemRangeRemoved(newCount, oldCount - newCount);
        data = newData;
        if(minCount > 0)
            notifyItemRangeChanged(0, minCount);
        countCap = null;
        if(oldCount < newCount)
            notifyItemRangeInserted(oldCount, newCount - oldCount);
        if(data != null) {
            data.registerListener(this);
            if(selected != null)
                selected = data.getSelection();
        }
    }

    @Override
    public int getItemCount() {
        int res = 0;
        if(data != null) {
            res = data.getCount();
            if((countCap != null) && (countCap < res))
                res = countCap;
        }
        return res;
    }

    @Override
    public void onItemRangeChanged(CollectionView.OPERATION_TYPE type,
                                   int startPosition,
                                   int itemCount) {
        switch (type) {
            case ITEM_RANGE_INSERTED:
                notifyItemRangeInserted(startPosition, itemCount);
                break;
            case ITEM_RANGE_CHANGED:
                notifyItemRangeChanged(startPosition, itemCount);
                break;
            case ITEM_RANGE_REMOVED:
                notifyItemRangeRemoved(startPosition, itemCount);
                break;
        }
    }

    @Override
    public void onSelectionChanged() {
        if((data != null) && (selected != null)) {
            int newSelected = data.getSelection();
            if(newSelected != selected) {
                notifyItemChanged(selected);
                selected = newSelected;
                notifyItemChanged(selected);
            }
        }
    }

    @Nullable
    protected COLLECTION_VIEW getData() {
        return data;
    }
}
