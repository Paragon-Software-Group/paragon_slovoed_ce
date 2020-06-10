package com.paragon_software.article_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.text.SpannableString;

import com.paragon_software.settings_manager.ApplicationSettings;

class Article
{
  @NonNull
  final ArticleStack stack = new ArticleStack();

  @Nullable
  String data = null;

  @Nullable
  String showVariantText = null;

  boolean isLocked = false;

  @Nullable
  String ftsAnchor = null;

  boolean isArticleInFavorites = false;

  boolean hasHideOrSwitchBlocks = false;

  boolean hasSound = false;

  @Nullable
  SpannableString backRunningHeadHeader = null;

  @Nullable
  SpannableString forwardRunningHeadHeader = null;

  boolean canAddWordFlashcard;

  boolean canRemoveWordFlashcard;

  float scale = ApplicationSettings.getDefaultArticleScale();
  // 'General' settings
  boolean usePinchToZoom = true;
  boolean showHighlighting = true;

  // 'My View' settings
  boolean myViewEnabled = true;
  boolean hidePronunciations = false;
  boolean hideExamples = false;
  boolean hidePictures = false;
  boolean hideIdioms = false;
  boolean hidePhrasalVerbs = false;

  String[] hideSoundIcons = new String[]{};
}
