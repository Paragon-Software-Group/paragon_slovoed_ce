package com.paragon_software.engine.views;

import androidx.annotation.NonNull;
import android.util.SparseArray;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.ArticleItemFactory;
import com.paragon_software.engine.nativewrapper.ListInfo;
import com.paragon_software.engine.nativewrapper.ListType;

public class RegularNativeCollectionView extends NativeCollectionView<Dictionary.Direction>
{
  private int[] variants;
  private String[] tmpBuffer = new String[ArticleItemFactory.NORMAL_VARIANTS.length];

  @Override
  void openHelper()
  {
    SparseArray< ListInfo > listInfo = dictionary.getLists(ListType.Group.Main);
    variants = null;
    ListInfo info = listInfo.get(listIndex);
    if ( info != null )
    {
      variants = info.getVariants(ArticleItemFactory.NORMAL_VARIANTS);
    }
  }

  @Override
  ArticleItem getItemHelper( int i )
  {
    ArticleItem articleItem = null;
    if ( variants != null )
    {
      articleItem = ArticleItemFactory
          .createNormal(dictionaryId, dictionary, listIndex, i, variants, tmpBuffer, null);
    }
    return articleItem;
  }

  public boolean startsWith( @NonNull String text )
  {
    ArticleItem curArticleItem = getItem(getPosition());
    if ( curArticleItem != null && isOpen())
    {
      return dictionary.startsWith(curArticleItem.getListId(), curArticleItem.getGlobalIndex(), text);
    }
    return false;
  }

}
