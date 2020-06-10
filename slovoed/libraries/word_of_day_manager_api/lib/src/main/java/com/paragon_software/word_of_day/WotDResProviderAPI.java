package com.paragon_software.word_of_day;

import android.content.Context;

import io.reactivex.annotations.NonNull;

public abstract class WotDResProviderAPI
{
    public interface Factory
    {
        WotDResProviderAPI create();

        Factory registerContext( @NonNull Context context );
    }

    public abstract String getStringRes(WotDRes wotDRes );

    public abstract int getResId(WotDRes wotDRes );
}
