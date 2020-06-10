package com.paragon_software.article_manager;

import android.app.Activity;
import android.graphics.Bitmap;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.collections.DictionariesCollectionView;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.dictionary_manager.Language;
import com.paragon_software.dictionary_manager.filter.FilterTypeSimple;
import com.paragon_software.dictionary_manager.filter.IFilterFactory;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.Disposable;

public class ShareController implements ShareControllerAPI
{
  private boolean mInsideChangeStateMethod = false;
  @Nullable
  private String mInitiallyQuery;
  @NonNull
  private List< Notifier > mNotifiers = new ArrayList<>();
  @NonNull
  private final ArticleControllerAPI    mArticleController;
  @NonNull
  private final ArticleManagerAPI       mArticleManager;
  @NonNull
  private final DictionaryControllerAPI mDictionaryController;
  @NonNull
  private final DictionaryManagerAPI    mDictionaryManager;
  @Nullable
  private ArticleItem mCurrentArticleItem;

  @NonNull
  private DictionariesCollectionView                                                             mDictionariesCollectionView;
  @NonNull
  private CollectionView< CollectionView< ArticleItem, SubstringInfo >, Void > mWordsCollectionView;

  private VisibilityState mDictionariesFetchUI            = VisibilityState.gone;
  private VisibilityState mDictionariesMissingPurchasesUI = VisibilityState.gone;
  private VisibilityState mDictionariesMissingFullBaseUI  = VisibilityState.gone;
  private VisibilityState mSearchUIVisibility;

  private final Disposable mUpdateDictionaryAndDirectionDisposable;

  private CollectionView.OnSelectionChange onDictionarySelectionListener = new CollectionView.OnSelectionChange()
  {
    @Override
    public void onSelectionChanged()
    {
      updateWordsCollectionView();
    }
  };

  private ShareController( @NonNull ArticleManagerAPI articleManager, @NonNull String controllerId,
                           @NonNull DictionaryControllerAPI dictionaryController, @NonNull DictionaryManagerAPI _dictionaryManager )
  {

    this.mArticleManager = articleManager;
    this.mDictionaryController = dictionaryController;
    this.mDictionaryManager = _dictionaryManager;
    this.mArticleController = mArticleManager.getArticleController(controllerId);
    this.mArticleController.setDictionaryIconVisibility(VisibilityState.gone);
    this.mArticleController.setDictionaryTitleVisibility(VisibilityState.gone);
    this.mDictionariesCollectionView = new DictionariesCollectionView();
    this.mSearchUIVisibility = this.mArticleController.getSearchUIVisibility();
    Dictionary dictionary = getSelectedDictionary();
    this.mWordsCollectionView = mArticleManager.getWords(mInitiallyQuery, dictionary, getSelectedDirection(dictionary));
    this.mUpdateDictionaryAndDirectionDisposable = this.mDictionaryManager.getDictionaryAndDirectionChangeObservable()
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe(aBoolean -> updateDictionaries(), Throwable::printStackTrace);
  }

  static ShareControllerAPI create( @NonNull ArticleManagerAPI _articleManager, @NonNull String _controllerId,
                                    @Nullable DictionaryManagerAPI _dictionaryManager )
  {

    ShareController shareController;
    DictionaryControllerAPI dictionaryController;
    if ( _dictionaryManager == null )
    {
      throw new IllegalArgumentException("Can not initialize Share controller if DictionaryManager is null");
    }
    dictionaryController = _dictionaryManager.createController(_controllerId);
    if ( null == dictionaryController )
    {
      throw new IllegalArgumentException("Can not initialize Share controller if DictionaryController is null");
    }

    shareController = new ShareController(_articleManager, _controllerId, dictionaryController, _dictionaryManager);
    shareController.registerToDictionaryController();
    shareController.mArticleController.registerNotifier(shareController.new OnSearchUIVisibilityListener());
    shareController.updateDictionaries();
    shareController.listenDictionarySelection();
    return shareController;
  }

  private void listenDictionarySelection()
  {
    this.mDictionariesCollectionView.registerListener(onDictionarySelectionListener);
  }

  private void registerToDictionaryController()
  {
    IFilterFactory filterFactory = mDictionaryManager.getFilterFactory();
    DictionaryFilter dictionaryFilter = filterFactory.createByType(FilterTypeSimple.SHARE_AND_API);
    mDictionaryController.installFilter(dictionaryFilter);
    mDictionaryManager.registerDictionaryListObserver(new DictionaryListChangedListener());
  }

  private void updateDictionaries()
  {
    this.mDictionariesCollectionView.update(mDictionaryController.getDictionaries(), mDictionaryManager.getDictionaryAndDirectionSelectedByUser());
  }

  @Override
  public void showDictionariesFetchUI()
  {
    if ( mDictionaryManager.getDictionaries().isEmpty() )
    {
      changeState(new Runnable()
      {
        @Override
        public void run()
        {
          mDictionariesFetchUI = VisibilityState.enabled;
        }
      });
    }
    else
    {
      onDictionaryListChangedImpl();
    }
  }

  @Override
  public void setInitiallyQuery( String query )
  {
    this.mInitiallyQuery = query;
    updateWordsCollectionView();
  }

  @NonNull
  @Override
  public CollectionView< Dictionary, Void > getDictionaries()
  {
    return mDictionariesCollectionView;
  }

  @NonNull
  @Override
  public CollectionView< CollectionView< ArticleItem, SubstringInfo >, Void > getWords()
  {
    return mWordsCollectionView;
  }

  @Override
  public void selectDictionary( @NonNull final Dictionary.DictionaryId dictionaryId )
  {
    changeState(new Runnable()
    {
      @Override
      public void run()
      {
        mDictionariesCollectionView.updateSelection(dictionaryId);
      }
    });
  }

  @Override
  public void selectWord( int wordIndex ) throws IndexOutOfBoundsException
  {
    CollectionView< ArticleItem, SubstringInfo > tabs = updateTabsCollectionViewSelection(wordIndex, 0);
    mWordsCollectionView.updateSelection(wordIndex);
    if ( null != tabs )
    {
      translateArticleBySelectedTab(tabs);
    }
  }

  @SuppressWarnings( "SameParameterValue" )
  @Nullable
  private CollectionView< ArticleItem, SubstringInfo > updateTabsCollectionViewSelection( int wordIndex, int tabSelection )
  {
    if ( wordIndex < mWordsCollectionView.getCount() )
    {
      CollectionView< ArticleItem, SubstringInfo > tabs = mWordsCollectionView.getItem(wordIndex);
      if ( null != tabs )
      {
        tabs.updateSelection(tabSelection);
      }
      return tabs;
    }
    return null;
  }

  @Override
  public void selectTab( int tabIndex ) throws IndexOutOfBoundsException
  {
    CollectionView< ArticleItem, SubstringInfo > tabsCollectionView = getTabsCollectionView();
    if (tabsCollectionView != null)
    {
      tabsCollectionView.updateSelection(tabIndex);
      translateArticleBySelectedTab(tabsCollectionView);
    }
  }

  private void translateArticleBySelectedTab( @NonNull
      CollectionView< ArticleItem, SubstringInfo > tabsCollectionView )
  {
    int tabSelection = tabsCollectionView.getSelection();
    if ((0 <= tabSelection) && (tabSelection < tabsCollectionView.getCount()))
    {
      ArticleItem articleItem = tabsCollectionView.getItem(tabSelection);
      if (null != articleItem)
      {
        translate(articleItem);
      }
    }
  }

  private void translate(@Nullable ArticleItem articleItem )
  {
    if (null != articleItem)
    {
      mArticleController.nextTranslation(articleItem);
      mCurrentArticleItem = articleItem;
    }
  }

  @Nullable
  private CollectionView< ArticleItem, SubstringInfo > getTabsCollectionView()
  {
    int selection = mWordsCollectionView.getSelection();
    if ((0 <= selection) && (selection < mWordsCollectionView.getCount()))
    {
      return mWordsCollectionView.getItem(selection);
    }
    return null;
  }

  @NonNull
  @Override
  public DirectionButtonViewInfo getSwitchDirectionButtonViewInfo()
  {
    ButtonState buttonState = new ButtonState(VisibilityState.gone, CheckState.uncheckable);
    Bitmap bitmap = null;
    Language langFrom = null;
    Language langTo = null;

    Dictionary dictionary = getSelectedDictionary();
    if ( null != dictionary )
    {
      Dictionary.Direction direction = getSelectedDirection(dictionary);
      if ( null != direction && null != direction.getIcon() )
      {
        IDictionaryIcon icon = direction.getIcon();
        bitmap = icon.getBitmap();
        langFrom = Language.fromCode(direction.getLanguageFrom());
        langTo = Language.fromCode(direction.getLanguageTo());

        VisibilityState visibilityState =
            direction.getLanguageFrom() == direction.getLanguageTo() ? VisibilityState.disabled
                                                                       : VisibilityState.enabled;
        buttonState = new ButtonState(visibilityState, CheckState.uncheckable);
        DictionaryAndDirection dnd = new DictionaryAndDirection(dictionary.getId(), direction);
        mDictionaryManager.setDictionaryAndDirectionSelectedByUser(dnd);
      }
    }
    return new DirectionButtonViewInfo(buttonState, bitmap, langFrom, langTo);
  }

  @NonNull
  @Override
  public VisibilityState getDictionariesSpinnerVisibility()
  {
    VisibilityState dictionariesSpinnerVisibility;
    if ( getDictionaries().getCount() < 1 )
    {
      dictionariesSpinnerVisibility = VisibilityState.gone;
    }
    else
    {
      boolean spinnerVisible = VisibilityState.gone.equals(mSearchUIVisibility);
      dictionariesSpinnerVisibility = spinnerVisible ? VisibilityState.enabled : VisibilityState.gone;
    }
    return dictionariesSpinnerVisibility;
  }

  @NonNull
  @Override
  public VisibilityState getDictionariesFetchUIVisibility()
  {
    return mDictionariesFetchUI;
  }

  @NonNull
  @Override
  public VisibilityState getDictionariesMissingPurchasesUIVisibility()
  {
    return mDictionariesMissingPurchasesUI;
  }

  @NonNull
  @Override
  public VisibilityState getDictionariesMissingFullBaseUIVisibility()
  {
    return mDictionariesMissingFullBaseUI;
  }

  @Override
  public void switchDirection()
  {
    changeState(new Runnable()
    {
      @Override
      public void run()
      {
        Dictionary dictionary = getSelectedDictionary();
        if ( null != dictionary )
        {
          Dictionary.Direction nextDirection = findNextDirection(dictionary);
          if ( null != nextDirection )
          {
            DictionaryAndDirection dnd = new DictionaryAndDirection(dictionary.getId(), nextDirection);
            mDictionaryManager.setDictionaryAndDirectionSelectedByUser(dnd);
          }
        }
      }
    });
    updateWordsCollectionView();
  }

  private Dictionary.Direction findNextDirection( Dictionary dictionary )
  {
    Dictionary.Direction nextDirection = null;
    Dictionary.Direction currentDirection = getSelectedDirection(dictionary);
    for ( Dictionary.Direction direction : dictionary.getDirections() )
    {
      if ( null != direction && !direction.equals(currentDirection) )
      {
        nextDirection = direction;
        break;
      }
    }
    return nextDirection;
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if ( !mNotifiers.contains(notifier) )
    {
      mNotifiers.add(notifier);
    }
  }

  @Override
  public void unregisterNotifier( @NonNull Notifier notifier )
  {
    mNotifiers.remove(notifier);
  }

  @Override
  public void openDictionarySearch( Activity activity )
  {
    if ( null != mCurrentArticleItem )
    {
      mDictionaryManager.openDictionaryForSearch(activity, mCurrentArticleItem.getDictId(), mCurrentArticleItem.getDirection(), mCurrentArticleItem.getShowVariantText());
    }
    else
    {
      Dictionary dictionary = getSelectedDictionary();
      Dictionary.Direction direction = getSelectedDirection(dictionary);
      if ( null != dictionary && null != direction && null != mInitiallyQuery )
      {
        mDictionaryManager.openDictionaryForSearch(activity, dictionary.getId(), direction, mInitiallyQuery);
      }
    }
  }

  @Override
  public void hideDictionariesFetchUI()
  {
    changeState(new Runnable()
    {
      @Override
      public void run()
      {
        mDictionariesFetchUI = VisibilityState.gone;
      }
    });
  }

  @Override
  public void hideMissingFullBaseUI()
  {
    changeState(new Runnable()
    {
      @Override
      public void run()
      {
        mDictionariesMissingFullBaseUI = VisibilityState.gone;
      }
    });
  }

  @Override
  public void hideMissingPurchasesUI()
  {
    changeState(new Runnable()
    {
      @Override
      public void run()
      {
        mDictionariesMissingPurchasesUI = VisibilityState.gone;
      }
    });
  }

  private synchronized void changeState( @NonNull Runnable op )
  {
    DirectionButtonViewInfo directionButtonViewInfo = null;
    VisibilityState dictionariesSpinnerVisibility = null;
    VisibilityState dictionariesFetchUIVisibility = null;
    VisibilityState dictionariesMissingFullBaseUIVisibility = null;
    VisibilityState dictionariesMissingPurchasesUIVisibility = null;
    if ( !mInsideChangeStateMethod )
    {
      directionButtonViewInfo = getSwitchDirectionButtonViewInfo();
      dictionariesSpinnerVisibility = getDictionariesSpinnerVisibility();
      dictionariesFetchUIVisibility = getDictionariesFetchUIVisibility();
      dictionariesMissingFullBaseUIVisibility = getDictionariesMissingFullBaseUIVisibility();
      dictionariesMissingPurchasesUIVisibility = getDictionariesMissingPurchasesUIVisibility();
    }

    boolean prevInsideValue = mInsideChangeStateMethod;
    mInsideChangeStateMethod = true;
    op.run();
    mInsideChangeStateMethod = prevInsideValue;

    if ( !mInsideChangeStateMethod )
    {
      if ( !directionButtonViewInfo.equals(getSwitchDirectionButtonViewInfo()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnSwitchDirectionButtonViewStateChangedListener )
          {
            ( (OnSwitchDirectionButtonViewStateChangedListener) notifier ).onSwitchDirectionButtonViewStateChanged();
          }
        }
      }
      if ( dictionariesSpinnerVisibility != getDictionariesSpinnerVisibility() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnDictionariesSpinnerVisibilityChangedListener )
          {
            ( (OnDictionariesSpinnerVisibilityChangedListener) notifier ).onDictionariesSpinnerVisibilityChanged();
          }
        }
      }
      if ( dictionariesFetchUIVisibility != getDictionariesFetchUIVisibility() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnDictionariesFetchUIVisibilityChangedListener )
          {
            ( (OnDictionariesFetchUIVisibilityChangedListener) notifier ).onDictionariesFetchUIVisibilityChanged();
          }
        }
      }
      if ( dictionariesMissingFullBaseUIVisibility != getDictionariesMissingFullBaseUIVisibility() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnDictionariesMissingFullBaseUIVisibilityListener )
          {
            ( (OnDictionariesMissingFullBaseUIVisibilityListener) notifier ).onDictionariesMissingFullBaseUIVisibility();
          }
        }
      }
      if ( dictionariesMissingPurchasesUIVisibility != getDictionariesMissingPurchasesUIVisibility() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnDictionariesMissingPurchasesUIVisibilityListener )
          {
            ( (OnDictionariesMissingPurchasesUIVisibilityListener) notifier ).onDictionariesMissingPurchasesUIVisibility();
          }
        }
      }
    }
  }

  class DictionaryListChangedListener implements DictionaryManagerAPI.IDictionaryListObserver
  {

    @Override
    public void onDictionaryListChanged()
    {
      onDictionaryListChangedImpl();
    }
  }

  private void onDictionaryListChangedImpl()
  {
    if ( !mDictionaryManager.getDictionaries().isEmpty() )
    {
      hideDictionariesFetchUI();
    }

    if( mDictionaryManager.isDictionariesInit() )
    {
      updateErrorUIVisibility();
    }

    if( mDictionaryController.getDictionaries().length > 0 )
    {
      changeState(new Runnable()
      {
        @Override
        public void run()
        {
          updateDictionaries();
        }
      });
    }
  }

  private void updateErrorUIVisibility()
  {
    changeState(new Runnable()
    {
      @Override
      public void run()
      {
        Collection< Dictionary > dictionaries = mDictionaryManager.getDictionaries();
        Dictionary[] dictionariesForShareAndApi = mDictionaryController.getDictionaries();
        if ( !dictionaries.isEmpty() && dictionariesForShareAndApi.length <= 0 )
        {
          if (!canFullBaseBeUsed(dictionaries))
          {
            mDictionariesMissingPurchasesUI = VisibilityState.enabled;
            mDictionariesMissingFullBaseUI = VisibilityState.gone;
          }
          else
          {
            mDictionariesMissingFullBaseUI = VisibilityState.enabled;
            mDictionariesMissingPurchasesUI = VisibilityState.gone;
          }
        }
        else
        {
          mDictionariesMissingFullBaseUI = VisibilityState.gone;
          mDictionariesMissingPurchasesUI = VisibilityState.gone;
        }
      }
    });
  }

  private boolean canFullBaseBeUsed( Collection< Dictionary > dictionaries )
  {
    return false;
  }

  @Nullable
  private Dictionary.Direction getSelectedDirection( @Nullable Dictionary dictionary )
  {
    Dictionary.Direction direction = null;
    if ( null != dictionary )
    {
      DictionaryAndDirection selectedByUser = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
      if ( null != selectedByUser && dictionary.getId().equals(selectedByUser.getDictionaryId()) )
      {
        direction = selectedByUser.getDirection();
      }

      Collection< Dictionary.Direction > directions = dictionary.getDirections();
      if ( null == direction && null != directions )
      {
        direction = directions.iterator().next();
      }
    }
    return direction;
  }

  class OnSearchUIVisibilityListener implements OnSearchUIVisibilityChangedListener
  {

    @Override
    public void onSearchUIVisibilityChanged()
    {
      changeState(new Runnable()
      {
        @Override
        public void run()
        {
          mSearchUIVisibility = mArticleController.getSearchUIVisibility();
        }
      });
    }
  }

  private Dictionary getSelectedDictionary()
  {
    Dictionary dictionary = null;
    int selection = getDictionaries().getSelection();
    if ( 0 <= selection )
    {
      dictionary = getDictionaries().getItem(selection);
    }
    return dictionary;
  }

  private void updateWordsCollectionView()
  {
    final int defaultSelectedWordIndex = 0;
    final int defaultSelectedTabIndex = 0;
    Dictionary dictionary = getSelectedDictionary();
    mWordsCollectionView = mArticleManager.getWords(mInitiallyQuery, dictionary, getSelectedDirection(dictionary));
    int selection = mWordsCollectionView.getSelection();
    if ( selection < 0)
    {
      mWordsCollectionView.updateSelection(defaultSelectedWordIndex);
      mWordsCollectionView.registerListener(new CollectionView.OnItemRangeChanged()
      {
        @Override
        public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int startPosition, int itemCount )
        {
          if ( CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED.equals(type) && itemCount > 0 )
          {
            mWordsCollectionView.unregisterListener(this);
            CollectionView< ArticleItem, SubstringInfo > tabsCollectionView = getTabsCollectionView();
            if ( tabsCollectionView != null && tabsCollectionView.getSelection() < 0 )
            {
              tabsCollectionView.updateSelection(defaultSelectedTabIndex);
              translateArticleBySelectedTab(tabsCollectionView);
            }
          }
        }
      });

    }
  }
}
