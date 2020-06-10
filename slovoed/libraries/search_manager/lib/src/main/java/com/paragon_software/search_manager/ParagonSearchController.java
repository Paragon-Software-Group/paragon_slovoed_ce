package com.paragon_software.search_manager;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.BackgroundColorSpan;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ShowArticleOptions;
import com.paragon_software.article_manager.SwipeMode;
import com.paragon_software.article_manager.WordReference;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.ArrayList;
import java.util.List;

import io.reactivex.Observable;

import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_DID_YOU_MEAN;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_FTS;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_WILD_CARD;

public class ParagonSearchController implements SearchController
{
  private final List< OnDictionarySelect >                  mOnDictionarySelects                          = new ArrayList<>();
  private final List< OnControllerFontSizeChangeListener >  mOnControllerFontSizeChangeListeners = new ArrayList<>();
  private final List< OnControllerErrorListener >           mOnControllerErrorListeners                   = new ArrayList<>();
  private final List< OnDictionaryListChanged >             mOnDictionaryListChangedListeners             = new ArrayList<>();

  private final SearchManagerAPI                            mSearchManager;
  private final ScreenOpenerAPI                             mScreenOpener;
  private final @Nullable HintManagerAPI                    mHintManager;
  private final @Nullable ToolbarManager                    mToolbarManager;

  private       ScrollResult                                mScrollResult;
  private       SearchAllResult                             mSearchAllResult;
  private       CollectionView< CollectionView< ArticleItem, GroupHeader >
          , Dictionary.Direction >                          mSpecialSearchResult;

                SearchType                                  mSelectedSearch       = null;
                boolean                                     mIsScrollSelected;
  private       Context                                     mContext;
//  TODO need to update mActiveArticle after,
//   when the database might have changed,
//   for example in onDictionaryListChanged ()
  private       ArticleItem                                 mActiveArticle;
                String                                      mSearchText;
  private       Dictionary.DictionaryId                     mSearchDictionaryId;
  private       boolean                                     mNeedUpdateSearchResults;

  private       boolean                                     mSpecialSearchGroupExpanded     = false;
  private       CollectionView<ArticleItem, GroupHeader>    mExpandedSpecialSearchGroup;
  private       String                                      mExpandedGroupHeader;
  private       Dictionary.DictionaryId                     mFilterDictionaryId;

  private       ScrollState mNormalSearchScrollState      = new ScrollState(0,0);
  private       ScrollState mDictionaryFilterScrollState  = new ScrollState(0,0);
  private       ScrollState mSpecialSearchScrollState               = new ScrollState(0,0);
  private       ScrollState mExpandedSpecialSearchGroupScrollState  = new ScrollState(0,0);

  private float mEntryListFontSize = ApplicationSettings.getDefaultFontSize();

  private boolean mShowKeyboard = true;
  private boolean mShowHighlighting = true;

  private final static String highLightColor = "#FFDE18";

  ParagonSearchController(SearchManagerAPI searchManager, ScreenOpenerAPI screenOpener, HintManagerAPI hintManager,
                          ToolbarManager toolbarManager)
  {
    mSearchManager = searchManager;
    mScreenOpener = screenOpener;
    mHintManager = hintManager;
    mToolbarManager = toolbarManager;
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnControllerFontSizeChangeListener && !mOnControllerFontSizeChangeListeners.contains(notifier))
    {
      mOnControllerFontSizeChangeListeners.add((OnControllerFontSizeChangeListener)notifier);
    }
    if (notifier instanceof OnControllerErrorListener && !mOnControllerErrorListeners.contains(notifier))
    {
      mOnControllerErrorListeners.add((OnControllerErrorListener) notifier);
    }
    if (notifier instanceof OnDictionarySelect && !mOnDictionarySelects.contains(notifier))
    {
      mOnDictionarySelects.add((OnDictionarySelect) notifier);
    }
    if (notifier instanceof OnDictionaryListChanged && !mOnDictionaryListChangedListeners.contains(notifier))
    {
      mOnDictionaryListChangedListeners.add((OnDictionaryListChanged) notifier);
    }
  }

  @Override
  public void unRegisterNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnControllerFontSizeChangeListener)
    {
      mOnControllerFontSizeChangeListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerErrorListener)
    {
      mOnControllerErrorListeners.remove(notifier);
    }
    if (notifier instanceof OnDictionarySelect)
    {
      mOnDictionarySelects.remove(notifier);
    }
    if (notifier instanceof OnDictionaryListChanged)
    {
      mOnDictionaryListChangedListeners.remove(notifier);
    }
  }

  @Override
  public void setSelectedSearch( SearchType searchType )
  {
    if(searchType != getSelectedSearch()) {
      setActiveArticle(null);
    }
    if(!SEARCH_TYPE_WILD_CARD.equals(searchType))
    mIsScrollSelected = searchType==null;

    mSelectedSearch = searchType;
    setToolbarFtsMode();
  }

  @Override
  public SearchType getSelectedSearch()
  {
    return mSelectedSearch;
  }

  protected void setToolbarFtsMode()
  {
    boolean value = SEARCH_TYPE_FTS.equals(mSelectedSearch) || SEARCH_TYPE_WILD_CARD.equals(mSelectedSearch);
    if (null != mToolbarManager)
    {
      mToolbarManager.showDictionaryList(value);
    }
  }

  @Override
  public ScrollResult scroll(String word, boolean autoChangeDirection, boolean exactly )
  {
    mScrollResult = mSearchManager.scroll(getSelectedDirection(), word, autoChangeDirection, exactly);
    mNeedUpdateSearchResults = false;
    return mScrollResult;
  }

  @Override
  public SearchAllResult searchAll(String word)
  {
    mSearchAllResult = mSearchManager.searchAll(word);
    mNeedUpdateSearchResults = false;
    return mSearchAllResult;
  }

  @Override
  public CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>
  search(@NonNull String word, boolean autoChangeDirection, @NonNull SearchType searchType, @NonNull SortType sortType)
  {
      return search(word, autoChangeDirection, searchType, sortType, true);
  }

  CollectionView<CollectionView<ArticleItem, GroupHeader>,Dictionary.Direction>
  search(@NonNull String word, boolean autoChangeDirection, @NonNull SearchType searchType, @NonNull SortType sortType, @Nullable Boolean needSearch)
  {
    mSpecialSearchResult = mSearchManager.search(getSelectedDirection(),word, autoChangeDirection, searchType, sortType, needSearch);
    mNeedUpdateSearchResults = false;
    return mSpecialSearchResult;
  }

  @Override
  public void setNeedUpdateSearchResults( boolean needUpdateSearchResults )
  {
    mNeedUpdateSearchResults = needUpdateSearchResults;
  }

  @Override
  public boolean needUpdateSearchResults()
  {
    Dictionary.DictionaryId selectedDictionaryId = getSelectedDictionary();
    if ( null != selectedDictionaryId && !selectedDictionaryId.equals(mSearchDictionaryId))
    {
      mSearchDictionaryId = selectedDictionaryId;
      return true;
    }
    return mNeedUpdateSearchResults;
  }

  @Override
  public void setSearchText( String text )
  {
    if ((text.contains("*") || text.contains("?"))) {
      setSelectedSearch(SEARCH_TYPE_WILD_CARD);
    } else if(!SEARCH_TYPE_DID_YOU_MEAN.equals(mSelectedSearch)){
      setSelectedSearch(mIsScrollSelected ? null : SEARCH_TYPE_FTS);
    }
    mSearchText = text;

    mSearchManager.saveIsScrollSelected(mIsScrollSelected);
    mSearchManager.saveSearchRequest(text);
  }

  @Override
  public String getSearchText()
  {
    return mSearchText;
  }

  @Override
  public float getEntryListFontSize()
  {
    return mEntryListFontSize;
  }

  @Override
  public boolean isShowKeyboardEnabled()
  {
    return mShowKeyboard;
  }

  @Override
  public boolean isShowHighlightingEnabled()
  {
    return mShowHighlighting;
  }

  @Override
  public CollectionView<ArticleItem, Dictionary.Direction> getArticles()
  {
    if (null != mScrollResult)
    {
      return mScrollResult.getArticleItemList();
    }
    else
    {
      return  null;
    }
  }

  @Override
  public CollectionView<ArticleItem, Void> getMorphoArticles()
  {
    if (null != mScrollResult)
    {
      return mScrollResult.getMorphoArticleItemList();
    }
    else
    {
      return null;
    }
  }

  @Override
  public SearchAllResult getSearchAllArticles() {
    return mSearchAllResult;
  }

  @Override
  public CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> getSpecialSearchArticles() {
    return mSpecialSearchResult;
  }

  @Override
  public void setNormalSearchScrollState(@NonNull ScrollState scrollState)
  {
    mNormalSearchScrollState = scrollState;
  }

  @NonNull
  @Override
  public ScrollState getNormalSearchScrollState()
  {
    return mNormalSearchScrollState;
  }

  @Override
  public void setDictionaryFilterScrollState(@NonNull ScrollState scrollState)
  {
      mDictionaryFilterScrollState = scrollState;
  }

  @NonNull
  @Override
  public ScrollState getDictionaryFilterScrollState()
  {
    return mDictionaryFilterScrollState;
  }

  @Override
  public void setSpecialSearchScrollState(@NonNull ScrollState scrollState)
  {
    mSpecialSearchScrollState = scrollState;
  }

  @NonNull
  @Override
  public ScrollState getSpecialSearchScrollState()
  {
    return mSpecialSearchScrollState;
  }

  @Override
  public void setExpandedSpecialSearchGroupScrollState(@NonNull ScrollState scrollState)
  {
    mExpandedSpecialSearchGroupScrollState = scrollState;
  }

  @NonNull
  @Override
  public ScrollState getExpandedSpecialSearchGroupScrollState()
  {
    return mExpandedSpecialSearchGroupScrollState;
  }

  @Override
  public boolean isScrollResultStartsWith( @Nullable String text )
  {
    if ( text != null && mScrollResult != null)
    {
      return mScrollResult.startsWith(text);
    }
    return false;
  }

  @Override
  public Dictionary.DictionaryId getSearchDictionaryId()
  {
    return mSearchDictionaryId;
  }

  @Override
  public ArticleItem getActiveArticle()
  {
    return mActiveArticle;
  }

  @Override
  public void setActiveArticle( @Nullable ArticleItem articleItem )
  {
    mActiveArticle = articleItem;
  }

  void setSelectedDictionary( Dictionary.DictionaryId dictionaryId )
  {
    mSearchDictionaryId = dictionaryId;
    for (OnDictionarySelect listener : mOnDictionarySelects)
    {
      listener.onDictionarySelect(dictionaryId);
    }
  }

  void setSelectedDirection( int direction )
  {
    for (OnDictionarySelect listener : mOnDictionarySelects)
    {
      listener.onDirectionSelect(direction);
    }
  }

  @Override
  public Dictionary.DictionaryId getSelectedDictionary()
  {
    if (mSearchManager instanceof ParagonSearchManager)
    {
      ParagonSearchManager searchManager = (ParagonSearchManager) mSearchManager;
      return searchManager.getSelectedDictionary();
    }
    return null;
  }

  @Override
  public int getSelectedDirection()
  {
    if (mSearchManager instanceof ParagonSearchManager)
    {
      ParagonSearchManager searchManager = (ParagonSearchManager) mSearchManager;
      return searchManager.getSelectedDirection();
    }
    return -1;
  }

  @Override
  public List< Dictionary > getDictionaries()
  {
    if (mSearchManager instanceof ParagonSearchManager) {
      ParagonSearchManager searchManager = (ParagonSearchManager) mSearchManager;
      return searchManager.getDictionaries();
    }
    return null;
  }

  void dictionaryListChanged()
  {
    for (OnDictionaryListChanged notifier : mOnDictionaryListChangedListeners)
    {
      notifier.onDictionaryListChanged();
    }
  }

  @Override
  public CollectionView<Dictionary.DictionaryId, Void> getFilterDictionaryIdList()
  {
    if (null == mSearchAllResult)
    {
      return null;
    }
    else
    {
      return mSearchAllResult.getDictionaryIdList();
    }
  }

  @Override
  public void setSelectedDirection(@Nullable Dictionary.Direction selectedDirection)
  {
    if (mSearchManager instanceof ParagonSearchManager)
    {
      ParagonSearchManager searchManager = (ParagonSearchManager) mSearchManager;
      searchManager.setSelectedDirection(selectedDirection);
    }
  }

  @Override
  public void restoreSearchState() {
    mIsScrollSelected = mSearchManager.restoreIsScrollSelected();
    setSearchText(mSearchManager.restoreSearchRequest());
  }

  @Override
  public SpannableString getHighLightLabel(ArticleItem item) {
    SpannableString label = new SpannableString(item.getLabel());
    if (mShowHighlighting)
    {
      for (WordReference word : item.getWordReferences())
      {
        BackgroundColorSpan span = new BackgroundColorSpan(getHighLightColor());
        label.setSpan(span, word.getStart(), word.getEnd(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
      }
    }
    return label;
  }

  @Override
  public SpannableString getHeadWord(ArticleItem item) {
    return new SpannableString(item.getFtsHeadword());
  }

  @Override
  public boolean launchTestMode( @NonNull Activity activity, @NonNull String text )
  {
    return mSearchManager.launchTestMode(activity, text);
  }

  int getHighLightColor() {
  	return Color.parseColor(highLightColor);
  }

  @Override
  public boolean IsScrollSelected() {
    return mIsScrollSelected;
  }

  @Override
  public void setContext( Context context )
  {
    mContext = context;
  }

  @Override
  public void openArticle( ArticleItem articleItem )
  {
    setActiveArticle(articleItem);
    mScreenOpener.showArticle(articleItem, getShowArticleOptions(), mContext);
  }

  @Nullable
  private ShowArticleOptions getShowArticleOptions()
  {
    ShowArticleOptions options = null;
    if ( !SearchType.SEARCH_TYPE_FTS.equals(mSelectedSearch) )
    {
      options = new ShowArticleOptions(SwipeMode.A_Z_OF_DICTIONARY);
    }
    return options;
  }

  @Override
  public void playSound( ArticleItem articleItem )
  {
    if (mSearchManager instanceof ParagonSearchManager)
    {
      ParagonSearchManager searchManager = (ParagonSearchManager) mSearchManager;
      searchManager.playSound(articleItem);
    }
  }

  @Override
  public boolean itemHasSound( ArticleItem articleItem )
  {
    if (mSearchManager instanceof ParagonSearchManager) {
      ParagonSearchManager searchManager = (ParagonSearchManager) mSearchManager;
      return searchManager.hasSound(articleItem);
    }
    return false;
  }

  @Override
  public void setSpecialSearchGroupExpanded( boolean value )
  {
    mSpecialSearchGroupExpanded = value;
  }

  @Override
  public boolean isSpecialSearchGroupExpanded()
  {
    return mSpecialSearchGroupExpanded;
  }

  @Override
  public void setExpandedSpecialSearchGroup( CollectionView<ArticleItem, GroupHeader> collectionView )
  {
    mExpandedSpecialSearchGroup = collectionView;
  }

  @Override
  public CollectionView<ArticleItem, GroupHeader> getExpandedSpecialSearchGroup()
  {
    return mExpandedSpecialSearchGroup;
  }

  @Override
  public void setExpandedGroupHeader( String specialSearchLabel)
  {
    mExpandedGroupHeader = specialSearchLabel;
  }

  @Override
  public String getExpandedGroupHeader()
  {
    return mExpandedGroupHeader;
  }

  @Override
  public void setFilterDictionaryId( Dictionary.DictionaryId dictionaryId )
  {
    mFilterDictionaryId = dictionaryId;
  }

  @Override
  public Dictionary.DictionaryId getFilterDictionaryId()
  {
    return mFilterDictionaryId;
  }

  @Override
  public void setEntryListFontSize( float entryListFontSize )
  {
    if (mEntryListFontSize != entryListFontSize)
    {
      mEntryListFontSize = entryListFontSize;
      for ( OnControllerFontSizeChangeListener listener : mOnControllerFontSizeChangeListeners)
      {
        listener.onControllerEntryListFontSizeChanged();
      }
    }
  }

  @Override
  public void setShowKeyboardEnabled( boolean enabled )
  {
    mShowKeyboard = enabled;
  }

  @Override
  public void setShowHighlightingEnabled( boolean enabled )
  {
    mShowHighlighting = enabled;
  }

  void onError( Exception exception )
  {
    for ( OnControllerErrorListener listener : mOnControllerErrorListeners)
    {
      listener.onControllerError(exception);
    }
  }

  @Override
  public boolean isFullDictionary() {
    DictionaryControllerAPI controller = DictionaryManagerHolder.getManager().createController(null);
    if (null != controller) {
      for (Dictionary dict : getDictionaries()) {
        if (dict.getId().equals(getSelectedDictionary())) {
          for (DictionaryComponent component : dict.getDictionaryComponents()) {
            if (!component.isDemo() && DictionaryComponent.Type.WORD_BASE.equals(component.getType())) {
              return controller.isDownloaded(component);
            }
          }
        }
      }
    }
    return false;
  }

  @Override
  public boolean isFtsNotAvailableInDictionary() {
    DictionaryManagerAPI dictionaryManager = DictionaryManagerHolder.getManager();
    DictionaryControllerAPI controller = dictionaryManager.createController(null);
    if (null != controller) {
      for (Dictionary dict : getDictionaries()) {
        if (dict.getId().equals(getSelectedDictionary())) {
          return dictionaryManager.isProductDemoFtsPromise() || dict.isIsDemoFts() || dict.isIsPromiseFtsInDemo();
        }
      }
    }
    return false;
  }

  @Override
  public boolean showHintDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager )
  {
    if ( mHintManager != null )
    {
      return mHintManager.showHintDialog(hintType, fragmentManager, null);
    }
    return false;
  }

  @Override
  public void buySelectedDictionary(Activity activity)
  {
    DictionaryManagerAPI dictionaryManager = DictionaryManagerHolder.getManager();

    DictionaryAndDirection dictionaryAndDirection = dictionaryManager.getDictionaryAndDirectionSelectedByUser();
    if(dictionaryAndDirection != null)
    {
      dictionaryManager.buy(activity, dictionaryAndDirection.getDictionaryId());
    }
  }

  @Override
  public void openScreen(ScreenType screenType)
  {
    mScreenOpener.openScreen(screenType);
  }

  @NonNull
  @Override
  public Observable<Boolean> getTopScreenOverlayStateObservable() {
    return mScreenOpener.getTopScreenOverlayStateObservable();
  }
}
