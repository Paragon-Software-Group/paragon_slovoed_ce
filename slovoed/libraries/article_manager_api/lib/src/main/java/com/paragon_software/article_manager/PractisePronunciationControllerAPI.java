package com.paragon_software.article_manager;


import io.reactivex.Observable;

public interface PractisePronunciationControllerAPI
{
    void mediaButtonClick(String buttonId);
    Observable<MediaButtonViewState> getMediaButtonState();
    void pause();
    void release();

    void restoreButtonsState();
}
