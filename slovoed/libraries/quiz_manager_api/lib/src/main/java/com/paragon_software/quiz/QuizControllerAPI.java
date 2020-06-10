package com.paragon_software.quiz;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;
import android.util.Pair;

import java.util.List;

import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.screen_opener_api.ScreenType;

import io.reactivex.Completable;
import io.reactivex.Observable;

public abstract class QuizControllerAPI
{
    abstract void savePersistState( String key, String state );

    abstract String getPersistState();

    abstract void saveProgress( String progress );

    abstract Observable< QuizProgress > getProgress();

    abstract Observable< List< QuizProgress >>  getProgressList();

    abstract Observable< List < QuizAchievement > > getAchievementList();

    abstract Observable<Float> getListFontSize();

    abstract Observable<Boolean> getClearProgressObservable();

    abstract Boolean isQuizInProgress();

    abstract void setQuizProgressState(boolean inProgress);

    abstract void saveAchievement(String achievementId );

    abstract void setAchievementKind( QuizAchievementKind achievementKind );

    abstract Observable<Boolean> getExitQuizObserver();

    abstract void exitQuiz(boolean isExit);

    abstract void clearProgress();

    abstract Completable openArticle(Context context, String entryId);

    abstract Completable playSound(String soundEntryKey,
                                   String soundBaseId,
                                   boolean isOnline);

    abstract boolean isFullDictionary();

    abstract boolean isAllQuestionsAnswered();

    abstract boolean isExternalBaseDownloaded(String soundBaseId);

    abstract void openManageDownloadsScreen( @NonNull Context context );

    abstract void updateLicensesStatus();

    abstract void close();

    public abstract Observable<Boolean> getSoundState();

    abstract boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );
    abstract Observable< Pair<HintType, HintParams> > getShowHintObservable();

    public abstract void openScreen(ScreenType screenType);
}
