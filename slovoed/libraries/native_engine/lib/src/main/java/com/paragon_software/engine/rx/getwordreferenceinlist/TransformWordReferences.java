package com.paragon_software.engine.rx.getwordreferenceinlist;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.ArticleItemFactory;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.utils_slovoed.collections.BasicCollectionView;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.Substring;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class TransformWordReferences
{
  private static final int SUB_ARRAY_LENGTH = 4; // See javadoc for NativeDictionary.getWordReferenceInList
  @NonNull
  private final Dictionary.DictionaryId dictionaryId;
  @NonNull
  private final NativeDictionary        nativeDictionary;
  @Nullable
  private final int[]                   wordRefs;
  private final int                     listIndex;

  TransformWordReferences( @NonNull Dictionary.DictionaryId dictionaryId, @NonNull NativeDictionary nativeDictionary,
                           @Nullable int[] wordRefs, int listIndex )
  {
    this.dictionaryId = dictionaryId;
    this.nativeDictionary = nativeDictionary;
    this.wordRefs = wordRefs;
    this.listIndex = listIndex;
  }

  public List< CollectionView< ArticleItem, SubstringInfo > > get()
  {
    return get(NativeDictionary.TWordRefInfo.TYPE_MORPHO_FORM);
  }

  /**
   * @param maxAcceptedType {@link NativeDictionary.TWordRefInfo}
   */
  public List< CollectionView< ArticleItem, SubstringInfo > > get(
      @NativeDictionary.TWordRefInfo int maxAcceptedType )
  {
    Map< Substring, List< ArticleItem > > map = createAndGroupArticles(maxAcceptedType);
    List< CollectionView< ArticleItem, SubstringInfo > > collectionView = new ArrayList<>(map.size());

    for ( Substring substring : map.keySet() )
    {
      List< ArticleItem > items = map.get(substring);
      if ( null != items && !items.isEmpty() )
      {
        SubstringToArticlesCollection substringToArticlesCollection = new SubstringToArticlesCollection(items);
        substringToArticlesCollection.updateMetadata(substring);
        collectionView.add(substringToArticlesCollection);
      }
    }
    return collectionView;
  }

  @NonNull
  private Map< Substring, List< ArticleItem > > createAndGroupArticles( int maxAcceptedType )
  {
    Map< Substring, List< ArticleItem > > map = new LinkedHashMap<>();

    if ( null != wordRefs && isDataValidLength(wordRefs) )
    {
      for ( int i = 0 ; i < wordRefs.length ; i += SUB_ARRAY_LENGTH )
      {
        int start = wordRefs[i];
        int end = wordRefs[i + 1];
        int type = wordRefs[i + 2];
        int globalIndex = wordRefs[i + 3];
        Substring substring = new Substring(start, end - start + 1);
        if ( type >= NativeDictionary.TWordRefInfo.TYPE_EXACT && type <= maxAcceptedType )
        {
          List< ArticleItem > articleItems = map.get(substring);
          if ( null == articleItems )
          {
            map.put(substring, articleItems = new ArrayList<>());
          }
          ArticleItem articleItem = getArticleItem(listIndex, globalIndex);
          articleItems.add(articleItem);
        }
      }
    }
    return map;
  }

  @Nullable
  private ArticleItem getArticleItem( int listIndex, int globalIndex )
  {
    return ArticleItemFactory.createNormal(dictionaryId, nativeDictionary, listIndex, globalIndex);
  }

  private boolean isDataValidLength( @NonNull int[] wordRefs )
  {
    return wordRefs.length > 0 && 0 == wordRefs.length % SUB_ARRAY_LENGTH;
  }

  static class SubstringToArticlesCollection extends BasicCollectionView< ArticleItem, SubstringInfo >
  {
    private List< ArticleItem > articles;

    SubstringToArticlesCollection( List< ArticleItem > articles )
    {
      this.articles = articles;
    }

    @Override
    public int getCount()
    {
      return articles.size();
    }

    @Override
    public ArticleItem getItem( int position )
    {
      return articles.get(position);
    }
  }
}
