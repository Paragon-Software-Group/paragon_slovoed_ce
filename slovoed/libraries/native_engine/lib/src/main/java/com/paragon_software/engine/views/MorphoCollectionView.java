package com.paragon_software.engine.views;

import android.util.SparseArray;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.ArticleItemFactory;
import com.paragon_software.engine.nativewrapper.ListInfo;
import com.paragon_software.engine.nativewrapper.ListType;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.utils_slovoed.collections.BasicCollectionView;

import java.util.ArrayList;
import java.util.List;

public class MorphoCollectionView extends BasicCollectionView<ArticleItem, Void>
{
    private List<Integer> baseFormIndexList = new ArrayList<>();
    private int[] variants;
    private String[] tmpBuffer = new String[ArticleItemFactory.NORMAL_VARIANTS.length];
    private NativeDictionary dictionary = null;
    private Dictionary.DictionaryId dictionaryId;
    private int listIndex;

    public void setData(Dictionary.DictionaryId _dictionaryId, NativeDictionary _dictionary, int _listIndex, List<Integer> _baseFormIndexList)
    {
        dictionaryId = _dictionaryId;
        dictionary = _dictionary;
        listIndex = _listIndex;
        SparseArray< ListInfo > listInfo = dictionary.getLists(ListType.Group.Main);
        variants = null;
        baseFormIndexList = _baseFormIndexList;
        ListInfo info = listInfo.get(listIndex);
        if ( info != null )
        {
            variants = info.getVariants(ArticleItemFactory.NORMAL_VARIANTS);
        }
        baseFormIndexList = _baseFormIndexList;
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, baseFormIndexList.size());
    }

    public void clear()
    {
        baseFormIndexList.clear();
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, baseFormIndexList.size());
    }

    @Override
    public int getCount()
    {
        return baseFormIndexList.size();
    }

    @Override
    public ArticleItem getItem(int position)
    {
        ArticleItem articleItem = null;
        if ( variants != null )
        {
            articleItem =
                    ArticleItemFactory.createNormal(dictionaryId, dictionary, listIndex, baseFormIndexList.get(position), variants, tmpBuffer, null);
        }
        return articleItem;
    }
}
