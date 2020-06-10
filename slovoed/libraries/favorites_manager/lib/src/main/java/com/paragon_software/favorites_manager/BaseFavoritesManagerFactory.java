package com.paragon_software.favorites_manager;

import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

public class BaseFavoritesManagerFactory implements FavoritesManagerAPI.Factory
{

  @Nullable
  private Class mFavoritesActivity;
  @Nullable
  private DialogFragment mAddInDirectoryDialogFragment;
  @Nullable
  private SearchEngineAPI mEngineSearcher;
  @Nullable
  private HintManagerAPI mHintManager;
  @Nullable
  private String mFileProviderAuthorities;

  @Override
  public FavoritesManagerAPI create(ScreenOpenerAPI screenOpener, EngineSerializerAPI engineSerializer,
                                    SettingsManagerAPI settingsManager, DictionaryManagerAPI dictionaryManager,
                                    ToolbarManager toolbarManager, SoundManagerAPI soundManager,
                                    FavoritesSorterAPI favoritesSorter,
                                    Class<IExportHtmlBuilder> exportHtmlBuilderClass, int maxNumberOfWords )
  {
    return new BaseFavoritesManager( screenOpener, engineSerializer, mEngineSearcher, settingsManager,
                                     dictionaryManager, toolbarManager, soundManager, mHintManager,
                                     favoritesSorter, mFavoritesActivity, mAddInDirectoryDialogFragment,
                                     exportHtmlBuilderClass, mFileProviderAuthorities,  maxNumberOfWords );
  }

  @Override
  public FavoritesManagerAPI.Factory registerSearchEngine( @Nullable SearchEngineAPI engineSearcher )
  {
    mEngineSearcher = engineSearcher;
    return this;
  }

  @Override
  public FavoritesManagerAPI.Factory registerHintManager( @Nullable HintManagerAPI hintManager )
  {
    mHintManager = hintManager;
    return this;
  }

  @Override
  public FavoritesManagerAPI.Factory registerFileProviderAuthorities(@Nullable String fileProviderAuthorities) {
    mFileProviderAuthorities = fileProviderAuthorities;
    return this;
  }

  @Override
  public FavoritesManagerAPI.Factory registerUI(@Nullable Class favoritesActivity) {
    mFavoritesActivity = favoritesActivity;
    return this;
  }

  @Override
  public FavoritesManagerAPI.Factory registerAddInDirectoryUI(
      @Nullable DialogFragment addInDirectoryDialogFragment )
  {
    mAddInDirectoryDialogFragment = addInDirectoryDialogFragment;
    return this;
  }
}
