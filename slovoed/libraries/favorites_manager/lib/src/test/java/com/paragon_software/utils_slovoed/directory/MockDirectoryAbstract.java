package com.paragon_software.utils_slovoed.directory;

import com.paragon_software.article_manager.ArticleItem;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public abstract class MockDirectoryAbstract extends Directory<ArticleItem>
{
  public MockDirectoryAbstract(@Nullable Directory< ArticleItem > parent, @NonNull String name )
  {
    super(parent, name);
  }
}