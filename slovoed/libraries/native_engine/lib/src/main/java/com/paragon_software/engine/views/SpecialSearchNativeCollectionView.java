package com.paragon_software.engine.views;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.engine.nativewrapper.ArticleItemFactory;
import com.paragon_software.engine.nativewrapper.ListInfo;
import com.paragon_software.engine.nativewrapper.ListType;
import com.paragon_software.engine.nativewrapper.WordVariant;
import com.paragon_software.native_engine.GroupHeader;

public class SpecialSearchNativeCollectionView extends NativeCollectionView<GroupHeader >
{
  @Nullable
  private String searchQuery;
  private final int[] realIndices = new int[2];
  private SparseArray< int[] > variantsBuffer, auxAsRegularVariantsBuffer;
  private SparseArray< String[] > stringsBuffer, auxAsRegularStringsBuffer;
  private SparseArray< ListInfo > regularLists, auxLists;

  public SpecialSearchNativeCollectionView(@Nullable String searchQuery) {
    this.searchQuery = searchQuery;
  }

  @Override
  void openHelper()
  {
    variantsBuffer = new SparseArray<>();
    auxAsRegularVariantsBuffer = new SparseArray<>();
    stringsBuffer = new SparseArray<>();
    auxAsRegularStringsBuffer = new SparseArray<>();
    regularLists = dictionary.getLists(ListType.Group.Main);
    auxLists = dictionary.getLists(ListType.FullTextAuxiliary);
    ListInfo info = dictionary.getListInfo(listIndex);
    if ( info != null )
    {
      initEx(info);
    }
  }

  @Override
  ArticleItem getItemHelper( int i )
  {
    return ArticleItemFactory
        .createFromSearch(dictionaryId, dictionary, listIndex, i, realIndices, variantsBuffer, stringsBuffer,
                          auxAsRegularVariantsBuffer, auxAsRegularStringsBuffer, regularLists, auxLists, searchQuery);
  }

  private void initEx( @NonNull ListInfo listInfo )
  {
    int[] phrase = listInfo.getVariants(new WordVariant[]{ WordVariant.Phrase });
    boolean isHeadwordVisible = ( phrase[0] >= 0 );
    updateMetadata(new GroupHeader(dictionaryId, convertListType(listInfo.listType), listInfo.listTypeOffset,
                                listInfo.getString(ListInfo.Name.Full), listInfo.languageFrom, listInfo.languageTo, isHeadwordVisible));

  }

  private static GroupHeader.LIST_TYPE convertListType(@NonNull ListType listType )
  {
    GroupHeader.LIST_TYPE res = null;
    switch ( listType )
    {
      case FullTextSearchBase:
        res = GroupHeader.LIST_TYPE.BASE;
        break;

      case FullTextSearchHeadword:
        res = GroupHeader.LIST_TYPE.HEADWORD;
        break;

      case FullTextSearchContent:
        res = GroupHeader.LIST_TYPE.CONTENT;
        break;

      case FullTextSearchTranslation:
        res = GroupHeader.LIST_TYPE.TRANSLATION;
        break;

      case FullTextSearchExample:
        res = GroupHeader.LIST_TYPE.EXAMPLE;
        break;

      case FullTextSearchDefinition:
        res = GroupHeader.LIST_TYPE.DEFINITION;
        break;

      case FullTextSearchPhrase:
        res = GroupHeader.LIST_TYPE.PHRASE;
        break;

      case FullTextSearchIdiom:
        res = GroupHeader.LIST_TYPE.IDIOM;
        break;

      case FullTextSearchLast:
        res = GroupHeader.LIST_TYPE.LAST;
        break;
    }
    return res;
  }
}
