package com.paragon_software.engine.nativewrapper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.WordReference;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.article_manager.LinkInfo;

import java.util.ArrayList;
import java.util.List;

public final class ArticleItemFactory
{
  public static final WordVariant[] NORMAL_VARIANTS = { WordVariant.Show , WordVariant.PartOfSpeech, WordVariant.Numeration, WordVariant.LockFlag, WordVariant.SortKey};
  public static final WordVariant[] AUX_VARIANTS    = { WordVariant.Phrase, WordVariant.Label };

  private static final String LockFlag = "locked";

  @Nullable
  public static ArticleItem createNormal( @NonNull Dictionary.DictionaryId dictionaryId,
                                          @NonNull NativeDictionary dictionary, int listIndex, int localIndex,
                                          @NonNull int[] show, @NonNull String[] tmpBuffer, @Nullable String anchor )
  {
    int globalIndex = dictionary.getWordByLocalIndex(listIndex, localIndex, show, tmpBuffer);
    byte[] historyElement = dictionary.getHistoryElementByGlobalIndex(listIndex, globalIndex);
    int soundId = dictionary.getSoundIndexByWordGlobalIndex(listIndex, globalIndex);
    ListInfo listsInfo = dictionary.getListInfo(listIndex);
    Dictionary.Direction direction = findDirection(dictionaryId, listsInfo.languageFrom, listsInfo.languageTo);
    LinkInfo[] linkInfo = dictionary.getArticleLinks(listIndex, globalIndex);
    return new ArticleItem.Builder(dictionaryId, listIndex, globalIndex)
          .setLabel(tmpBuffer[0])
          .setPartOfSpeech(tmpBuffer.length > 1 ? tmpBuffer[1] : null)
          .setNumeration(tmpBuffer.length > 2 ? tmpBuffer[2] : null)
          .setFtsAnchor(anchor)
          .setLocked(tmpBuffer.length > 3 && LockFlag.equals(tmpBuffer[3]))
          .setDirection(direction)
          .setHistoryElement(historyElement)
          .setHasInternalSound(soundId != NativeDictionary.SLD_INDEX_SOUND_NO)
          .setSortKey(tmpBuffer.length > 4 ? tmpBuffer[4] : null)
          .setListType(getArticleListTypeFromEngineListType(listsInfo.listType))
          .setLinkInfo(linkInfo)
          .build();
  }

  @Nullable
  public static ArticleItem createNormalFromHistoryElement( @NonNull Dictionary.DictionaryId dictionaryId,
                                                            @NonNull NativeDictionary dictionary,
                                                            @NonNull byte[] historyElement )
  {

    int[] indexes = dictionary.getArticleIndexesByHistoryElement(historyElement);
    int listIndex = indexes[0];
    int globalIndex = indexes[1];
    return createNormal(dictionaryId, dictionary, listIndex, globalIndex);
  }

  @Nullable
  public static ArticleItem createNormal( @NonNull Dictionary.DictionaryId dictionaryId, @NonNull NativeDictionary dictionary, int listIndex, int globalIndex )
  {
    ArticleItem res = null;
    if ( globalIndex != -1 && listIndex != -1 )
    {
      ListInfo listInfo = dictionary.getListInfo(listIndex);
      int localIndex = dictionary.resetList(listIndex, globalIndex);
      if ( null != listInfo )
      {
        int[] variants = listInfo.getVariants(NORMAL_VARIANTS);
        String words[] = new String[variants.length];
        dictionary.getWordByLocalIndex(listIndex, localIndex, variants, words);
        if ( words[0] != null )
        {
          res = ArticleItemFactory.createNormal(dictionaryId, dictionary, listIndex, localIndex, variants, words, null);
        }
      }
    }
    return res;
  }

  @Nullable
  public static ArticleItem createFromSearch( @NonNull Dictionary.DictionaryId dictionaryId,
                                              @NonNull NativeDictionary dictionary, int listIndex, int localIndex,
                                              @NonNull int[] realIndicesBuffer,
                                              @NonNull SparseArray< int[] > variantsBuffer,
                                              @NonNull SparseArray< String[] > stringsBuffer,
                                              @NonNull SparseArray< int[] > auxAsRegularVariantsBuffer,
                                              @NonNull SparseArray< String[] > auxAsRegularStringsBuffer,
                                              @NonNull SparseArray< ListInfo > regularLists,
                                              @NonNull SparseArray< ListInfo > auxLists,
                                              @Nullable String searchQuery )
  {
    ArticleItem res = null;
    if ( dictionary.getRealWordLocation(listIndex, localIndex, realIndicesBuffer) )
    {
      int realListIndex = realIndicesBuffer[0];
      int realGlobalIndex = realIndicesBuffer[1];
      if ( auxLists.indexOfKey(realListIndex) >= 0 )
      {
        int[] variants;
        String[] strings;
        if ( ( variantsBuffer.indexOfKey(realListIndex) < 0 ) || ( stringsBuffer.indexOfKey(realListIndex) < 0 ) )
        {
          variants = auxLists.get(realListIndex).getVariants(AUX_VARIANTS);
          strings = new String[variants.length];
          variantsBuffer.put(realListIndex, variants);
          stringsBuffer.put(realListIndex, strings);
        }
        else
        {
          variants = variantsBuffer.get(realListIndex);
          strings = stringsBuffer.get(realListIndex);
        }
        int[] path = dictionary.getPathFromGlobalIndex(realListIndex, realGlobalIndex);
        if ( path.length > 0 )
        {
          if ( dictionary.resetList(realListIndex, path, path.length - 1) )
          {
            int auxLocalIndex = path[path.length - 1];
            dictionary.getWordByLocalIndex(realListIndex, auxLocalIndex, variants, strings);
            boolean pointsToRegularList =
                dictionary.getRealWordLocation(realListIndex, auxLocalIndex, realIndicesBuffer);
            if ( pointsToRegularList )
            {
              pointsToRegularList = ( regularLists.indexOfKey(realIndicesBuffer[0]) >= 0 );
            }
            if ( pointsToRegularList )
            {
              res = createNormal(dictionaryId, dictionary, realIndicesBuffer[0], realIndicesBuffer[1], variantsBuffer,
                                 stringsBuffer, regularLists, strings, searchQuery);
            }
            else
            {
              res = createNormal(dictionaryId, dictionary, realListIndex, realGlobalIndex, auxAsRegularVariantsBuffer,
                                 auxAsRegularStringsBuffer, auxLists, strings, searchQuery);
            }
          }
        }
      }
      else if ( regularLists.indexOfKey(realListIndex) >= 0 )
      {
        res = createNormal(dictionaryId, dictionary, realListIndex, realGlobalIndex, variantsBuffer, stringsBuffer,
                           regularLists, null, null);
      }
    }
    return res;
  }

  @Nullable
  public static ArticleItem createFromTranslate(@NonNull Dictionary.DictionaryId dictionaryId,
                                                @NonNull NativeDictionary dictionary, int listIndex, int localIndex,
                                                @NonNull int[] show, @NonNull String[] tmpBuffer,
                                                @Nullable String label, @Nullable String ftsHeadWord,
                                                @Nullable String ftsAnchor, @Nullable String searchQuery, List<WordReference> wordReferences)
  {
    int globalIndex = dictionary.getWordByLocalIndex(listIndex, localIndex, show, tmpBuffer);
    byte[] historyElement = dictionary.getHistoryElementByGlobalIndex(listIndex, globalIndex);
    int soundId = dictionary.getSoundIndexByWordGlobalIndex(listIndex, globalIndex);
    ListInfo listsInfo = dictionary.getListInfo(listIndex);
    Dictionary.Direction direction = findDirection(dictionaryId, listsInfo.languageFrom, listsInfo.languageTo);
    LinkInfo[] linkInfo = dictionary.getArticleLinks(listIndex, globalIndex);
    return new ArticleItem.Builder(dictionaryId, listIndex, globalIndex)
        .setLabel(label)
        .setPartOfSpeech(tmpBuffer.length > 1 ? tmpBuffer[1] : null)
        .setNumeration(tmpBuffer.length > 2 ? tmpBuffer[2] : null)
        .setFtsHeadword(ftsHeadWord)
        .setFtsAnchor(ftsAnchor)
        .setSearchQuery(searchQuery)
        .setWordReferences(wordReferences)
        .setLocked(tmpBuffer.length > 3 && LockFlag.equals(tmpBuffer[3]))
        .setDirection(direction)
        .setHistoryElement(historyElement)
        .setHasInternalSound(soundId != NativeDictionary.SLD_INDEX_SOUND_NO)
        .setSortKey(tmpBuffer.length > 4 ? tmpBuffer[4] : null)
        .setListType(getArticleListTypeFromEngineListType(listsInfo.listType))
        .setLinkInfo(linkInfo)
        .build();
  }

  @Nullable
  private static ArticleItem createNormal( @NonNull Dictionary.DictionaryId dictionaryId,
                                           @NonNull NativeDictionary dictionary, int listIndex, int globalIndex,
                                           @NonNull SparseArray< int[] > variantsBuffer,
                                           @NonNull SparseArray< String[] > stringsBuffer,
                                           @NonNull SparseArray< ListInfo > regularLists,
                                           @Nullable String[] auxStrings, @Nullable String searchQuery )
  {
    ArticleItem res = null;
    int[] variants;
    String[] strings;
    if ( ( variantsBuffer.indexOfKey(listIndex) < 0 ) || ( stringsBuffer.indexOfKey(listIndex) < 0 ) )
    {
      variants = regularLists.get(listIndex).getVariants(NORMAL_VARIANTS);
      strings = new String[variants.length];
      variantsBuffer.put(listIndex, variants);
      stringsBuffer.put(listIndex, strings);
    }
    else
    {
      variants = variantsBuffer.get(listIndex);
      strings = stringsBuffer.get(listIndex);
    }
    int[] path = dictionary.getPathFromGlobalIndex(listIndex, globalIndex);
    if ( path.length > 0 )
    {
      if ( dictionary.resetList(listIndex, path, path.length - 1) )
      {
        dictionary.getWordByLocalIndex(listIndex, path[path.length - 1], variants, strings);
        String show = strings[0];
        String partOfSpeech = strings.length > 1 ? strings[1] : null;
        String numeration = strings.length > 2 ? strings[2] : null;
        boolean lockFlag = strings.length > 3  && LockFlag.equals(strings[3]);
        if ( show != null )
        {
          String label = show;
          String ftsExtra = null;
          String ftsAnchor = null;
          List<WordReference> wordReferences = new ArrayList<>();
          ListInfo listsInfo = dictionary.getListInfo(listIndex);
          if ( auxStrings != null )
          {
            if ( auxStrings[0] != null )
            {
              label = auxStrings[0];
              ftsExtra = show;
              if ( searchQuery != null )
              {
                wordReferences = dictionary.getHighlightWordReferencesIncludingWhitespace(listIndex, label.toLowerCase(), searchQuery.toLowerCase());
              }
            }
            if ( auxStrings[1] != null )
            {
              ftsAnchor = auxStrings[1];
            }
          }
          byte[] historyElement = dictionary.getHistoryElementByGlobalIndex(listIndex, globalIndex);
          int soundId = dictionary.getSoundIndexByWordGlobalIndex(listIndex, globalIndex);
          Dictionary.Direction direction = findDirection(dictionaryId, listsInfo.languageFrom, listsInfo.languageTo);
          LinkInfo[] linkInfo = dictionary.getArticleLinks(listIndex, globalIndex);
          res = new ArticleItem.Builder(dictionaryId, listIndex, globalIndex).setFtsHeadword(ftsExtra)
              .setLabel(label).setSearchQuery(searchQuery).setPartOfSpeech(partOfSpeech).setNumeration(numeration).setLocked(lockFlag)
              .setFtsAnchor(ftsAnchor).setWordReferences(wordReferences).setDirection(direction)
              .setHistoryElement(historyElement).setHasInternalSound(soundId != NativeDictionary.SLD_INDEX_SOUND_NO)
              .setListType(getArticleListTypeFromEngineListType(listsInfo.listType))
              .setLinkInfo(linkInfo)
              .build();
        }
      }
    }
    return res;
  }

  @NonNull
  private static Dictionary.Direction findDirection( Dictionary.DictionaryId dictId, int from, int to )
  {
    DictionaryManagerAPI dictionaryManager = DictionaryManagerHolder.getManager();
    if ( dictionaryManager != null )
    {
      for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
      {
        if ( dictionary.getId().equals(dictId) )
        {
          for ( Dictionary.Direction direction : dictionary.getDirections() )
          {
            if ( from == direction.getLanguageFrom() && to == direction.getLanguageTo() )
            {
              return direction;
            }
          }
        }
      }
    }
    return new Dictionary.Direction(from, to, null);
  }

  private static com.paragon_software.article_manager.ListType getArticleListTypeFromEngineListType(ListType engineListType)
  {
    return com.paragon_software.article_manager.ListType.valueOf(engineListType.name());
  }
}
