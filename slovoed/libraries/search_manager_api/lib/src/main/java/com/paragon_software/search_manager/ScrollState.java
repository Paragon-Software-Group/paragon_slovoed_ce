package com.paragon_software.search_manager;

public class ScrollState {

    private final int mPosition;
    private final int mOffset;

    public ScrollState(int position, int offset)
    {
        mPosition = position;
        mOffset = offset;
    }

    public int getPosition()
    {
        return mPosition;
    }

    public int getOffset()
    {
        return mOffset;
    }
}
