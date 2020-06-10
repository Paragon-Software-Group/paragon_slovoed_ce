package com.paragon_software.article_manager;

import androidx.annotation.NonNull;

class TransitionState
{
  @NonNull
  TranslationKind translation = TranslationKind.None;
  boolean favoritesBusy = false;
  boolean soundBusy     = false;
}
