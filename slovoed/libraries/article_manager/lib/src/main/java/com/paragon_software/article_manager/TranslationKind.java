package com.paragon_software.article_manager;

enum TranslationKind
{
  None(false, false),
  Article(true, false),
  Popup(false, true);

  private final boolean disableArticleButtons, disableNavigation;

  TranslationKind( boolean _disableArticleButtons, boolean _disableNavigation )
  {
    disableArticleButtons = _disableArticleButtons;
    disableNavigation = _disableNavigation;
  }

  boolean mustDisableArticleButtons()
  {
    return disableArticleButtons;
  }

  boolean mustDisableNavigation()
  {
    return disableNavigation;
  }
}
