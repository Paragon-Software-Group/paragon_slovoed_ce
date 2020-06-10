package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;

public class DefaultApplicationSettingsInitializerOALD10
    extends DefaultApplicationSettingsInitializer
{
  @Override
  public boolean initialize()
  {
    ApplicationSettings.DEFAULT_FONT_SIZE = 16;
    ApplicationSettings.MIN_FONT_SIZE = 0.88f * ApplicationSettings.DEFAULT_FONT_SIZE;
    ApplicationSettings.MAX_FONT_SIZE = 1.36f * ApplicationSettings.DEFAULT_FONT_SIZE;

    ApplicationSettings.MIN_ARTICLE_SCALE = 0.88f * ApplicationSettings.DEFAULT_ARTICLE_SCALE;
    ApplicationSettings.MAX_ARTICLE_SCALE = 1.36f * ApplicationSettings.DEFAULT_ARTICLE_SCALE;

    ApplicationSettings.DEFAULT_SHOW_KEYBOARD_FOR_SEARCH = false;
    return true;
  }
}
