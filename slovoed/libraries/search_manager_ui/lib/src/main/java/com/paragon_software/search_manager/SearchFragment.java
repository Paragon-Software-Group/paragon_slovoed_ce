package com.paragon_software.search_manager;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.search_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.List;

import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_DID_YOU_MEAN;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_FTS;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_WILD_CARD;


public class SearchFragment extends Fragment implements View.OnClickListener, TextWatcher,
                                                        TextView.OnEditorActionListener,
                                                        OnArticleClickListener,
                                                        OnDictionarySelect,
                                                        OnControllerFontSizeChangeListener,
                                                        OnControllerErrorListener,
                                                        DictionaryFilterAdapter.OnDictionaryFilterSelect,
                                                        CollectionView.OnScrollToPosition,
                                                        CollectionView.OnItemRangeChanged,
                                                        SpecialSearchResultAdapter.OnSpecialSearchGroupExpandClickListener,
                                                        OnDictionaryListChanged, CollectionView.OnProgressChanged
{
  private        SearchController         mSearchController;
  private        RecyclerView             mNormalResultList;
  private        RecyclerView             mSpecialSearchResultList;
  private        RecyclerView             mDictionaryFilterList;
  private        RecyclerView             mMorphoList;
  private        View                     mSearchQueryHint;
  private        TextView                 mSearchQueryHintText;
  private        View                     mNormalResultContainer;
  private        SearchResultAdapter      mNormalSearchAdapter;
  private SpecialSearchResultAdapter mSpecialSearchAdapter;
  private        DictionaryFilterAdapter  mDictionaryFilterAdapter;
  private        View                     mSearchTextContainer;
  private        View                     mSearchResultContainer;
  private        View                     mSpecialSearchResultContainer;
  private        RecyclerView             mSpecialSearchExpandedGroupList;
  private        View                     mSpecialSearchGroupContainer;
  private        TextView                 mSpecialSearchGroupHeader;
  private        View                     mSpecialSearchCollapseResults;
  private        View                     mNoDataResultHint;
  private        EditText                 mSearchText;
  private        ImageButton              mSearchType;
  private        ImageButton              mClearSearchText;
  private SpecialSearchListener           mSpecialSearchListener                   = new SpecialSearchListener();
  private        DictionaryFilterListener mDictionaryFilterListener                = new DictionaryFilterListener();
  private ExpandedSpecialSearchGroupListener mExpandedSpecialSearchGroupListener   = new ExpandedSpecialSearchGroupListener();
  private        MorphoResultListener     mMorphoResultListener                    = new MorphoResultListener();
  private SpecialSearchGroupAdapter       mSpecialSearchExpandedGroupAdapter;
  private        View                     mDownloadAllDictionaries;
  private        View                     mMorphoView;
  private        MorphoAdapter            mMorphoAdapter;

  @NonNull
  private Handler mHandler = new Handler();

  private static boolean firstShow = true;

  public static SearchFragment newInstance()
  {
    return new SearchFragment();
  }

  @Nullable
  @Override
  public View onCreateView( @NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                            @Nullable Bundle savedInstanceState )
  {

    View v = inflater.inflate(R.layout.fragment_search, container, false);
    initViews(v);
    return v;
  }

  private void initController()
  {
    if ( SearchManagerHolder.getManager() != null )
    {
      mSearchController = SearchManagerHolder.getManager().getController(SearchController.CONTROLLER_TYPE_DEFAULT);
      mSearchController.setContext(getContext());
      mSearchController.restoreSearchState();
    }
  }

  @Override
  public void onResume()
  {
    super.onResume();
    initController();
    updateViews();
    mSearchController.registerNotifier(this);
    CollectionView<ArticleItem, GroupHeader> expandedSpecialSearchGroup = mSearchController.getExpandedSpecialSearchGroup();
    if (null != expandedSpecialSearchGroup)
    {
      expandedSpecialSearchGroup.registerListener(this);
    }
    checkObsoleteSearchResults();
  }

  @Override
  public void onPause()
  {
    hideSoftwareKeyboard(mSearchText);
    mHandler.removeCallbacksAndMessages(null);
    mSearchController.unRegisterNotifier(this);
    if ( SearchManagerHolder.getManager() != null )
    {
      SearchManagerHolder.getManager().freeController(SearchController.CONTROLLER_TYPE_DEFAULT);
    }

    CollectionView<ArticleItem, ?> result = mNormalSearchAdapter.getData();
    if (null != result)
    {
      result.unregisterListener(this);
    }

    CollectionView<ArticleItem, Void> morphoResult = mMorphoAdapter.getData();
    if (null != morphoResult)
    {
      morphoResult.unregisterListener(mMorphoResultListener);
    }

    CollectionView<Dictionary.DictionaryId, Void> dictionaryFilterList = mDictionaryFilterAdapter.getData();
    if (null != dictionaryFilterList)
    {
      dictionaryFilterList.unregisterListener(mDictionaryFilterListener);
    }

    CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> specialSearchArticles = mSearchController.getSpecialSearchArticles();
    if (null != specialSearchArticles)
    {
      specialSearchArticles.unregisterListener(mSpecialSearchListener);
    }

    CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> specialSearchResult = mSpecialSearchAdapter.getData();
    if (null != specialSearchResult)
    {
      specialSearchResult.unregisterListener(mSpecialSearchListener);
    }

    CollectionView<ArticleItem, GroupHeader> expandedSpecialSearchGroup = mSearchController.getExpandedSpecialSearchGroup();
    if (null != expandedSpecialSearchGroup)
    {
      expandedSpecialSearchGroup.unregisterListener(mExpandedSpecialSearchGroupListener);
    }

    ScrollState scrollState;
    SearchType searchType = mSearchController.getSelectedSearch();
    if (searchType==null)
    {
      scrollState = getScrollState(mNormalResultList);
      if (null != scrollState)
      {
        mSearchController.setNormalSearchScrollState(scrollState);
      }
      scrollState = getScrollState(mDictionaryFilterList);
      if (null != scrollState)
      {
        mSearchController.setDictionaryFilterScrollState(scrollState);
      }
    }
    else
    {
      scrollState = getScrollState(mSpecialSearchResultList);
      if (null != scrollState) {
        mSearchController.setSpecialSearchScrollState(scrollState);
      }
      scrollState = getScrollState(mSpecialSearchExpandedGroupList);
      if (null != scrollState) {
        mSearchController.setExpandedSpecialSearchGroupScrollState(scrollState);
      }
    }


    mSearchText.setOnEditorActionListener(null);
    mSearchText.removeTextChangedListener(this);
    super.onPause();
  }

  @Nullable
  private ScrollState getScrollState(@NonNull RecyclerView view)
  {
    LinearLayoutManager manager = (LinearLayoutManager) view.getLayoutManager();
    int position = manager.findFirstVisibleItemPosition();
    View item = manager.findViewByPosition(position);
    int offset;
    if (null == item)
    {
      return null;
    }
    else
    {
      if (RecyclerView.VERTICAL == manager.getOrientation())
      {
        offset = item.getTop();
      }
      else
      {
        offset = item.getLeft();
      }
    }
    return new ScrollState(position,offset);
  }

  private void setScrollState(@NonNull RecyclerView view, @NonNull ScrollState state)
  {
    LinearLayoutManager manager =(LinearLayoutManager) view.getLayoutManager();
    manager.scrollToPositionWithOffset(state.getPosition(),state.getOffset());
  }

  private void initViews( View fragmentView )
  {

    mDownloadAllDictionaries = fragmentView.findViewById(R.id.download_all_dictionaries);

    mSearchTextContainer = fragmentView.findViewById(R.id.search_text_container);
    mSearchTextContainer.setBackgroundColor(getResources().getColor(R.color.transparent));
    mSearchType = fragmentView.findViewById(R.id.searchType);
    mSearchText = fragmentView.findViewById(R.id.searchText);
    mClearSearchText = fragmentView.findViewById(R.id.clearSearchText);

    mDictionaryFilterList = fragmentView.findViewById(R.id.filter_list);
    mDictionaryFilterList.setHasFixedSize(true);
    mDictionaryFilterList.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.HORIZONTAL, false));

    mSearchQueryHint = fragmentView.findViewById(R.id.search_query_hint);
    mSearchQueryHintText = fragmentView.findViewById(R.id.search_query_hint_text);

    mSearchResultContainer = fragmentView.findViewById(R.id.search_result_container);
    mNormalResultContainer = fragmentView.findViewById(R.id.normal_result_container);
    mNormalResultList = fragmentView.findViewById(R.id.normal_result_list);
    mNormalResultList.setHasFixedSize(true);
    mNormalResultList.setLayoutManager(new LinearLayoutManager(getContext()));

    mSpecialSearchResultContainer = fragmentView.findViewById(R.id.special_search_result_container);
    mSpecialSearchResultList = fragmentView.findViewById(R.id.special_search_result_list);
    mSpecialSearchResultList.setHasFixedSize(true);
    mSpecialSearchResultList.setLayoutManager(new LinearLayoutManager(getContext()));

    mSpecialSearchGroupContainer = fragmentView.findViewById(R.id.special_search_group_container);
    mSpecialSearchGroupHeader = fragmentView.findViewById(R.id.special_search_group_header);

    mSpecialSearchExpandedGroupList = fragmentView.findViewById(R.id.special_search_group_list);
    mSpecialSearchExpandedGroupList.setHasFixedSize(true);
    mSpecialSearchExpandedGroupList.setLayoutManager(new LinearLayoutManager(getContext()));

    mSpecialSearchCollapseResults = fragmentView.findViewById(R.id.special_search_group_collapse_results);
    mSpecialSearchCollapseResults.setOnClickListener(this);

    mNoDataResultHint = fragmentView.findViewById(R.id.no_data_result_hint);

    mMorphoView = fragmentView.findViewById(R.id.morphoView);
    mMorphoList = fragmentView.findViewById(R.id.morphoList);
    mMorphoList.setLayoutManager(new LinearLayoutManager(getContext()));
  }


  private void updateViews()
  {
    mSearchType.setOnClickListener(this);
    String text = mSearchController.getSearchText();
    if (null == text || text.isEmpty())
    {
      mClearSearchText.setVisibility(View.GONE);
    }
    else
    {
      mClearSearchText.setVisibility(View.VISIBLE);
    }
    mSearchText.setText(text);
    mSearchText.setOnEditorActionListener(this);
    mSearchText.addTextChangedListener(this);
    mClearSearchText.setOnClickListener(this);

    mDictionaryFilterAdapter = new DictionaryFilterAdapter();
    mDictionaryFilterAdapter.setSearchController(mSearchController);
    mDictionaryFilterAdapter.setDictionaries((List<Dictionary>) mSearchController.getDictionaries());
    CollectionView<Dictionary.DictionaryId, Void> dictionaryFilterList = mSearchController.getFilterDictionaryIdList();
    if (null != dictionaryFilterList)
    {
      dictionaryFilterList.registerListener(mDictionaryFilterListener);
    }
    mDictionaryFilterAdapter.setData(dictionaryFilterList);
    mDictionaryFilterAdapter.setSelectedDictionaryId(mSearchController.getFilterDictionaryId());
    mDictionaryFilterList.setAdapter(mDictionaryFilterAdapter);
    mDictionaryFilterList.scrollToPosition(mDictionaryFilterAdapter.getSelectedPosition());
    mDictionaryFilterAdapter.registerOnDictionaryFilterSelectListener(this);

    mNormalSearchAdapter = new SearchResultAdapter();
    mNormalSearchAdapter.registerOnArticleClickListener(this);
    mNormalSearchAdapter.setSearchController(mSearchController);
    Dictionary.DictionaryId selectedDictionary = mSearchController.getSelectedDictionary();
    mNormalSearchAdapter.selectDictionary(selectedDictionary);
    CollectionView result;
    if (null == selectedDictionary)
    {
      SearchAllResult searchAllResult = mSearchController.getSearchAllArticles();
      if (null == searchAllResult)
      {
        result = null;
      }
      else
      {
        result = searchAllResult.getArticleItemList();
      }
    }
    else
    {
        result = mSearchController.getArticles();
    }
    if (null != result)
    {
      result.registerListener(this);
    }
    mNormalSearchAdapter.setDictionaries((List<Dictionary>)mSearchController.getDictionaries());
    mNormalSearchAdapter.setData(result,mSearchController.getEntryListFontSize());
    mNormalSearchAdapter.setFilterDictionaryId(mSearchController.getFilterDictionaryId());
    mNormalResultList.setAdapter(mNormalSearchAdapter);

    SearchType searchType = mSearchController.getSelectedSearch();
    if (searchType == null)
    {
      setScrollState(mNormalResultList, mSearchController.getNormalSearchScrollState());
      setScrollState(mDictionaryFilterList, mSearchController.getDictionaryFilterScrollState());
    }
    else
    {
      setScrollState(mSpecialSearchResultList,mSearchController.getSpecialSearchScrollState());
      setScrollState(mSpecialSearchExpandedGroupList,mSearchController.getExpandedSpecialSearchGroupScrollState());
    }

    mSpecialSearchAdapter = new SpecialSearchResultAdapter();
    mSpecialSearchAdapter.registerOnSpecialSearchGroupExpandClickListener(this);
    mSpecialSearchAdapter.registerOnArticleClickListener(this);
    mSpecialSearchAdapter.setSearchController(mSearchController);
    CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> specialSearchResult = mSearchController.getSpecialSearchArticles();
    if (null != specialSearchResult)
    {
      specialSearchResult.registerListener(mSpecialSearchListener);
    }
    mSpecialSearchAdapter.setData(specialSearchResult,mSearchController.getEntryListFontSize());
    mSpecialSearchResultList.setAdapter(mSpecialSearchAdapter);

    String header = mSearchController.getExpandedGroupHeader();
    if (null != header)
    {
      mSpecialSearchGroupHeader.setText(header);
    }
    mSpecialSearchExpandedGroupAdapter = new SpecialSearchGroupAdapter();
    mSpecialSearchExpandedGroupAdapter.registerOnArticleClickListener(this);
    mSpecialSearchExpandedGroupAdapter.setSearchController(mSearchController);
    CollectionView<ArticleItem, GroupHeader> specialSearchGroupResult = mSearchController.getExpandedSpecialSearchGroup();
    if (null != specialSearchGroupResult)
    {
        specialSearchGroupResult.registerListener(mExpandedSpecialSearchGroupListener);
    }
    mSpecialSearchExpandedGroupAdapter.setData(specialSearchGroupResult,mSearchController.getEntryListFontSize());
    mSpecialSearchExpandedGroupList.setAdapter(mSpecialSearchExpandedGroupAdapter);

    mMorphoAdapter = new MorphoAdapter();
    mMorphoAdapter.setSearchController(mSearchController);
    mMorphoAdapter.registerOnArticleClickListener(this);
    mMorphoAdapter.setData(mSearchController.getMorphoArticles(),mSearchController.getEntryListFontSize());
    mMorphoList.setAdapter(mMorphoAdapter);

    if (mMorphoAdapter.getItemCount() > 0)
    {
      mMorphoList.setVisibility(View.VISIBLE);
    }

    updateSearchContainersVisibility();
    updateDictionaryFilterVisibility();
    checkNoDataResult();

    // Update "All Dictionaries" item visibility in toolbar
    mSearchController.setSelectedSearch(searchType);

    if ( mSearchController.isShowKeyboardEnabled() )
    {
      showSoftwareKeyboard(mSearchText, firstShow);
      mSearchText.selectAll();
    }
    firstShow = false;
  }

  private void checkObsoleteSearchResults()
  {
    if (mSearchController.needUpdateSearchResults())
    {
      search(mSearchText.getText().toString(),false);
    }
  }

  @Override
  public void onClick( View v )
  {
    if (v == mSearchType)
    {
      SearchType searchType = mSearchController.getSelectedSearch();
      onDictionaryFilterSelect(null);
      if(SEARCH_TYPE_WILD_CARD.equals(searchType))
      {
        return;
      } else if (searchType == null) {

        mSearchController.setSelectedSearch(SEARCH_TYPE_FTS);
      } else {
        mSearchController.setSelectedSearch(null);
      }

      search(mSearchText.getText().toString(),false);
    }
    else if (v == mClearSearchText)
    {
      mSearchText.setText("");
    }
    else if (v == mSpecialSearchCollapseResults)
    {
      CollectionView<ArticleItem, GroupHeader> specialSearchGroup = mSearchController.getExpandedSpecialSearchGroup();
      if (null != specialSearchGroup)
      {
        specialSearchGroup.unregisterListener(mExpandedSpecialSearchGroupListener);
      }
      mSearchController.setSpecialSearchGroupExpanded(false);
      mSearchController.setExpandedSpecialSearchGroup(null);
      mSearchController.setExpandedGroupHeader(null);

      updateSearchContainersVisibility();
    }
  }

  private void updateSearchContainersVisibility()
  {
    SearchType searchType = mSearchController.getSelectedSearch();
    if (searchType == null) {
      mSearchType.setImageResource(R.drawable.ic_search_headwords_24dp);
      mSearchText.setHint(R.string.utils_slovoed_ui_common_search);
      mSpecialSearchResultList.setVisibility(View.GONE);
      mSearchText.setEnabled(true);
      mClearSearchText.setEnabled(true);

      if (mSearchText.getText().toString().isEmpty()) {
        mSearchResultContainer.setVisibility(View.GONE);
        mNormalResultContainer.setVisibility(View.GONE);
        mSearchQueryHint.setVisibility(View.VISIBLE);
        mSearchQueryHintText.setText(R.string.search_manager_ui_search_hint);
      } else {
        mSearchResultContainer.setVisibility(View.VISIBLE);
        mNormalResultContainer.setVisibility(View.VISIBLE);
        mSearchQueryHint.setVisibility(View.GONE);
      }

      if (null != mSearchController.getSelectedDictionary()) {
        if (mMorphoAdapter.getItemCount() > 0) {
          mMorphoView.setVisibility(View.VISIBLE);
        } else {
          mMorphoView.setVisibility(View.GONE);
        }
      } else {
        mMorphoView.setVisibility(View.GONE);
      }
    } else {
      switch (searchType) {
        case SEARCH_TYPE_FTS:
        case SEARCH_TYPE_DID_YOU_MEAN:
          mSearchType.setImageResource(R.drawable.ic_search_fts_24dp);
          mSearchText.setHint(R.string.search_manager_ui_edit_text_fts_search_hint);
          mNormalResultContainer.setVisibility(View.GONE);
          mSpecialSearchResultContainer.setVisibility(View.VISIBLE);
          mMorphoView.setVisibility(View.GONE);

          if (null == mSearchController.getSelectedDictionary()) {
            mSearchResultContainer.setVisibility(View.GONE);
            mSearchText.setEnabled(false);
            mClearSearchText.setEnabled(false);
            mSearchQueryHint.setVisibility(View.VISIBLE);
            mSearchQueryHintText.setText(R.string.search_manager_ui_select_dictionary);
          } else {
            mSearchText.setEnabled(true);
            mClearSearchText.setEnabled(true);
            if (mSearchText.getText().toString().isEmpty()) {
              mSearchResultContainer.setVisibility(View.GONE);
              mSearchQueryHint.setVisibility(View.VISIBLE);
              mSearchQueryHintText.setText(R.string.search_manager_ui_search_hint);
            } else {
              mSearchResultContainer.setVisibility(View.VISIBLE);
              mSearchQueryHint.setVisibility(View.GONE);
              if (mSearchController.isSpecialSearchGroupExpanded()) {
                mSpecialSearchResultList.setVisibility(View.GONE);
                mSpecialSearchGroupContainer.setVisibility(View.VISIBLE);
              } else {
                mSpecialSearchResultList.setVisibility(View.VISIBLE);
                mSpecialSearchGroupContainer.setVisibility(View.GONE);
              }
            }
          }


          break;

        case SEARCH_TYPE_WILD_CARD:
          mSearchType.setImageResource(R.drawable.ic_search_wildcard_24dp);
          mSearchText.setEnabled(true);
          mClearSearchText.setEnabled(true);

          if (null == mSearchController.getSelectedDictionary()) {
            mSearchResultContainer.setVisibility(View.GONE);
            mSearchQueryHint.setVisibility(View.VISIBLE);
            mSearchQueryHintText.setText(R.string.search_manager_ui_select_dictionary);
          } else {
            mSearchResultContainer.setVisibility(View.VISIBLE);
            if (mSearchController.isSpecialSearchGroupExpanded()) {
              mSpecialSearchResultList.setVisibility(View.GONE);
              mSpecialSearchGroupContainer.setVisibility(View.VISIBLE);
            } else {
              mSpecialSearchResultList.setVisibility(View.VISIBLE);
              mSpecialSearchGroupContainer.setVisibility(View.GONE);
            }
            mSearchQueryHint.setVisibility(View.GONE);
          }

          if (mMorphoAdapter.getItemCount() > 0) {
            mMorphoView.setVisibility(View.VISIBLE);
          } else {
            mMorphoView.setVisibility(View.GONE);
          }
          break;
      }
    }
  }

  @Override
  public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
    if (actionId == EditorInfo.IME_ACTION_SEARCH)
    {

      search(v.getText().toString(),true);

      hideSoftwareKeyboard(v);
      return true;
    }
    return false;
  }

  private void hideSoftwareKeyboard(View view)
  {
    InputMethodManager imm = (InputMethodManager)getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
    imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
  }

  private void showSoftwareKeyboard( final @Nullable View view, boolean appStartup )
  {
    // 1000 ms delay is needed to show keyboard at app startup,
    // 300 ms in other cases (hide/show app, orientation change)
    mHandler.postDelayed(new Runnable()
    {
      @Override
      public void run()
      {
        if ( getActivity() != null && view != null && view.requestFocus() )
        {
          InputMethodManager imm = (InputMethodManager) getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
          if (imm != null)
          {
            // InputMethodManager.SHOW_FORCED to show keyboard even in landscape orientation
            imm.showSoftInput(view, InputMethodManager.SHOW_FORCED);
          }
        }
      }
    }, appStartup ? 1000 : 300);
  }

  @Override
  public void onArticleClick( ArticleItem  item )
  {
    if (SEARCH_TYPE_DID_YOU_MEAN.equals(mSearchController.getSelectedSearch())) {
      resetDidYouMeanSearch();
      mSearchText.setText(item.getLabel());
    } else {
      mSearchController.openArticle(item);
    }
  }

  private void showResultList()
  {
    mNoDataResultHint.setVisibility(View.GONE);

    SearchType searchType = mSearchController.getSelectedSearch();
    if (searchType == null)
    {
      mSpecialSearchResultContainer.setVisibility(View.GONE);
      mNormalResultContainer.setVisibility(View.VISIBLE);
    }
    else
    {
      mSpecialSearchResultContainer.setVisibility(View.VISIBLE);
      mNormalResultContainer.setVisibility(View.GONE);
    }
  }


  private void checkNoDataResult()
  {
    SearchType searchType = mSearchController.getSelectedSearch();
    if (searchType == null)
    {
      mSpecialSearchResultContainer.setVisibility(View.GONE);
      if (hasNormalResults())
      {
        mNormalResultContainer.setVisibility(View.VISIBLE);
        mNoDataResultHint.setVisibility(View.GONE);
      }
      else
      {
        mNormalResultContainer.setVisibility(View.GONE);
        mNoDataResultHint.setVisibility(View.VISIBLE);
      }
    }
    else
    {
      mNormalResultContainer.setVisibility(View.GONE);
      if (hasSpecialSearchResults())
      {
        mSpecialSearchResultContainer.setVisibility(View.VISIBLE);
        mNoDataResultHint.setVisibility(View.GONE);
      }
      else
      {
        mSpecialSearchResultContainer.setVisibility(View.GONE);
        mNoDataResultHint.setVisibility(View.VISIBLE);
      }
    }
  }

  private boolean hasNormalResults()
  {
    Dictionary.DictionaryId selectedDictionaryId = mSearchController.getSelectedDictionary();
    CollectionView articles;
    if (null == selectedDictionaryId)
    {
      if (null != mSearchController.getSearchAllArticles())
      {
        articles = mSearchController.getSearchAllArticles().getArticleItemList();
      }
      else
      {
        articles = null;
      }
    }
    else
    {
      articles = mSearchController.getArticles();
    }
    if (null == articles)
    {
      return false;
    }
    if (articles.isInProgress())
    {
      return true;
    }
    return 0 != articles.getCount();
  }

  //Main usage in function checkNoDataResult(). Show "no data result" hint if false
  private boolean hasSpecialSearchResults()
  {
    CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> specialSearchArticles = mSearchController.getSpecialSearchArticles();
    if (null == specialSearchArticles)
    {
      return false;
    }
    if (specialSearchArticles.isInProgress())
    {
      return true;
    }
    if (0 == specialSearchArticles.getCount())
    {
      return false;
    }
    for (int i = 0; i < specialSearchArticles.getCount(); ++i)
    {
      if (0 != specialSearchArticles.getItem(i).getCount())
      {
        return true;
      }
    }
    return false;
  }

  private void search(String text, boolean autoChangeDirection)
  {
    if ( null != getActivity() && null != text && mSearchController.launchTestMode(getActivity(), text) )
    {
      mSearchText.setText("");
      return;
    }

    mSearchController.setSearchText(text);
    mSearchController.setSpecialSearchGroupExpanded(false);
    mSearchController.setExpandedSpecialSearchGroup(null);
    mSearchController.setExpandedGroupHeader(null);

    updateSearchContainersVisibility();
    updateDictionaryFilterVisibility();
    onDictionaryFilterSelect(null);

    if (null == text)
    {
      text = "";
    }

    SearchType searchType = mSearchController.getSelectedSearch();
    if (searchType == null) {
      Dictionary.DictionaryId selectedDictionaryId = mSearchController.getSelectedDictionary();
      mNormalSearchAdapter.selectDictionary(selectedDictionaryId);
      CollectionView<ArticleItem, ?> result = null;
      CollectionView<ArticleItem, Void> morphoResult = null;
      if (null == selectedDictionaryId) {
        SearchAllResult searchAllResult = mSearchController.searchAll(text);
        if (null != searchAllResult) {
          result = searchAllResult.getArticleItemList();
          CollectionView<Dictionary.DictionaryId, Void> dictionaryIdList = searchAllResult.getDictionaryIdList();
          if (null != dictionaryIdList) {
            CollectionView<Dictionary.DictionaryId, Void> previousDictIdList = mDictionaryFilterAdapter.getData();
            if (null != previousDictIdList && previousDictIdList.equals(dictionaryIdList)) {
              previousDictIdList.unregisterListener(mDictionaryFilterListener);
            }
            dictionaryIdList.registerListener(mDictionaryFilterListener);
          }
          mDictionaryFilterAdapter.setData(dictionaryIdList);
        }
      } else {
        ScrollResult scrollResult = mSearchController.scroll(text, autoChangeDirection, false);
        result = scrollResult.getArticleItemList();
        morphoResult = scrollResult.getMorphoArticleItemList();
      }

      if (null != result) {
        CollectionView<ArticleItem, ?> previousResult = mNormalSearchAdapter.getData();
        if (null != previousResult && !previousResult.equals(result)) {
          previousResult.unregisterListener(this);
        }
        result.registerListener(this);
      }

      if (null != morphoResult) {
        CollectionView<ArticleItem ,Void> previousResult = mMorphoAdapter.getData();
        if (null != previousResult && !previousResult.equals(morphoResult)) {
          previousResult.unregisterListener(mMorphoResultListener);
        }
        morphoResult.registerListener(mMorphoResultListener);
      }
      mNormalSearchAdapter.setData(result, mSearchController.getEntryListFontSize());
      mMorphoAdapter.setData(morphoResult, mSearchController.getEntryListFontSize());
    } else {
      if (null == mSearchController.getSelectedDictionary()) {
        return;
      }
      CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> result = mSearchController.search(text, autoChangeDirection, searchType, SortType.Full);
      if (null != result) {
        CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> previousResult = mSpecialSearchAdapter.getData();
        if (null != previousResult && !previousResult.equals(result)) {
          previousResult.unregisterListener(mSpecialSearchListener);
        }
        result.registerListener(mSpecialSearchListener);
      }
      mSpecialSearchAdapter.setData(result, mSearchController.getEntryListFontSize());
      mSpecialSearchExpandedGroupAdapter.setData(mSearchController.getExpandedSpecialSearchGroup(), mSearchController.getEntryListFontSize());
    }
    updateSearchContainersVisibility();
    updateDictionaryFilterVisibility();
		showResultList();
  }

  @Override
  public void onDictionarySelect( @Nullable Dictionary.DictionaryId dictionaryId )
  {
    resetDidYouMeanSearch();
    search(mSearchText.getText().toString(),true);
  }

  @Override
  public void onDirectionSelect( int direction )
  {
    resetDidYouMeanSearch();
    search(mSearchText.getText().toString(),true);
  }

  @Override
  public void beforeTextChanged( CharSequence s, int start, int count, int after )
  {

  }

  private void resetDidYouMeanSearch()
  {
    if (SEARCH_TYPE_DID_YOU_MEAN.equals(mSearchController.getSelectedSearch()))
    {
      mSearchController.setSelectedSearch(SEARCH_TYPE_FTS);
    }
  }

  @Override
  public void onTextChanged( CharSequence s, int start, int before, int count )
  {
    resetDidYouMeanSearch();
    String text = mSearchText.getText().toString();
    if (text.isEmpty())
    {
      mClearSearchText.setVisibility(View.GONE);
    }
    else
    {
      mClearSearchText.setVisibility(View.VISIBLE);
    }
    search(s.toString(),true);
  }


  @Override
  public void afterTextChanged( Editable s )
  {

  }

  @Override
  public void onControllerEntryListFontSizeChanged( )
  {
    mNormalSearchAdapter.setEntryListFontSize(mSearchController.getEntryListFontSize());
    mSpecialSearchAdapter.setEntryListFontSize(mSearchController.getEntryListFontSize());
    mSpecialSearchExpandedGroupAdapter.setEntryListFontSize(mSearchController.getEntryListFontSize());
  }

  @Override
  public void onControllerError( Exception exception )
  {
    Toast.makeText(getContext(), exception.getMessage(), Toast.LENGTH_SHORT).show();
  }

  @Override
  public void onDictionaryFilterSelect( Dictionary.DictionaryId dictionaryId )
  {
    mSearchController.setFilterDictionaryId(dictionaryId);
    mNormalSearchAdapter.setFilterDictionaryId(dictionaryId);
  }

  @Override
  public void onScrollToPosition( @NonNull CollectionView collectionView )
  {
    if (collectionView.getMetadata() instanceof Dictionary.Direction)
    {
      mSearchController.setSelectedDirection((Dictionary.Direction) collectionView.getMetadata());
    }
    LinearLayoutManager manager =(LinearLayoutManager) mNormalResultList.getLayoutManager();
    manager.scrollToPositionWithOffset(collectionView.getPosition(),0);
  }

  @Override
  public void onItemRangeChanged( final CollectionView.OPERATION_TYPE type, final int position, final int count )
  {
    switch ( type )
    {
      case ITEM_RANGE_INSERTED:
        mNormalSearchAdapter.notifyRangeInserted(position,count);
        break;
      case ITEM_RANGE_REMOVED:
        mNormalSearchAdapter.notifyItemRangeRemoved(position,count);
        break;
      case ITEM_RANGE_CHANGED:
        mNormalSearchAdapter.notifyItemRangeChanged(position,count);
        break;
    }
  }

  private void updateDictionaryFilterVisibility()
  {
    SearchType searchType = mSearchController.getSelectedSearch();
    if (searchType == null
        && null == mSearchController.getSelectedDictionary()
        && null != mDictionaryFilterAdapter.getData()
        && 1 < mDictionaryFilterAdapter.getData().getCount()
        && null != mSearchText.getText()
        && !mSearchText.getText().toString().isEmpty())
    {
      mDictionaryFilterList.setVisibility(View.VISIBLE);
      mSearchTextContainer.setBackgroundColor(getResources().getColor(R.color.white));
    }
    else
    {
      mDictionaryFilterList.setVisibility(View.GONE);
      mSearchTextContainer.setBackgroundColor(getResources().getColor(R.color.transparent));
    }
  }

  @Override
  public void onSpecialSearchGroupExpandClick( CollectionView<ArticleItem, GroupHeader> view, String header )

  {
    mSearchController.setSpecialSearchGroupExpanded(true);
    mSearchController.setExpandedSpecialSearchGroup(view);
    mSearchController.setExpandedGroupHeader(header);
    mSpecialSearchGroupHeader.setText(header);

    mSpecialSearchExpandedGroupAdapter.setData(view,mSearchController.getEntryListFontSize());
    mSpecialSearchExpandedGroupList.scrollToPosition(0);

    view.registerListener(mExpandedSpecialSearchGroupListener);

    updateSearchContainersVisibility();
  }

  @Override
  public void onDictionaryListChanged()
  {
    mNormalSearchAdapter.setDictionaries((List<Dictionary>)mSearchController.getDictionaries());
    mDictionaryFilterAdapter.setDictionaries((List<Dictionary>)mSearchController.getDictionaries());
    search(mSearchText.getText().toString(), true);
  }

  @Override
  public void onProgressChanged()
  {
    mNormalSearchAdapter.notifyDataSetChanged();
    checkNoDataResult();
  }


  class DictionaryFilterListener implements CollectionView.OnItemRangeChanged
  {

    @Override
    public void onItemRangeChanged(CollectionView.OPERATION_TYPE type, int position, int count) {
      if (null == mDictionaryFilterAdapter) return;
      switch ( type )
      {
        case ITEM_RANGE_INSERTED:
          mDictionaryFilterAdapter.notifyItemRangeInserted(position,count);
          break;
        case ITEM_RANGE_REMOVED:
          mDictionaryFilterAdapter.notifyItemRangeRemoved(position,count);
          break;
      }
      updateDictionaryFilterVisibility();
    }
  }

  class SpecialSearchListener implements CollectionView.OnItemRangeChanged, CollectionView.OnProgressChanged
  {
    @Override
    public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int position, int count )
    {

    }

    @Override
    public void onProgressChanged() {
      CollectionView <CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>  data = mSpecialSearchAdapter.getData();
      mSpecialSearchAdapter.notifyDataSetChanged();
      SearchType searchType = mSearchController.getSelectedSearch();
      if (null != data
              && !data.isInProgress()
              && (SEARCH_TYPE_FTS.equals(searchType) && !hasSpecialSearchResults())
              && !mSearchText.getText().toString().isEmpty())
      {
        mSearchController.setSelectedSearch(SEARCH_TYPE_DID_YOU_MEAN);
            search(mSearchText.getText().toString(),false);
            return;
      }

      if (null != data
			  && SEARCH_TYPE_WILD_CARD.equals(searchType))
      {
        mSearchController.setSelectedDirection(data.getMetadata());
      }
      checkNoDataResult();
    }
  }

  class ExpandedSpecialSearchGroupListener implements CollectionView.OnItemRangeChanged
  {
    @Override
    public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int position, int count )
    {
      if (null == mSpecialSearchExpandedGroupAdapter) return;
      switch ( type )
      {
        case ITEM_RANGE_INSERTED:
          mSpecialSearchExpandedGroupAdapter.notifyItemRangeInserted(position,count);
          break;
        case ITEM_RANGE_REMOVED:
          mSpecialSearchExpandedGroupAdapter.notifyItemRangeRemoved(position,count);
          break;
        case ITEM_RANGE_CHANGED:
          mSpecialSearchExpandedGroupAdapter.notifyItemRangeChanged(position,count);
          break;
      }
    }
  }

  class MorphoResultListener implements CollectionView.OnItemRangeChanged
  {
    @Override
    public void onItemRangeChanged(CollectionView.OPERATION_TYPE type, int startPosition, int itemCount)
    {
      if (mMorphoAdapter.getItemCount() > 0)
      {
        mMorphoAdapter.notifyDataSetChanged();
        mMorphoView.setVisibility(View.VISIBLE);
      }
      else
      {
        mMorphoView.setVisibility(View.GONE);
      }
    }
  }
}
