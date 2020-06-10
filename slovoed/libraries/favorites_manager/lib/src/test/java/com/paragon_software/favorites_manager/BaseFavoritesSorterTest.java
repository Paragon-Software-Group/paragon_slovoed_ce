package com.paragon_software.favorites_manager;

import com.paragon_software.article_manager.ArticleItem;

import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static com.paragon_software.favorites_manager.mock.StaticConsts.DICT1_ID;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DICT2_ID;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DIRECT1;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DIRECT2;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DIRECT3;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DIRECT4;
import static org.junit.Assert.assertEquals;

public class BaseFavoritesSorterTest
{
  private static final ArticleItem ARTICLE1 = new ArticleItem.Builder(DICT1_ID, 1, 1)
          .setDirection(DIRECT1).setLabel("aaa").build();
  private static final ArticleItem ARTICLE2 = new ArticleItem.Builder(DICT1_ID, 1, 2)
          .setDirection(DIRECT1).setLabel("aab").build();
  private static final ArticleItem ARTICLE3 = new ArticleItem.Builder(DICT1_ID, 1, 3)
          .setDirection(DIRECT2).setLabel("aba").build();
  private static final ArticleItem ARTICLE4 = new ArticleItem.Builder(DICT1_ID, 1, 4)
          .setDirection(DIRECT2).setLabel("abb").build();
  private static final ArticleItem ARTICLE5 = new ArticleItem.Builder(DICT2_ID, 1, 5)
          .setDirection(DIRECT3).setLabel("baa").build();
  private static final ArticleItem ARTICLE6 = new ArticleItem.Builder(DICT2_ID, 1, 6)
          .setDirection(DIRECT3).setLabel("bab").build();
  private static final ArticleItem ARTICLE7 = new ArticleItem.Builder(DICT2_ID, 1, 7)
          .setDirection(DIRECT4).setLabel("bba").build();
  private static final ArticleItem ARTICLE8 = new ArticleItem.Builder(DICT2_ID, 1, 8)
          .setDirection(DIRECT4).setLabel("bbb").build();

  private final List< ArticleItem > favoritesArticles = new ArrayList(){{
    add(0, ARTICLE2);
    add(0, ARTICLE5);
    add(0, ARTICLE1);
    add(0, ARTICLE8);
    add(0, ARTICLE3);
    add(0, ARTICLE7);
    add(0, ARTICLE4);
    add(0, ARTICLE6);
  }};

  private BaseFavoritesSorter favoritesSorter;

  @Before
  public void setUp()
  {
    favoritesSorter = new BaseFavoritesSorter();
  }

  @Test
  public void testSortAlphabeticallyAscending()
  {
    List< ArticleItem > sortedArticles = favoritesSorter.sort(favoritesArticles, FavoritesSorting.ALPHABETICALLY_ASCENDING);
    assertEquals(sortedArticles.get(0), ARTICLE1);
    assertEquals(sortedArticles.get(1), ARTICLE2);
    assertEquals(sortedArticles.get(2), ARTICLE3);
    assertEquals(sortedArticles.get(3), ARTICLE4);
    assertEquals(sortedArticles.get(4), ARTICLE5);
    assertEquals(sortedArticles.get(5), ARTICLE6);
    assertEquals(sortedArticles.get(6), ARTICLE7);
    assertEquals(sortedArticles.get(7), ARTICLE8);
  }

  @Test
  public void testSortAlphabeticallyDescending()
  {
    List< ArticleItem > sortedArticles = favoritesSorter.sort(favoritesArticles, FavoritesSorting.ALPHABETICALLY_DESCENDING);
    assertEquals(sortedArticles.get(0), ARTICLE8);
    assertEquals(sortedArticles.get(1), ARTICLE7);
    assertEquals(sortedArticles.get(2), ARTICLE6);
    assertEquals(sortedArticles.get(3), ARTICLE5);
    assertEquals(sortedArticles.get(4), ARTICLE4);
    assertEquals(sortedArticles.get(5), ARTICLE3);
    assertEquals(sortedArticles.get(6), ARTICLE2);
    assertEquals(sortedArticles.get(7), ARTICLE1);
  }

  @Test
  public void testSortByDateAscending()
  {
    List< ArticleItem > sortedArticles = favoritesSorter.sort(favoritesArticles, FavoritesSorting.BY_DATE_ASCENDING);
    assertEquals(sortedArticles.get(0), ARTICLE2);
    assertEquals(sortedArticles.get(1), ARTICLE5);
    assertEquals(sortedArticles.get(2), ARTICLE1);
    assertEquals(sortedArticles.get(3), ARTICLE8);
    assertEquals(sortedArticles.get(4), ARTICLE3);
    assertEquals(sortedArticles.get(5), ARTICLE7);
    assertEquals(sortedArticles.get(6), ARTICLE4);
    assertEquals(sortedArticles.get(7), ARTICLE6);
  }

  @Test
  public void testSortByDateDescending()
  {
    List< ArticleItem > sortedArticles = favoritesSorter.sort(favoritesArticles, FavoritesSorting.BY_DATE_DESCENDING);
    assertEquals(sortedArticles.get(0), ARTICLE6);
    assertEquals(sortedArticles.get(1), ARTICLE4);
    assertEquals(sortedArticles.get(2), ARTICLE7);
    assertEquals(sortedArticles.get(3), ARTICLE3);
    assertEquals(sortedArticles.get(4), ARTICLE8);
    assertEquals(sortedArticles.get(5), ARTICLE1);
    assertEquals(sortedArticles.get(6), ARTICLE5);
    assertEquals(sortedArticles.get(7), ARTICLE2);
  }
}