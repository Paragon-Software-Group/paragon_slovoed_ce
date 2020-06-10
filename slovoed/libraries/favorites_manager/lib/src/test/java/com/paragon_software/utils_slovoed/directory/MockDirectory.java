package com.paragon_software.utils_slovoed.directory;

import com.paragon_software.article_manager.ArticleItem;

import java.util.List;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class MockDirectory extends Directory<ArticleItem> {
  public MockDirectory(@Nullable Directory<ArticleItem> parent, @NonNull String name) {
    super(parent, name);
  }

  public void setChildList(List<Directory<ArticleItem>> childList) {
    mChildList = childList;
  }

  public void setArticle(List<ArticleItem> item) {
    mItems = item;
  }

  @NonNull
  @Override
  public List<ArticleItem> getItems() {
    return mItems;
  }
}