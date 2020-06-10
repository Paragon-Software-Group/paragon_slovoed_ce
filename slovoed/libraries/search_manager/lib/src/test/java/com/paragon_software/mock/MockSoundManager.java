package com.paragon_software.mock;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.sound_manager.SoundManagerAPI;

public abstract class MockSoundManager implements SoundManagerAPI
{
    public int itemHasSoundCounter = 0;
    public int playSoundCounter = 0;

    @Override
    public boolean playSound(@NonNull ArticleItem currentItem) {
        ++playSoundCounter;
        return false;
    }

    @Override
    public boolean itemHasSound(@NonNull ArticleItem item) {
        ++itemHasSoundCounter;
        return false;
    }
}
