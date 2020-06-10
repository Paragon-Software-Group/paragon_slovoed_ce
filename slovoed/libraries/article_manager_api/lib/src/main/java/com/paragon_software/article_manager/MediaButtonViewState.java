package com.paragon_software.article_manager;

public class MediaButtonViewState
{
    private final int              mButtonIndex;
    private final MediaButtonState mButtonState;

    public MediaButtonViewState(int mButtonIndex, MediaButtonState mButtonState)
    {
        this.mButtonIndex = mButtonIndex;
        this.mButtonState = mButtonState;
    }

    public int getButtonIndex()
    {
        return mButtonIndex;
    }

    public MediaButtonState getButtonState()
    {
        return mButtonState;
    }
}
