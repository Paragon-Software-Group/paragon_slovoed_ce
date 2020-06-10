package com.paragon_software.article_manager;

import androidx.annotation.NonNull;

interface SearchTarget
{
  void search( @NonNull String text );

  void findNext();

  void findPrevious();

  void clearMatches();

  boolean canNavigate();

  @NonNull
  String getResult();

  void registerListener( @NonNull OnStateChangedListener listener );

  void unregisterListener( @NonNull OnStateChangedListener listener );

  interface OnStateChangedListener
  {
    void onSearchStateChanged();
  }
}
