package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

final class FlashcardManagerHolder
{
    @Nullable
    private static FlashcardManagerAPI mManager = null;

    static void set( @NonNull FlashcardManagerAPI flashcardManager )
    {
        if( mManager == null )
            mManager = flashcardManager;
        else
            throw new IllegalStateException("Can't have more than one instance");
    }

    @NonNull
    static FlashcardManagerAPI get()
    {
        if( mManager != null )
            return mManager;
        else
            throw new IllegalStateException("FlashcardManager must be initialized at this stage");
    }
}
