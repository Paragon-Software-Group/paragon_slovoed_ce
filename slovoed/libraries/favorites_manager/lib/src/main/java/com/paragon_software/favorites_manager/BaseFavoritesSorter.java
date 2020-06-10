package com.paragon_software.favorites_manager;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import androidx.annotation.NonNull;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.directory.Directory;

/**
 * Default implementation of {@link FavoritesSorterAPI} to sort list of articles according to specified type of sorting.
 * When sorting by date it considers that given articles already sorted by date descending (in base implementation of
 * {@link FavoritesManagerAPI} they stored in list sorted by date descending).
 */
public class BaseFavoritesSorter implements FavoritesSorterAPI
{

  /**
   * Default comparator to compare {@link ArticleItem} objects alphabetically.
   */
  private final Comparator< ArticleItem > defaultAlphabeticComparator = new Comparator< ArticleItem >()
  {
    @Override
    public int compare( ArticleItem articleItem1, ArticleItem articleItem2 )
    {
      if ((articleItem1.getDictId() == null && articleItem2.getDictId() == null)
          || articleItem1.getDictId().equals(articleItem2.getDictId()))
      {
        if ((articleItem1.getDirection() == null && articleItem2.getDirection() == null)
            || articleItem1.getDirection().equals(articleItem2.getDirection()))
        {
          if (articleItem1.getLabel() == null && articleItem2.getLabel() == null)
          {
            return 0;
          }
          else
          {
            return articleItem1.getLabel() == null ? -1 :
                   (articleItem2.getLabel() == null ? 1 :
                    articleItem1.getLabel().compareToIgnoreCase(articleItem2.getLabel()));

          }
        }
        else
        {
          return articleItem1.getDirection() == null ? -1 :
                 (articleItem2.getDirection() == null ? 1 :
                  (articleItem1.getDirection().getLanguageFrom() - articleItem2.getDirection().getLanguageFrom()));
        }
      }
      else
      {
        return articleItem1.getDictId() == null ? -1 :
               (articleItem2.getDictId() == null ? 1 :
                articleItem1.getDictId().toString().compareToIgnoreCase(articleItem2.getDictId().toString()));
      }
    }
  };

  /**
   * Default comparator to compare {@link Directory<ArticleItem>} objects alphabetically.
   */
  private final Comparator<Directory<ArticleItem>> defaultDirAlphabeticComparator = (dirItem1, dirItem2) -> dirItem1.getName().compareToIgnoreCase(dirItem2.getName());

  @Override
  public List< ArticleItem > sort( List< ArticleItem > articles, FavoritesSorting sorting )
  {
    List< ArticleItem > sortedArticles = articles.subList(0, articles.size());
    if (sorting == FavoritesSorting.BY_DATE_ASCENDING)
    {
      Collections.reverse(sortedArticles);
    }
    else if (sorting == FavoritesSorting.ALPHABETICALLY_ASCENDING)
    {
      Collections.sort(sortedArticles, defaultAlphabeticComparator);
    }
    else if (sorting == FavoritesSorting.ALPHABETICALLY_DESCENDING)
    {
      Collections.sort(sortedArticles, defaultAlphabeticComparator);
      Collections.reverse(sortedArticles);
    }
    return sortedArticles;
  }

  @NonNull
  @Override
  public List<Directory<ArticleItem>> sortDirectories(@NonNull List<Directory<ArticleItem>> directories, FavoritesSorting sorting) {
    List<Directory<ArticleItem>> sortedDirectories = directories.subList(0, directories.size());
    if (sorting == FavoritesSorting.BY_DATE_ASCENDING) {
      Collections.reverse(sortedDirectories);
    } else if (sorting == FavoritesSorting.ALPHABETICALLY_ASCENDING) {
      Collections.sort(sortedDirectories, defaultDirAlphabeticComparator);
    } else if (sorting == FavoritesSorting.ALPHABETICALLY_DESCENDING) {
      Collections.sort(sortedDirectories, defaultDirAlphabeticComparator);
      Collections.reverse(sortedDirectories);
    }
    return sortedDirectories;
  }
}
