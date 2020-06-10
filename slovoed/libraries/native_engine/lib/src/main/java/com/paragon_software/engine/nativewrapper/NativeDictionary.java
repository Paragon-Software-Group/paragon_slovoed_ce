package com.paragon_software.engine.nativewrapper;

import android.content.Context;
import androidx.annotation.IntDef;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;

import com.paragon_software.article_manager.WordReference;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.extbase.MorphoInfo;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.ExternalBasesHolder;
import com.paragon_software.engine.destructionqueue.ResourceHolder;
import com.paragon_software.engine.rx.preloadedwords.PreloadedWordsNativeCallback;
import com.paragon_software.native_engine.HtmlBuilderParams;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.article_manager.LinkInfo;
import com.paragon_software.native_engine.data.SoundData;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.List;

public class NativeDictionary
{
  public static final int SLD_INDEX_SOUND_NO = -1;

  /**
   * see. enum ESwipingMode from 'SldDefines.h'
   */
  public enum SwipingMode
  {
    /// Свайп на любой элемент в пределах данного уровня вложенности
    ePassingAnySwipe,
    /// Свайп на элемент статейного типа (пропуская элементы нестатейного типа) в пределах данного уровня вложенности
    eSkipingCatalogSwipe,
    /// Сквозной свайп на элемент статейного типа (пропуская элементы нестатейного типа)
    eAcrossingCatalogSwipe,
    /// Свайп на элемент статейного типа (блокируясь на элементах нестатейного типа) в пределах данного уровня вложенности
    eInterruptingOnCatalogSwipe
  }

  /**
   * Constants for word reference type {@link NativeDictionary#getWordReferenceInList(int, String)} <br>
   * See: TWordRefInfo (SldTypes.h)
   */
  @IntDef( { TWordRefInfo.TYPE_EXACT, TWordRefInfo.TYPE_MORPHO_BASE, TWordRefInfo.TYPE_MORPHO_FORM } )
  @Retention( RetentionPolicy.SOURCE )
  public @interface TWordRefInfo
  {
    int TYPE_EXACT       = 0;
    int TYPE_MORPHO_BASE = 1;
    int TYPE_MORPHO_FORM = 2;
  }

  @NonNull
  private final DictionaryLocation      mLocation;
  @NonNull
  private final NativeFunctions         mFunctions;
  @NonNull
  private final DictionaryInfo          mDictionaryInfo;
  @NonNull
  private final ListInfo[]              mMainListInfo;
  @NonNull
  private final SparseArray< ListInfo > mSearchResultLists;
  @NonNull
  private final Context                 mAppContext;

  private volatile ResourceHolder< NativePayload > mPayloadHolder;

  @Nullable
  public static NativeDictionary open(Context context, @Nullable DictionaryLocation location, @Nullable List<MorphoInfo> morphoInfoList, boolean openMorpho)
  {
    NativeDictionary res = null;
    NativePayload payload = NativePayload.open(context, location,morphoInfoList ,openMorpho);
    if ( payload != null && location != null)
    {
      res = new NativeDictionary(context, location, payload);
      res.mPayloadHolder = ResourceHolder.create(res, payload);
    }
    return res;
  }

  @Nullable
  public static NativeDictionary open(Context context, @Nullable Dictionary dictionary, boolean openMorpho, boolean openSoundBases )
  {
    NativeDictionary res = null;
    if ( dictionary != null )
    {
      res = open(context, dictionary.getDictionaryLocation(), dictionary.getMorphoInfoList(), openMorpho);
      if ( res != null && openSoundBases )
      {
        ExternalBasesHolder.openExternalBases(context, dictionary);
      }
    }
    return res;
  }

  private NativeDictionary( Context context, @NonNull DictionaryLocation location, @NonNull NativePayload payload )
  {
    mLocation = location;
    mFunctions = payload.getFunctions();
    mDictionaryInfo = new DictionaryInfo(context, mFunctions);
    mAppContext = context.getApplicationContext();

    mMainListInfo = new ListInfo[mDictionaryInfo.mainListCount];
    int ftsListCount = 0;
    for ( int i = 0 ; i < mDictionaryInfo.mainListCount ; i++ )
    {
      mMainListInfo[i] = new ListInfo(context, mFunctions, i, i);
      if ( mMainListInfo[i].listType.belongsToGroup(ListType.Group.Fts) )
      {
        ftsListCount++;
      }
    }
    mSearchResultLists = new SparseArray<>(ftsListCount);
  }

  @NonNull
  public DictionaryLocation getLocation()
  {
    return mLocation;
  }

  /**
   * @return array with data structure:<br>
   * <br>
   * [ [A_0] [B_0] [C_0] [D_0]   [A_1] [B_1] [C_1] [D_1]  ... [A_N] [B_N] [C_N] [D_N] ]<br>
   * <br>
   * A - SubstringStart<br>
   * B - SubstringEnd<br>
   * C - Type {@link NativeDictionary.TWordRefInfo}<br>
   * D - WordGlobalIndex<br>
   */
  @Nullable
  public int[] getWordReferenceInList( int listIndex, @NonNull String query )
  {
    return (int[]) mFunctions.call(NativeFunctions.PUBLIC.GET_WORD_REFERENCE_IN_LIST, listIndex, query);
  }
  
  @Nullable
  public LinkInfo[] getArticleLinks(int listIndex, int wordIndex )
  {
    return (LinkInfo[]) mFunctions.call(NativeFunctions.PUBLIC.GET_ARTICLE_LINKS, listIndex, wordIndex);
  }
  
  @NonNull
  public int[] getQueryReferenceInPhrase( int listIndex, @NonNull String phrase, @NonNull String query )
  {
    return (int[]) mFunctions.call(NativeFunctions.PUBLIC.GET_QUERY_REFERENCE_IN_PHRASE, listIndex, phrase, query);
  }

  @NonNull
  public DictionaryInfo getDictionaryInfo()
  {
    return mDictionaryInfo;
  }

  public int getWordByLocalIndex( int listIndex, int wordIndex, int[] variants, String[] word )
  {
    return (Integer) mFunctions
        .call(NativeFunctions.PUBLIC.GET_WORD_BY_LOCAL_INDEX, listIndex, wordIndex, variants, word);
  }

  public boolean getRealWordLocation( int listIndex, int localIndex, int[] realIndices )
  {
    return (Boolean) mFunctions.call(NativeFunctions.PUBLIC.GET_REAL_WORD_LOCATION, listIndex, localIndex, realIndices);
  }

  @NonNull
  public int[] getPathFromGlobalIndex( int listIndex, int globalIndex )
  {
    return (int[]) mFunctions.call(NativeFunctions.PUBLIC.GET_PATH_FROM_GLOBAL_INDEX, listIndex, globalIndex);
  }

  public void getPreloadedFavorites( int listIndex, PreloadedWordsNativeCallback callback )
  {
    mFunctions.call(NativeFunctions.PUBLIC.GET_PRELOADED_FAVORITES, listIndex, callback);
  }

  @NonNull
  public  Object[] getCurrentWordStylizedVariant( int variantIndex )
  {
    return (Object[]) mFunctions.call(NativeFunctions.PUBLIC.GET_CURRENT_WORD_STYLIZED_VARIANT, variantIndex);
  }

  /**
   * @param listIndex     index of list to reset
   * @param globalIndex   global index of word inside desired hierarchy location
   * @return              local index of this word or negative value in case of error
   */
  public int resetList(int listIndex, int globalIndex )
  {
    int[] path = getPathFromGlobalIndex(listIndex, globalIndex);
    if ( path.length > 0 )
    {
      if ( resetList(listIndex, path, path.length - 1) )
      {
        return path[path.length - 1];
      }
    }
    return -1;
  }

  public int switchDirection(int listIndex, String word)
  {
      return (int) mFunctions.call(NativeFunctions.PUBLIC.SWITCH_DIRECTION,listIndex,word);
  }

  @NonNull
  List<WordReference> getWordReferences(int listIndex, String phrase) {
    int[] ref = (int[]) mFunctions.call(NativeFunctions.PUBLIC.GET_WORD_REFERENCE, listIndex, phrase);
    List<WordReference> wordReferences = new ArrayList<>();
      for (int i = 0; i < ref.length; i += 2) {
        wordReferences.add(new WordReference(ref[i], ref[i + 1]));
      }
    return wordReferences;
  }

  @NonNull
  List<WordReference> getHighlightWordReferencesIncludingWhitespace(int listIndex, String phrase, String query) {
    int[] ref = (int[]) mFunctions.call(NativeFunctions.PUBLIC.GET_HIGHLIGHT_WORD_REFERENCE_INCLUDING_WHITESPACE, listIndex, phrase, query);
    List<WordReference> wordReferences = new ArrayList<>();
    for (int i = 0; i < ref.length; i += 2) {
      wordReferences.add(new WordReference(ref[i], ref[i + 1]));
    }
    return wordReferences;
  }


  public byte[] getHistoryElementByGlobalIndex( int listIndex, int globalIndex )
  {
    return (byte[]) mFunctions
        .call(NativeFunctions.PUBLIC.GET_HISTORY_ELEMENT_BY_GLOBAL_INDEX, listIndex, globalIndex);
  }

  /**
   * Get article list index and global index by history element.
   * @param historyElement
   * @return if article is found then return array with article indexes:
   * int[0] - list index,
   * int[1] - global index,
   * otherwise
   * int[0] = int[1] = -1
   */
  public int[] getArticleIndexesByHistoryElement( byte[] historyElement )
  {
    return (int[]) mFunctions
        .call(NativeFunctions.PUBLIC.GET_ARTICLE_INDEXES_BY_HISTORY_ELEMENT, historyElement);
  }

  /**
   * Try to get sound index for word with specified list index and global index.
   * @param listIndex list index of word
   * @param globalIndex global index of word
   * @return sound index if there is a sound data for specified word, otherwise return -1 (SLD_INDEX_SOUND_NO)
   */
  public int getSoundIndexByWordGlobalIndex( int listIndex, int globalIndex )
  {
    return (int) mFunctions.call(NativeFunctions.PUBLIC.GET_SOUND_INDEX_BY_WORD_GLOBAL_INDEX, listIndex, globalIndex);
  }

  /**
   * Try to get sound index by searching for word label text.
   * @param text word label text to searching for
   * @return sound index if there is a sound data for word found by specified text,
   * otherwise return -1 (SLD_INDEX_SOUND_NO)
   */
  public int getSoundIndexByText( String text )
  {
    return (int) mFunctions.call(NativeFunctions.PUBLIC.GET_SOUND_INDEX_BY_TEXT, text);
  }

  /**
   * Инициирует процедуру декодирования и проигрывания озвучки с заданным индексом
   * и с параметрами по умолчанию.
   * @param soundIndex индекс озвучки
   * @return данные для озвучки
   */
  public SoundData getSoundDataByIndex( int soundIndex )
  {
    return (SoundData) mFunctions.call(NativeFunctions.PUBLIC.PLAY_SOUND_BY_INDEX, soundIndex);
  }

  public SoundData convertSpx(byte[] bytes) {
    return (SoundData) mFunctions.call(NativeFunctions.PUBLIC.CONVERT_SPX, bytes);
  }


  @Nullable
  public byte[] getExternalImage(int listId, String imgKey ) {
    return (byte[])  mFunctions.call(NativeFunctions.PUBLIC.GET_EXTERNAL_IMAGE, listId, imgKey);
  }


  public boolean resetList( int listIndex, int[] hierarchyPath, int pathLength )
  {
    return (Boolean) mFunctions.call(NativeFunctions.PUBLIC.RESET_LIST, listIndex, hierarchyPath, pathLength);
  }

  @Nullable
  public ListInfo getListInfo( final int listIndex )
  {
    SparseArray< ListInfo > listInfo = getLists(new ListFilter()
    {
      @Override
      public boolean apply( int n, @NonNull ListType listType )
      {
        return listIndex == n;
      }
    });
    ListInfo res = null;
    if ( listInfo.size() == 1 )
    {
      res = listInfo.valueAt(0);
    }
    return res;
  }

  @NonNull
  public SparseArray< ListInfo > getLists( @NonNull final ListType neededListType )
  {
    return getLists(new ListFilter()
    {
      @Override
      public boolean apply( int n, @NonNull ListType listType )
      {
        return neededListType.equals(listType);
      }
    });
  }

  @NonNull
  public SparseArray< ListInfo > getLists( @NonNull final ListType.Group listTypeGroup )
  {
    return getLists(new ListFilter()
    {
      @Override
      public boolean apply( int n, @NonNull ListType listType )
      {
        return listType.belongsToGroup(listTypeGroup);
      }
    });
  }

  public int getListCurrentSize( int listIndex )
  {
    return (Integer) mFunctions.call(NativeFunctions.PUBLIC.GET_WORD_COUNT, listIndex);
  }

  public int getWordByText( int listIndex, String word, boolean exact )
  {
    return getWordByText(listIndex, word, exact, WordVariant.Show);
  }

  public int getWordByText( int listIndex, String word, boolean exact, WordVariant listVariant )
  {
    return (Integer) mFunctions.call(NativeFunctions.PUBLIC.GET_WORD_BY_TEXT, listIndex, word, exact, listVariant.ordinal());
  }

  public String[] getBaseForms(int direction, String word)
  {
    return (String[]) mFunctions.call(NativeFunctions.PUBLIC.GET_BASE_FORMS, direction, word);
  }

  public void fullTextSearch( int listIndex, String word, int maximumWords, SortType sortType )
  {
    int newListIndex = (Integer) mFunctions
        .call(NativeFunctions.PUBLIC.FULL_TEXT_SEARCH, listIndex, word, maximumWords, sortType.ordinal() - 1);
    if ( newListIndex >= mMainListInfo.length )
    {
      mSearchResultLists.append(newListIndex, new ListInfo(mAppContext, mFunctions, newListIndex, listIndex));
    }
  }

  public void  didYouMeanSearch(int listIndex, String word, int maximumWords) {
    int newListIndex = (Integer) mFunctions.call(NativeFunctions.PUBLIC.DID_YOU_MEAN_SEARCH, listIndex, word, maximumWords);
    if ( newListIndex >= mMainListInfo.length )
    {
      mSearchResultLists.append(newListIndex, new ListInfo(mAppContext, mFunctions, newListIndex, listIndex));
    }
  }

  public void anagramSearch(int listIndex, String word, int maximumWords)  {
    int newListIndex = (Integer) mFunctions
            .call(NativeFunctions.PUBLIC.ANAGRAM_SEARCH, listIndex, word, maximumWords);

    if ( newListIndex >= mMainListInfo.length )
    {
      mSearchResultLists.append(newListIndex, new ListInfo(mAppContext, mFunctions, newListIndex, listIndex));
    }
  }

  public void fuzzySearch(int listIndex, String word, int maximumWords)  {
    int newListIndex = (Integer) mFunctions
            .call(NativeFunctions.PUBLIC.FUZZY_SEARCH, listIndex, word, maximumWords);

    if ( newListIndex >= mMainListInfo.length )
    {
      mSearchResultLists.append(newListIndex, new ListInfo(mAppContext, mFunctions, newListIndex, listIndex));
    }
  }

  public void wildCardSearch( int listIndex, String word, int maximumWords)
  {
    int newListIndex = (Integer) mFunctions
            .call(NativeFunctions.PUBLIC.WILD_CARD_SEARCH, listIndex, word, maximumWords);

    if ( newListIndex >= mMainListInfo.length )
    {
      mSearchResultLists.append(newListIndex, new ListInfo(mAppContext, mFunctions, newListIndex, listIndex));
    }
  }

  public boolean deleteAllSearchResultLists()
  {
    boolean res = (Boolean) mFunctions.call(NativeFunctions.PUBLIC.DELETE_ALL_SEARCH_LISTS);
    if ( res )
    {
      mSearchResultLists.clear();
    }
    return res;
  }

  public boolean startsWith( int listIndex, int globalIndex, @NonNull String word )
  {
    return (Boolean) mFunctions.call(NativeFunctions.PUBLIC.STARTS_WITH, listIndex, globalIndex, word);
  }

  public String translate( int listIndex, int localIndex, @NonNull HtmlBuilderParams htmlParams )
  {
    return (String) mFunctions.call(NativeFunctions.PUBLIC.TRANSLATE, listIndex, localIndex, htmlParams);
  }

  public int swipe( int listIndex, int globalIndex, int swipingDirection, SwipingMode swipingMode )
  {
    return (Integer) mFunctions
        .call(NativeFunctions.PUBLIC.SWIPE, listIndex, globalIndex, swipingDirection, swipingMode.ordinal());
  }

  @NonNull
  public int[] getQueryHighLightData( int listIndex, @NonNull String articleText, @NonNull String headword,  @NonNull String phrase, @NonNull String query  )
  {
    return (int[]) mFunctions.call(NativeFunctions.PUBLIC.GET_QUERY_HIGHLIGHT_DATA,listIndex,  articleText, headword,  phrase, query);
  }

  public int close()
  {
    mPayloadHolder.free();
    return mPayloadHolder.getResource().getCloseResult();
  }

  @NonNull
  private SparseArray< ListInfo > getLists( @NonNull ListFilter filter )
  {
    SparseArray< ListInfo > res = new SparseArray<>();
    for ( int i = 0 ; i < mMainListInfo.length ; i++ )
    {
      if ( filter.apply(i, mMainListInfo[i].listType) )
      {
        res.append(i, mMainListInfo[i]);
      }
    }
    for ( int i = 0 ; i < mSearchResultLists.size() ; i++ )
    {
      int listIndex = mSearchResultLists.keyAt(i);
      ListInfo listInfo = mSearchResultLists.valueAt(i);
      if ( filter.apply(listIndex, listInfo.listType) )
      {
        res.append(listIndex, listInfo);
      }
    }
    return res;
  }

  private interface ListFilter
  {
    boolean apply( int n, @NonNull ListType listType );
  }
}
