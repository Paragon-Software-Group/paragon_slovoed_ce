package com.paragon_software.flash_cards_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;

import java.io.File;

import io.reactivex.Completable;
import io.reactivex.Single;

public abstract class FlashcardManagerAPI {
    public interface Notifier {}

    public interface ArticleFragmentFactory {
        @NonNull
        Fragment createArticleFragment();
    }

    public abstract void registerFlashcardExchanger(@NonNull FlashcardExchangerAPI flashcardExchanger);
    public abstract void registerScreenOpener(@NonNull ScreenOpenerAPI screenOpener);
    public abstract void registerEngineSerializer(@NonNull EngineSerializerAPI engineSerializer);
    public abstract void registerSettingsManager(@NonNull SettingsManagerAPI settingsManager);
    public abstract void registerDictionaryManager(@NonNull DictionaryManagerAPI dictionaryManager);
    public abstract void registerSoundManager(@NonNull SoundManagerAPI soundManager);
    public abstract void registerArticleFragmentFactory(@NonNull ArticleFragmentFactory factory);

    public abstract void addWord( @NonNull Context context, @NonNull ArticleItem word);
    public abstract void removeWord( @NonNull Context context, @NonNull ArticleItem word);
    public abstract boolean canAddWord(@NonNull ArticleItem word);
    public abstract boolean canRemoveWord(@NonNull ArticleItem word);

    public abstract void registerNotifier(@NonNull Notifier notifier);
    public abstract void unregisterNotifier(@NonNull Notifier notifier);

    @NonNull
    abstract AbstractFlashcardCollectionView getFlashcardCollectionView();

    @NonNull
    abstract AbstractFlashcardCollectionView getFlashcardCollectionView(
        @NonNull Dictionary.DictionaryId dictionaryId );

    abstract void randomizeQuiz();

    @NonNull
    abstract AbstractQuizCollectionView getQuizCollectionView();

    @NonNull
    abstract AbstractQuizCollectionView getQuizCollectionView( @NonNull Dictionary.DictionaryId dictionaryId );

    @Nullable
    abstract ArticleFragmentFactory getArticleFragmentFactory();

    abstract void showQuizArticle( @NonNull ArticleItem ... articleItem );

    abstract void saveSortMode(@NonNull FlashcardMetadata.SortMode sortMode);

    abstract Single<File> exportFlashcards();
    abstract Completable importFlashcards(File file);
}
