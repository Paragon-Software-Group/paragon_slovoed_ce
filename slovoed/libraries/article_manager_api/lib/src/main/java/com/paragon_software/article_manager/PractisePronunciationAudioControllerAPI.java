package com.paragon_software.article_manager;

import androidx.annotation.NonNull;

public interface PractisePronunciationAudioControllerAPI
{
    boolean startRecord(int id);
    boolean stopRecord();

    void startPlaying(int id, @NonNull Runnable onAudioPlayComplete);
    void stopPlaying();

    void release();
}
