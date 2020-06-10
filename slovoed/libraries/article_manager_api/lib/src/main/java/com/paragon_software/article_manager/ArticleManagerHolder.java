package com.paragon_software.article_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

/*
 * article_manager_api
 *
 *  Created on: 16.05.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */
public final class ArticleManagerHolder
{
  @Nullable
  private static ArticleManagerAPI manager = null;

  @Nullable
  public static ArticleManagerAPI getManager()
  {
    return manager;
  }

  public static void setManager( @NonNull ArticleManagerAPI manager )
  {
    ArticleManagerHolder.manager = manager;
  }
}
