package com.paragon_software.article_manager;

import androidx.annotation.NonNull;

final class SearchState
{
  boolean searchModeOn = false;
  @NonNull
  String searchText   = "";
  @NonNull
  String searchResult = "";
  boolean canNavigateResults = false;
}
