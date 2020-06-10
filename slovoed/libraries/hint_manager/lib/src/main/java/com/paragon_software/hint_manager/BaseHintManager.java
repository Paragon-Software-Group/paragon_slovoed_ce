package com.paragon_software.hint_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import java.util.HashMap;
import java.util.Map;

import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

public class BaseHintManager extends HintManagerAPI
{

  private static final Map< HintType, String > HINT_SETTINGS_KEYS =
      new HashMap< HintType, String >()
      {{
        // WARNING!!! Don't change values of this map.
        // They are used to save hints state in persistent settings.
        put(HintType.History, "need_to_show_hint_history");
        put(HintType.Favorites, "need_to_show_hint_favorites");
        put(HintType.Quiz, "need_to_show_hint_quiz");
        put(HintType.WordOfTheDay, "need_to_show_hint_word_of_the_day");
        put(HintType.News, "need_to_show_hint_news");

        put(HintType.FullTextSearch, "need_to_show_hint_search_fts");
        put(HintType.SimpleSearch, "need_to_show_hint_search_simple");
        put(HintType.SwitchLanguage,"need_to_show_hint_switch_language");

        put(HintType.FavoritesSort, "need_to_show_hint_favorites_sort");

        put(HintType.AddToFavourites, "need_to_show_hint_article_add_to_favorites");
        put(HintType.PronunciationPractice, "need_to_show_hint_article_pronunciation_practice");
        put(HintType.AudioBritish, "need_to_show_hint_article_audio_british");
        put(HintType.AudioAmerican, "need_to_show_hint_article_audio_american");
        put(HintType.AudioWorldEnglish, "need_to_show_hint_article_audio_world_english");
        put(HintType.AudioOnlineStreaming, "need_to_show_hint_article_audio_online_streaming");
        put(HintType.JumpToEntry, "need_to_show_hint_article_jump_to_entry");
        put(HintType.Idioms, "need_to_show_hint_article_idioms");
        put(HintType.PhrasalVerbs, "need_to_show_hint_article_phrasal_verbs");
        put(HintType.ArticleFullScreenView, "need_to_show_hint_article_full_screen_view");
      }};

  @NonNull
  private Map< HintType, HintParams > hintParamsMap = new HashMap<>();

  @Nullable
  private SettingsManagerAPI mSettingsManager;

  @Nullable
  private HintUIHelperAPI mHintUIHelper;

  BaseHintManager( @Nullable HintUIHelperAPI hintUIHelper,
                   @Nullable SettingsManagerAPI settingsManager )
  {
    mHintUIHelper = hintUIHelper;
    mSettingsManager = settingsManager;
  }

  @Override
  void setHintParams( @NonNull HintType hintType, @Nullable HintParams hintParams )
  {
    hintParamsMap.put(hintType, hintParams);
  }

  @Nullable
  @Override
  HintParams getHintParams( @NonNull HintType hintType )
  {
    return hintParamsMap.get(hintType);
  }

  @Override
  public boolean isNeedToShowHint( @NonNull HintType hintType )
  {
    String hintSettingsKey = HINT_SETTINGS_KEYS.get(hintType);
    if ( mSettingsManager != null && hintSettingsKey != null )
    {
      try
      {
        return mSettingsManager.load(hintSettingsKey, true);
      }
      catch ( WrongTypeException | ManagerInitException ignore )
      {
      }
    }
    return false;
  }

  @Override
  public void setNeedToShowHint( @NonNull HintType hintType, boolean show )
  {
    String hintSettingsKey = HINT_SETTINGS_KEYS.get(hintType);
    if ( mSettingsManager != null && hintSettingsKey != null )
    {
      try
      {
        mSettingsManager.save(hintSettingsKey, show, true);
      }
      catch ( LocalResourceUnavailableException | ManagerInitException ignore )
      {
      }
    }
  }

  @Override
  public boolean showHintDialog( @Nullable HintType hintType,
                                 @Nullable FragmentManager fragmentManager,
                                 @Nullable HintParams hintParams )
  {
    if ( mHintUIHelper != null )
    {
      setHintParams(hintType, hintParams);
      return mHintUIHelper.showHintDialog(hintType, fragmentManager);
    }
    return false;
  }
}
