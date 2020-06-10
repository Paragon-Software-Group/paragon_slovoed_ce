package com.paragon_software.history_manager;

import androidx.annotation.Nullable;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

public class BaseHistoryManagerFactory implements HistoryManagerAPI.Factory
{

  private ScreenOpenerAPI mScreenOpener;
  private EngineSerializerAPI  mEngineSerializer;
  private SettingsManagerAPI   mSettingsManager;
  private DictionaryManagerAPI mDictionaryManager;
  private ToolbarManager       mToolbarManager;
  private SoundManagerAPI      mSoundManager;
  @Nullable
  private HintManagerAPI       mHintManager;
  private Class                mHistoryActivity;
  private Class<IExportHTMLBuilder> mExportHtmlBuilderClass;

  private int mMaxNumberOfWords;
  private String mAuthoritiesFileProvider;

  @Override
  public HistoryManagerAPI create()
  {
    BaseHistoryManager res = new BaseHistoryManager(mScreenOpener, mEngineSerializer, mSettingsManager,
                                                     mDictionaryManager, mHintManager, mAuthoritiesFileProvider, mMaxNumberOfWords );
    res.registerToolbarManager(mToolbarManager);
    res.registerSoundManager(mSoundManager);
    res.registerUI(mHistoryActivity);
    res.registerExportHTMLBuilderClass(mExportHtmlBuilderClass);
    return res;
  }

  @Override
  public HistoryManagerAPI.Factory registerScreenOpener(@Nullable ScreenOpenerAPI screenOpener)
  {
    mScreenOpener = screenOpener;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerEngineSerializer( @Nullable EngineSerializerAPI engineSerializer )
  {
    mEngineSerializer = engineSerializer;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerSettingsManager( @Nullable SettingsManagerAPI settingsManager )
  {
    mSettingsManager = settingsManager;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerDictionaryManager( @Nullable DictionaryManagerAPI dictionaryManager )
  {
    mDictionaryManager = dictionaryManager;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerSoundManager( @Nullable SoundManagerAPI soundManager )
  {
    mSoundManager = soundManager;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerToolbarManager( @Nullable ToolbarManager toolbarManager )
  {
    mToolbarManager = toolbarManager;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerHintManager( @Nullable HintManagerAPI hintManager )
  {
    mHintManager = hintManager;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerExportHTMLBuilderClass(@Nullable Class<IExportHTMLBuilder> exportHtmlBuilderClass )
  {
    mExportHtmlBuilderClass = exportHtmlBuilderClass;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerUI( @Nullable Class historyActivity )
  {
    mHistoryActivity = historyActivity;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory setMaxNumberOfWords( int maxNumberOfWords )
  {
    mMaxNumberOfWords = maxNumberOfWords;
    return this;
  }

  @Override
  public HistoryManagerAPI.Factory registerFileProviderAuthorities(String authoritiesFileProvider) {
    mAuthoritiesFileProvider = authoritiesFileProvider;
    return this;
  }
}
