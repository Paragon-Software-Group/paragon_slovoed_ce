package com.paragon_software.favorites_manager;

import androidx.annotation.NonNull;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.directory.Directory;

/**
 * Interface for sorting list of favorites articles.
 */
public interface FavoritesSorterAPI
{
  /**
   * Sort list of articles according to specified type of sorting.
   * @param articles List of words to sort
   * @param sorting Type of sorting
   */
  @NonNull List< ArticleItem > sort( @NonNull List< ArticleItem > articles, FavoritesSorting sorting );

  /**
   * Sort list of directories according to specified type of sorting.
   * @param directories List of directories to sort
   * @param sorting Type of sorting
   */
  @NonNull List<Directory<ArticleItem>> sortDirectories(@NonNull List<Directory<ArticleItem>> directories, FavoritesSorting sorting );
}
