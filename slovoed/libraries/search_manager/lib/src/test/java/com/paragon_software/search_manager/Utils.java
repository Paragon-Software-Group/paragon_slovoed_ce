package com.paragon_software.search_manager;

import com.paragon_software.article_manager.ArticleItem;

public class Utils
{
  public static ArticleItem getTestArticleItem ()
  {
    return new ArticleItem.Builder(null,0,0).build();
  }
}
