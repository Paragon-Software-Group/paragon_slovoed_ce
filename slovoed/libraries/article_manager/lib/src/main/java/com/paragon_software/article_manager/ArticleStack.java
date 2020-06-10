package com.paragon_software.article_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.util.Pair;

import java.util.ArrayList;
import java.util.List;


class ArticleStack
{
  private @NonNull
  List< Pair<ArticleItem, ShowArticleOptions> > articleItems = new ArrayList<>();

  {
    articleItems.add(null);
  }

  private int currentItemIdx = 0;

  @Nullable
  ArticleItem getCurrentItem()
  {
    return extractArticleItem(currentItemIdx);
  }

  @Nullable
  public ShowArticleOptions getCurrentShowArticleOptions()
  {
    return extractShowArticleOptions(currentItemIdx);
  }

  @Nullable
  private ArticleItem extractArticleItem( int index )
  {
    ArticleItem res = null;
    if ( (index >= 0) && (index < articleItems.size()) )
    {
      Pair< ArticleItem, ShowArticleOptions > pair = articleItems.get(index);
      if (null != pair)
      {
        res = pair.first;
      }
    }
    return res;
  }

  @Nullable
  private ShowArticleOptions extractShowArticleOptions( int index )
  {
    ShowArticleOptions res = null;
    if ( (index >= 0) && (index < articleItems.size()) )
    {
      Pair< ArticleItem, ShowArticleOptions > pair = articleItems.get(index);
      if (null != pair)
      {
        res = pair.second;
      }
    }
    return res;
  }

  @Nullable
  ArticleItem getNextItem() {
      ArticleItem res = null;
      if ( canGoForward() ) {
          res = extractArticleItem(currentItemIdx + 1);
      }
      return res;
  }

  @Nullable
  ArticleItem getPreviousItem() {
      ArticleItem res = null;
      if ( canGoBack() ) {
          res = extractArticleItem(currentItemIdx - 1);
      }
      return res;
  }

  @Nullable
  ArticleItem goToNext() {
      ArticleItem res = null;
      if ( canGoForward() ) {
          res = extractArticleItem(++currentItemIdx);
      }
      return res;
  }

  @Nullable
  ArticleItem goToPrevious() {
      ArticleItem res = null;
      if ( canGoBack() ) {
          res = extractArticleItem(--currentItemIdx);
      }
      return res;
  }

  public ShowArticleOptions getPreviousShowArticleOptions()
  {
    ShowArticleOptions res = null;
    if ( canGoBack() )
    {
      res = extractShowArticleOptions(currentItemIdx - 1);
    }
    return res;
  }

  public ShowArticleOptions getNextShowArticleOptions()
  {
    ShowArticleOptions res = null;
    if ( canGoForward() )
    {
      res = extractShowArticleOptions(currentItemIdx + 1);
    }
    return res;
  }

  boolean canGoBack()
  {
    return currentItemIdx > 0;
  }

  boolean canGoForward()
  {
    return ( currentItemIdx + 1 ) < articleItems.size();
  }

  void nextTranslation( @Nullable ArticleItem article, boolean clearFollowingItems, @Nullable ShowArticleOptions showArticleOptions )
  {
    if ( clearFollowingItems )
    {
      for ( int i = articleItems.size() - 1 ; i > currentItemIdx ; i-- )
      {
        articleItems.remove(i);
      }
    }
    ArticleItem current = getCurrentItem();
    if ( current != null )
    {
      if ( !current.equals(article) )
      {
        articleItems.add(currentItemIdx + 1, null);
        currentItemIdx++;
      }
    }
    articleItems.set(currentItemIdx, new Pair<>(article, showArticleOptions));
  }

  void back( @Nullable ArticleItem article, @Nullable ShowArticleOptions showArticleOptions )
  {
    if ( !canGoBack() )
    {
      articleItems.add(0, null);
      currentItemIdx++;
    }
    boolean currentIsNull = ( getCurrentItem() == null );
    currentItemIdx--;
    articleItems.set(currentItemIdx, new Pair<>(article, showArticleOptions));
    if ( currentIsNull )
    {
      articleItems.remove(currentItemIdx + 1);
    }
  }

  void forward( @Nullable ArticleItem article, @Nullable ShowArticleOptions showArticleOptions )
  {
    if ( !canGoForward() )
    {
      articleItems.add(null);
    }
    boolean currentIsNull = ( getCurrentItem() == null );
    currentItemIdx++;
    articleItems.set(currentItemIdx, new Pair<>(article, showArticleOptions));
    if ( currentIsNull )
    {
      articleItems.remove(currentItemIdx - 1);
    }
  }

  public void additional(ArticleItem article, @Nullable ShowArticleOptions showArticleOptions ) {
    articleItems.clear();
    articleItems.add(new Pair<>(article, showArticleOptions));
  }

  public void updateCurrentArticleItem(ArticleItem article) {
    articleItems.set( currentItemIdx, new Pair<>(article, getCurrentShowArticleOptions()) );
  }

  public void setNewArticleItems(@NonNull List<ArticleItem> newArticleItems,
                                    @Nullable ShowArticleOptions showArticleOptions) {
    setNewArticleItems(newArticleItems, showArticleOptions, 0);
  }

  public void setNewArticleItems(@NonNull List<ArticleItem> newArticleItems,
                                 @Nullable ShowArticleOptions showArticleOptions,
                                 int newItemIdx) {
    articleItems.clear();
    int articlesSize = newArticleItems.size();
    if (articlesSize > 0) {
        for (int i = 0; i < articlesSize; i++) {
            articleItems.add(new Pair<>(newArticleItems.get(i), showArticleOptions) );
        }
    } else {
        articleItems.add(null);
    }
    boolean isNewIndexAvailable = (newItemIdx >= 0) && (newItemIdx < articlesSize);
    currentItemIdx = isNewIndexAvailable ? newItemIdx : 0;
  }
}
