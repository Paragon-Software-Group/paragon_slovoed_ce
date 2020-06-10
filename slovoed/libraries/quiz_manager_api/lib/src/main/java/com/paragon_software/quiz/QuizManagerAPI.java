package com.paragon_software.quiz;


import android.app.Activity;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;

import java.util.List;

import io.reactivex.Observable;

public abstract class QuizManagerAPI
{
    public interface Factory
    {
        QuizManagerAPI create();
        Factory registerDictionaryManager(DictionaryManagerAPI dictionaryManager);
        Factory registerSettingsManager(SettingsManagerAPI settingsManager);
        Factory registerScreenOpener(ScreenOpenerAPI screenOpener);
        Factory registerArticleEngine(EngineArticleAPI articleEngine);
        Factory registerSoundManager(SoundManagerAPI soundManager);
        Factory registerHintManager( @Nullable HintManagerAPI hintManager );
        Factory registerQuizManagerUI(@NonNull Class<? extends Activity> mainActivity);
        Factory registerQuizAchievementsGenerator( @Nullable QuizAchievementsGeneratorAPI generator);
        Factory registerQuizProgressListFactory( @Nullable QuizProgressListFactoryAPI factory );
    }

    abstract QuizControllerAPI getController( String controllerId );

    abstract void savePersistState( String key, String state );

    abstract String getPersistState();

    abstract void saveProgress( String progress );

    abstract Observable< List < QuizProgress > > getProgressList();

    abstract void saveAchievement( String achievementId );

    abstract Observable< List < QuizAchievement > > getAchievementList();

    abstract void reloadAchievementList();

    abstract void clearProgress();
}
