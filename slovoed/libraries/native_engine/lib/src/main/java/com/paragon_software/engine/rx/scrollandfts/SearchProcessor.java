package com.paragon_software.engine.rx.scrollandfts;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.Pair;
import android.util.SparseArray;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.ExternalBasesHolder;
import com.paragon_software.engine.nativewrapper.ListInfo;
import com.paragon_software.engine.nativewrapper.ListType;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.scrollandfts.params.AbstractParams;
import com.paragon_software.engine.rx.scrollandfts.params.ScrollParams;
import com.paragon_software.engine.rx.scrollandfts.params.SpecialSearchParam;
import com.paragon_software.engine.rx.scrollandfts.result.AbstractResult;
import com.paragon_software.engine.rx.scrollandfts.result.SpecialSearchResult;
import com.paragon_software.engine.rx.scrollandfts.result.NoResult;
import com.paragon_software.engine.rx.scrollandfts.result.ScrollSearchResult;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_ANAGRAM;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_DID_YOU_MEAN;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_FTS;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_FUZZY;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_WILD_CARD;

class SearchProcessor
{
  @Nullable
  private NativeDictionary dictionary;

  private static final int MAX_WORDS_FTS = 1024;
  private static final int MAX_WORDS_DID_YOU_MEAN = 20;
  private static final int MAX_WORDS_ANAGRAM = 200;
  private static final int MAX_WORDS_FUZZY = 200;
  private static final int MAX_WORDS_WILD_CARD = 200;


  @NonNull
  AbstractResult process( Context applicationContext, @Nullable Dictionary _dictionary,
                          @NonNull AbstractParams params )
  {
    AbstractResult res = new NoResult();
    if ( params instanceof ScrollParams )
    {
      res = scroll(applicationContext, _dictionary, (ScrollParams) params);
    }
    else if ( params instanceof SpecialSearchParam )
    {
      res = specialSearch(applicationContext, _dictionary, (SpecialSearchParam) params);
    }
    return res;
  }

  @NonNull
  private ScrollSearchResult scroll(Context applicationContext, @Nullable Dictionary _dictionary, @NonNull ScrollParams params ) {
    ScrollSearchResult res = new ScrollSearchResult(null, null, -1, -1,null,null);
    List<Integer> baseFormIndexList = new ArrayList<>();
    updateDictionary(applicationContext, _dictionary,true);
    if (( dictionary != null ) && ( params.word != null )) {
      Pair<Integer, Dictionary.Direction> where = findList(dictionary, params.word, params.direction, params.availableDirections);
      int wordIndex = dictionary.getWordByText(where.first, params.word, true);
      if (-1 == wordIndex) {
        if (params.exactly) return res;
        wordIndex = dictionary.getWordByText(where.first, params.word, false);
        if (null != where.second)
          baseFormIndexList = getBaseForms(where.first, where.second.getLanguageFrom(), params.word);
      }
      res = new ScrollSearchResult(params.dictionaryId, dictionary, where.first, wordIndex, baseFormIndexList, where.second);
    }
    return res;
  }

  @NonNull
  private List<Integer> getBaseForms(int listIndex, int direction, @NonNull String word)
  {
      Set<Integer> baseFormIndexList = new HashSet<>();
      String[] baseFormList = dictionary.getBaseForms(direction,word);
      if (null != baseFormList)
      {
        for (String baseForm : baseFormList)
        {
          int baseFormIndex = dictionary.getWordByText(listIndex, baseForm, true);
          if (-1 != baseFormIndex)
          {
            baseFormIndexList.add(baseFormIndex);
          }
        }
      }
    return new ArrayList<>(baseFormIndexList);
  }


  @NonNull
  private SpecialSearchResult specialSearch(Context applicationContext, @Nullable Dictionary _dictionary, @NonNull SpecialSearchParam params )
  {
    SpecialSearchResult res = new SpecialSearchResult(null, null, new int[0], null, null);
    updateDictionary(applicationContext, _dictionary, true);

    if((dictionary == null) || (params.word == null))
      return res;

    List<Integer> lists;
    Dictionary.Direction resultDirection;
    if (SEARCH_TYPE_FTS.equals(params.searchType)) {
      lists = findLists(params.direction, null, ListType.Group.Fts, false);
      resultDirection = params.direction;
    }
    else {
      Pair<Integer, Dictionary.Direction> where = findList(dictionary, params.word, params.direction, params.availableDirections);
      lists = Collections.singletonList(where.first);
      resultDirection = where.second;
    }

    if (!lists.isEmpty()) {
      dictionary.deleteAllSearchResultLists();
      if(params.needRunSearch) {
        for (int listIndex : lists)
          if (SEARCH_TYPE_FTS.equals(params.searchType))
            dictionary.fullTextSearch(listIndex, params.word, MAX_WORDS_FTS, params.sortType);
           else if (SEARCH_TYPE_WILD_CARD.equals(params.searchType))
             dictionary.wildCardSearch(listIndex, params.word, MAX_WORDS_WILD_CARD);
           else if(SEARCH_TYPE_DID_YOU_MEAN.equals(params.searchType))
              dictionary.didYouMeanSearch(listIndex, params.word, MAX_WORDS_DID_YOU_MEAN);
            else if(SEARCH_TYPE_ANAGRAM.equals(params.searchType))
              dictionary.anagramSearch(listIndex, params.word, MAX_WORDS_ANAGRAM);
            else if(SEARCH_TYPE_FUZZY.equals(params.searchType))
                dictionary.fuzzySearch(listIndex, params.word, MAX_WORDS_FUZZY);
      }
      SparseArray<ListInfo> searchResultLists = dictionary.getLists(ListType.RegularSearch);
      int n = searchResultLists.size();
      int[] indices = new int[n];
      for (int i = 0; i < n; i++)
        indices[i] = searchResultLists.keyAt(i);
      res = new SpecialSearchResult(params.dictionaryId, dictionary, indices, resultDirection, params.word);
    }

    return res;
  }

  private void updateDictionary( Context applicationContext, @Nullable Dictionary _dictionary, boolean openMorpho )
  {
    DictionaryLocation location = null;
    if (null != _dictionary)
    {
      location = _dictionary.getDictionaryLocation();
    }
    if ( location != null )
    {
      DictionaryLocation currentLocation = null;
      if ( dictionary != null )
      {
        currentLocation = dictionary.getLocation();
      }
      if ( !location.equals(currentLocation) )
      {
        dictionary = NativeDictionary.open(applicationContext, _dictionary, openMorpho, true);
      }
      else
      {
        // in case when some sound base was downloaded at the moment when current dictionary was already opened
        ExternalBasesHolder.openExternalBases(applicationContext, _dictionary);
      }
    }
    else
    {
      dictionary = null;
    }
  }

  @NonNull
  private Pair<Integer, Dictionary.Direction> findList(@NonNull NativeDictionary dictionary,
                                                       @NonNull String word,
                                                       @Nullable Dictionary.Direction direction,
                                                       @Nullable Collection<Dictionary.Direction> availableDirections) {
    int listIndex = -1;
    Dictionary.Direction resultDirection = direction;
    Collection<Dictionary.Direction> available = availableDirections;
    if(available == null)
      available = Collections.emptySet();

    if (direction != null) {
      int lang1 = direction.getLanguageFrom();
      int lang2 = direction.getLanguageTo();
      SparseArray< ListInfo > listInfo = dictionary.getLists(ListType.Group.Main);
      int i, n = listInfo.size();

      for ( i = 0; i < n ; i++ ) {
        ListInfo info = listInfo.valueAt(i);
        if (( info.languageFrom == lang1 ) && ( info.languageTo == lang2 )) {
          listIndex = listInfo.keyAt(i);
          break;
        }
      }

      // Try to find better direction
      if ((!available.isEmpty()) && (-1 != listIndex)) {
        int selectedListIndex = dictionary.switchDirection(listIndex, word);
        if ((-1 != selectedListIndex) && (listIndex != selectedListIndex)) {
          ListInfo selectedListInfo = listInfo.get(selectedListIndex);
          if(selectedListInfo != null)
            for(Dictionary.Direction newDirection : available)
              if((newDirection.getLanguageFrom() == selectedListInfo.languageFrom) && (newDirection.getLanguageTo() == selectedListInfo.languageTo)) {
                listIndex = selectedListIndex;
                resultDirection = newDirection;
                break;
              }
        }
      }
    }

    return new Pair<>(listIndex, resultDirection);
  }

  @NonNull
  private List< Integer > findLists( @Nullable Dictionary.Direction direction, @Nullable ListType listType,
                                     @Nullable ListType.Group listGroup, boolean checkLanguageTo )
  {
    List< Integer > res = Collections.emptyList();
    if ( ( direction != null ) && ( dictionary != null ) && ( ( listType != null ) || ( listGroup != null ) ) )
    {
      int lang1 = direction.getLanguageFrom();
      int lang2 = direction.getLanguageTo();
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
        if ( ( info.languageFrom == lang1 ) && ( ( info.languageTo == lang2 ) || ( !checkLanguageTo ) ) )
        {
          res.add(listInfo.keyAt(i));
        }
      }
    }
    return res;
  }
}
