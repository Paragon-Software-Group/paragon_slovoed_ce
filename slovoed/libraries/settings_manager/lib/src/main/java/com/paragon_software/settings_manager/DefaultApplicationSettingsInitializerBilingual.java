package com.paragon_software.settings_manager;

public class DefaultApplicationSettingsInitializerBilingual
    extends DefaultApplicationSettingsInitializer
{
  @Override
  public boolean initialize()
  {
    ApplicationSettings.DEFAULT_FONT_SIZE = 16;
    ApplicationSettings.MIN_FONT_SIZE = 0.45f * ApplicationSettings.DEFAULT_FONT_SIZE;
    ApplicationSettings.MAX_FONT_SIZE = 2f * ApplicationSettings.DEFAULT_FONT_SIZE;

    ApplicationSettings.MIN_ARTICLE_SCALE = 0.45f * ApplicationSettings.DEFAULT_ARTICLE_SCALE;
    ApplicationSettings.MAX_ARTICLE_SCALE = 2f * ApplicationSettings.DEFAULT_ARTICLE_SCALE;

    ApplicationSettings.DEFAULT_SHOW_KEYBOARD_FOR_SEARCH = false;
    return true;
  }
}
