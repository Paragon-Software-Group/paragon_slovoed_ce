package com.paragon_software.utils_slovoed.collections;

import java.util.Collection;
import java.util.LinkedList;

public class LimitedLinkedList < E > extends LinkedList < E >
{
    private int limit;

    public LimitedLinkedList( int limit )
    {
        this.limit = limit;
    }

    @Override
    public boolean add( E obj )
    {
        boolean added = super.add(obj);
        removeOverflowItems();
        return added;
    }

    @Override
    public void add( int index, E element )
    {
        super.add(index, element);
        removeOverflowItems();
    }

    @Override
    public boolean addAll( Collection< ? extends E > collection )
    {
        boolean res = super.addAll(collection);
        removeOverflowItems();
        return res;
    }

    @Override
    public boolean addAll( int index, Collection< ? extends E > collection )
    {
        boolean res = super.addAll(index, collection);
        removeOverflowItems();
        return res;
    }

    private void removeOverflowItems()
    {
        while ( size() > limit )
        {
            super.remove(size() - 1);
        }
    }
}