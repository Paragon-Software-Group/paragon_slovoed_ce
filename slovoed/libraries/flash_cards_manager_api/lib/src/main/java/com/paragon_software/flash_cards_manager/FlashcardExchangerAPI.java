package com.paragon_software.flash_cards_manager;

import android.content.Context;
import android.net.Uri;

import androidx.annotation.NonNull;

import com.paragon_software.native_engine.SyncEngineAPI;

import java.io.File;
import java.util.List;

import io.reactivex.Single;

public interface FlashcardExchangerAPI {
    interface Factory {
        Factory registerContext(@NonNull Context context);
        Factory registerEngine(@NonNull SyncEngineAPI engine);
        FlashcardExchangerAPI create();
    }

    Single< File > getExportFlashcardsFile(String productId, List< FlashcardBox > flashcardBoxes );
    Single< List< FlashcardBox > > importFlashcards( Uri fileUri );
}
