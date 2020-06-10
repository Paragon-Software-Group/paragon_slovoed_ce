package com.paragon_software.settings_manager;

import android.content.Context;
import android.content.SharedPreferences;

import androidx.preference.PreferenceManager;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;

public class BaseMigrationHelper implements MigrationHelper {


  private static final String CONTAINER_SETTINGS_PREFERENCE_NAME = "mcontainer_settings_preference_name";

  private static final String KEY_FONT_SIZE = "font_size";
  private static final String KEY_LIST_FONT_SIZE = "list_text_size";
  private static final String KEY_KEYBOARD = "key_keyboard";
  private static final String KEY_HIGHLIGHT = "key_highlight";
  private static final String KEY_MATCH_FONT = "key_match_font";

  private static final String KEY_PREFERENCE_ADS_AGREE = "preference_ads_agree";

  @Override
  public void migrateSettings(SettingsManagerAPI settingsManager, Context context) {
    ApplicationSettings applicationSetting = settingsManager.loadApplicationSettings();
    SharedPreferences defaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(context);
    SharedPreferences settingsPreferences = context.getSharedPreferences(CONTAINER_SETTINGS_PREFERENCE_NAME, Context.MODE_PRIVATE);

    float scale;
    float listFont;
    if (defaultSharedPreferences.contains(KEY_LIST_FONT_SIZE)) {
      listFont = defaultSharedPreferences.getFloat(KEY_LIST_FONT_SIZE, ApplicationSettings.getDefaultFontSize());
      applicationSetting.setEntryListFontSize(listFont);
      defaultSharedPreferences.edit().remove(KEY_LIST_FONT_SIZE).apply();
    }

    if (defaultSharedPreferences.contains(KEY_FONT_SIZE)) {
      scale = (float) defaultSharedPreferences.getInt(KEY_FONT_SIZE, -100) / 100;
      applicationSetting.setArticleScale(scale != -1 ? scale : ApplicationSettings.getDefaultArticleScale());
      defaultSharedPreferences.edit().remove(KEY_FONT_SIZE).apply();
    }

    if (defaultSharedPreferences.contains(KEY_KEYBOARD)) {
      applicationSetting.setShowKeyboardForSearchEnabled(defaultSharedPreferences.getBoolean(KEY_KEYBOARD, ApplicationSettings.DEFAULT_SHOW_KEYBOARD_FOR_SEARCH));
      defaultSharedPreferences.edit().remove(KEY_KEYBOARD).apply();
    }

    if (defaultSharedPreferences.contains(KEY_HIGHLIGHT)) {
      applicationSetting.setShowHighlightingEnabled(defaultSharedPreferences.getBoolean(KEY_HIGHLIGHT, ApplicationSettings.DEFAULT_SHOW_HIGHLIGHTING));
      defaultSharedPreferences.edit().remove(KEY_HIGHLIGHT).apply();
    }

    if (defaultSharedPreferences.contains(KEY_MATCH_FONT)) {
      applicationSetting.setMatchFontEnabled(defaultSharedPreferences.getBoolean(KEY_MATCH_FONT, ApplicationSettings.DEFAULT_KEY_MATCH_FONT));
      defaultSharedPreferences.edit().remove(KEY_MATCH_FONT).apply();
    }

    if (settingsPreferences.contains(KEY_PREFERENCE_ADS_AGREE)) {
      applicationSetting.setNewsEnabled(settingsPreferences.getBoolean(KEY_PREFERENCE_ADS_AGREE, ApplicationSettings.DEFAULT_RECEIVE_NEWS));
      settingsPreferences.edit().remove(KEY_PREFERENCE_ADS_AGREE).apply();
    }
    try {
      settingsManager.saveApplicationSettings(applicationSetting);
    } catch (ManagerInitException | LocalResourceUnavailableException e) {
      e.printStackTrace();
    }
  }
}
