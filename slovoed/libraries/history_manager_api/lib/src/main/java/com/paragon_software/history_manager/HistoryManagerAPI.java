package com.paragon_software.history_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;


public abstract class HistoryManagerAPI
{
  public interface Factory
  {
    HistoryManagerAPI create();

    Factory registerScreenOpener(@Nullable ScreenOpenerAPI screenOpener);

    Factory registerEngineSerializer( @Nullable EngineSerializerAPI engineSerializer );

    Factory registerSettingsManager( @Nullable SettingsManagerAPI settingsManager );

    Factory registerDictionaryManager( @Nullable DictionaryManagerAPI dictionaryManager );

    Factory registerSoundManager( @Nullable SoundManagerAPI soundManager );

    Factory registerToolbarManager( @Nullable ToolbarManager toolbarManager );

    Factory registerHintManager( @Nullable HintManagerAPI hintManager );

    Factory registerUI( @Nullable Class historyActivity );

    Factory registerExportHTMLBuilderClass(@Nullable Class<IExportHTMLBuilder> exportHtmlBuilder );

    Factory setMaxNumberOfWords( int maxNumberOfWords );

    Factory registerFileProviderAuthorities(String authoritiesFileProvider);
  }

  public abstract void registerNotifier( @NonNull Notifier notifier );

  public abstract void unregisterNotifier( @NonNull Notifier notifier );

  @NonNull
  public abstract List< ArticleItem > getWords();

  public abstract boolean addWord( @NonNull ArticleItem word );

  public abstract boolean addWords( @NonNull List< ArticleItem > words );

  public abstract boolean removeWords( @NonNull List< ArticleItem > words );

  public abstract boolean removeAllWords();

  public abstract boolean isInTransition();

  public abstract void selectionModeOff();

  public abstract boolean showHistoryScreen( @Nullable Context context );

  @NonNull
  abstract HistoryControllerAPI getController( @NonNull String uiName );

  abstract void freeController( @NonNull String uiName );

  interface Notifier
  {
  }
}
