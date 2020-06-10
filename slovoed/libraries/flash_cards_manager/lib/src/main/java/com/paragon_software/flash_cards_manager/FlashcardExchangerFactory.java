package com.paragon_software.flash_cards_manager;

import android.content.Context;

import androidx.annotation.NonNull;

import com.paragon_software.native_engine.SyncEngineAPI;

public class FlashcardExchangerFactory implements FlashcardExchangerAPI.Factory {

    private Context mContext;
    private SyncEngineAPI mEngine;

    @Override
    public FlashcardExchangerAPI.Factory registerContext(@NonNull Context context) {
        mContext = context;
        return this;
    }

    @Override
    public FlashcardExchangerAPI.Factory registerEngine(@NonNull SyncEngineAPI engine) {
        mEngine = engine;
        return this;
    }

    @Override
    public FlashcardExchangerAPI create() {
        return new FlashcardExchanger(mContext, mEngine);
    }
}
