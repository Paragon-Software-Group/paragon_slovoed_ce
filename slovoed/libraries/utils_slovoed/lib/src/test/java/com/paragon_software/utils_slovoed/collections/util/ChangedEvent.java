package com.paragon_software.utils_slovoed.collections.util;

import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.Objects;

class ChangedEvent
{
    private final CollectionView.OPERATION_TYPE type;
    private final int position;
    private final int count;

    ChangedEvent(CollectionView.OPERATION_TYPE _type, int _position, int _count)
    {
        type = _type;
        position = _position;
        count = _count;
    }

    @Override
    public boolean equals(Object obj)
    {
        boolean res = false;
        if(obj instanceof ChangedEvent)
        {
            ChangedEvent other = (ChangedEvent) obj;
            res = Objects.equals(type, other.type) &&
                    (position == other.position) &&
                    (count == other.count);
        }
        return res;
    }

    @Override
    public String toString()
    {
        return type.toString() + ':' + position + ':' + count;
    }
}
