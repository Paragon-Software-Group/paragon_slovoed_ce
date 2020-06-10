package com.paragon_software.engine.nativewrapper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.ArticleSearcherAPI;

import static com.paragon_software.engine.nativewrapper.ArticleItemFactory.NORMAL_VARIANTS;

public class ArticleSearcher implements ArticleSearcherAPI {

    @NonNull
    private final NativeDictionary dictionary;

    public ArticleSearcher(@NonNull NativeDictionary _dictionary)
    {
        dictionary = _dictionary;
    }


    @Override
    public ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, @NonNull String next, int directionFrom )
    {
        synchronized (dictionary) {
            List<Integer> lists = findLists(directionFrom, null, ListType.Group.Main);
            int wordIndex = -1;
            int listIndex = -1;
            if (lists.size() == 1)
            {
                listIndex = lists.get(0);
                wordIndex = dictionary.getWordByText(listIndex, next, true);
            }

            if (-1 == wordIndex)
            {
                wordIndex = getBaseFormIndex(listIndex,directionFrom,next);
            }

            if (-1 == wordIndex)
            {
                return null;
            }

            SparseArray<ListInfo> listInfo = dictionary.getLists(ListType.Group.Main);
            ListInfo info = listInfo.get(listIndex);
            int[] variants = info.getVariants(NORMAL_VARIANTS);
            String[] tmpBuffer = new String[NORMAL_VARIANTS.length];

            return ArticleItemFactory
                .createNormal(dictionaryId, dictionary, listIndex, wordIndex, variants, tmpBuffer, null);
        }
    }

    @Override
    public ArticleItem findLinkedWotDArticleItem(@NonNull Dictionary.DictionaryId dictionaryId, @NonNull String entryId, int directionFrom) {
        List<Integer> lists = findLists(directionFrom, ListType.WordOfTheDay, null);

        if(lists.size()==0) return null;;
        int listIndex = lists.get(0);
        int wordIndex = dictionary.getWordByText(listIndex, entryId, true);

        if (-1 == wordIndex)
        {
            return null;
        }

        int[] realIndices = new int[2];
        dictionary.getRealWordLocation(listIndex, wordIndex, realIndices);

        return find(dictionaryId, realIndices[0], realIndices[1], null);
    }


    @Override
    public Object[] getSoundInfoFromExternalKey(@NonNull String entryId, int directionFrom)
    {
        List<Integer> lists = findLists(directionFrom, ListType.WordOfTheDay, null);

        int listIndex = lists.get(0);
        int wordIndex = dictionary.getWordByText(listIndex, entryId, true);

        if(wordIndex != -1)
        {
            SparseArray<ListInfo> listInfo = dictionary.getLists(ListType.WordOfTheDay);
            ListInfo info = listInfo.get(listIndex);
            WordVariant[] var = { WordVariant.ExternalKey};
            int[] variants = info.getVariants(var);

            return dictionary.getCurrentWordStylizedVariant(variants[0]);
        }
        return new Object[0];
    }

    private int getBaseFormIndex(int listIndex, int direction, @NonNull String word)
    {
        int baseFormIndex = -1;
        String[] baseFormList = dictionary.getBaseForms(direction,word);
        if (null != baseFormList)
        {
            for (String baseForm : baseFormList)
            {
                baseFormIndex = dictionary.getWordByText(listIndex, baseForm, true);
                if (-1 != baseFormIndex)
                {
                    break;
                }
            }
        }
        return baseFormIndex;
    }

    @Override
    public ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, int listId, int globalIdx,
                             @Nullable String label )
    {
        ListInfo info = dictionary.getListInfo(listId);
        int[] variants = info.getVariants(NORMAL_VARIANTS);
        String[] tmpBuffer = new String[NORMAL_VARIANTS.length];

        int wordIndex = dictionary.resetList(listId, globalIdx);
        return ArticleItemFactory.createNormal(dictionaryId, dictionary, listId, wordIndex, variants, tmpBuffer, label);
    }

    @Override
    public ArticleItem findPractisePronunciationArticleItem(@NonNull Dictionary.DictionaryId dictionaryId,
                                                            @NonNull String next,
                                                            int directionFrom)
    {
        synchronized (dictionary) {
            List<Integer> lists = findLists(directionFrom, ListType.SpecialAdditionalInteractiveInfo, null);
            int wordIndex = -1;
            int listIndex = -1;
            if (lists.size() == 1)
            {
                listIndex = lists.get(0);
                wordIndex = dictionary.getWordByText(listIndex, next, true, WordVariant.SortKey);
            }

            if (-1 == wordIndex)
            {
                wordIndex = getBaseFormIndex(listIndex,directionFrom,next);
            }

            if (-1 == wordIndex)
            {
                return null;
            }

            SparseArray<ListInfo> listInfo = dictionary.getLists(ListType.SpecialAdditionalInteractiveInfo);
            ListInfo info = listInfo.get(listIndex);
            int[] variants = info.getVariants(NORMAL_VARIANTS);
            String[] tmpBuffer = new String[NORMAL_VARIANTS.length];

            return ArticleItemFactory
                    .createNormal(dictionaryId, dictionary, listIndex, wordIndex, variants, tmpBuffer, null);
        }
    }

    @Nullable
    @Override
    public ArticleItem findForSwipe( @NonNull ArticleItem currentArticleItem, boolean swipeForward )
    {
        ArticleItem articleItem = null;
        int listId = currentArticleItem.getListId();
        int resultGlobalIndex = dictionary.swipe(listId, currentArticleItem.getGlobalIndex(), swipeForward ? 1 : -1,
                                                 NativeDictionary.SwipingMode.ePassingAnySwipe);
        if ( resultGlobalIndex >= 0 )
        {
            articleItem =
                ArticleItemFactory.createNormal(currentArticleItem.getDictId(), dictionary, listId, resultGlobalIndex);
        }
        return articleItem;
    }

    @Override
    public int[] getQueryHighlightData( int listIndex, @NonNull String articleText, @NonNull String headword, @NonNull String phrase, @NonNull String query ) {
        return dictionary.getQueryHighLightData(listIndex, articleText, headword, phrase, query  );
    }

    @NonNull
    private List< Integer > findLists(int directionFrom, @Nullable ListType listType,
                                      @Nullable ListType.Group listGroup)
    {
        List< Integer > res = Collections.emptyList();
        if ( ( ( listType != null ) || ( listGroup != null ) ) )
        {
            SparseArray< ListInfo > listInfo;
            if ( listType != null )
            {
                listInfo = dictionary.getLists(listType);
            }
            else
            {
                listInfo = dictionary.getLists(listGroup);
            }
            int i, n = listInfo.size();
            res = new ArrayList<>(n);
            for ( i = 0; i < n ; i++ )
            {
                ListInfo info = listInfo.valueAt(i);
                if ( ( info.languageFrom == directionFrom ))
                {
                    res.add(listInfo.keyAt(i));
                }
            }
        }
        return res;
    }
}
