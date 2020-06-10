package com.paragon_software.dictionary_manager;
/*
 * dictionary manager paragon
 *
 *  Created on: 19.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import android.os.Parcelable;
import android.text.TextUtils;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.downloader.DownloadLibraryBuilder;
import com.paragon_software.dictionary_manager.errors.ErrorType;
import com.paragon_software.dictionary_manager.errors.ErrorsUtils;
import com.paragon_software.dictionary_manager.errors.OperationType;
import com.paragon_software.dictionary_manager.exceptions.InitDictionaryManagerException;
import com.paragon_software.dictionary_manager.filter.FilterFactory;
import com.paragon_software.dictionary_manager.filter.IFilterFactory;
import com.paragon_software.dictionary_manager.local_dictionary_library.ILocalDictionaryLibrary;
import com.paragon_software.dictionary_manager.local_dictionary_library.LocalDictionaryLibrary;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.InitDictionaryLibraryException;
import com.paragon_software.license_manager_api.LicenseFeature;
import com.paragon_software.license_manager_api.LicenseManager;
import com.paragon_software.license_manager_api.exceptions.ActivateException;
import com.paragon_software.license_manager_api.exceptions.ItemAlreadyOwnedException;
import com.paragon_software.license_manager_api.exceptions.ItemUnavailableException;
import com.paragon_software.license_manager_api.exceptions.UserAccountManagementException;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.trial_manager.TrialComplexAPI;
import com.paragon_software.trial_manager.TrialManagerAPI;
import com.paragon_software.user_core_manager.ProductLicense;
import com.paragon_software.user_core_manager.UserCoreManagerAPI;
import com.paragon_software.utils_slovoed.network.ConnectivityReceiver;
import com.paragon_software.utils_slovoed.pdahpc.Catalog;
import com.paragon_software.utils_slovoed.pdahpc.PDAHPCDataParser;
import com.paragon_software.utils_slovoed.static_consts.BundleKeys;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.FutureTask;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import androidx.fragment.app.FragmentManager;
import io.reactivex.Observable;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;
import io.reactivex.subjects.BehaviorSubject;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

@SuppressWarnings( { "WeakerAccess", "unused" } )
public final class ParagonDictionaryManager extends DictionaryManagerAPI
                                            implements TrialController.FeatureNameFinder,
                                                       ConnectivityReceiver.OnConnected
{
  private static final String DICTIONARY_AND_DIRECTION_SELECTED_BY_USER_KEY = "com.paragon_software.dictionary_manager.ParagonDictionaryManager.DICTIONARY_AND_DIRECTION_SELECTED_BY_USER_KEY";
  @VisibleForTesting
  protected static final String IS_SELECTED_ALL_DICTIONARIES = "com.paragon_software.dictionary_manager.ParagonDictionaryManager.IS_SELECTED_ALL_DICTIONARIES";

  private final Context mApplicationContext;
  @NonNull
  private final ThreadPoolExecutor mUpdateThreadPool =
      new ThreadPoolExecutor(1, 1, 60, TimeUnit.SECONDS, new LinkedBlockingQueue< Runnable >());
  @Nullable
  private final LicenseManager     mLicenseManager;
  @NonNull
  private final SettingsManagerAPI mSettingsManager;
  @VisibleForTesting
  @NonNull
  protected final LicenseManagerListener mLicenseManagerListener = new LicenseManagerListener();
  @NonNull
  private final TrialManagerListener mTrialManagerListener = new TrialManagerListener();
  @NonNull
  private final DictionariesInitializer mInitializer;
  @NonNull
  private final UIData          mUIData          = new UIData();
  @NonNull
  private final ILocalDictionaryLibrary    mDictionaryLibrary;
  @NonNull
  private final IFilterFactory             mFilterFactory;
  @NonNull
  private final DictionaryLocationResolver componentLocationResolver;
  @NonNull
  final private ArrayList< IDictionaryListObserver > mDictionaryListObservers = new ArrayList<>();
  @NonNull
  final private ArrayList< IErrorObserver > mErrorObservers = new ArrayList<>();
  private final DownloadLibraryBuilder mDownloadLibraryBuilder;
  @NonNull
  Map< String, DictionaryControllerAPI > controllers = new HashMap<>();
  @NonNull
  private CatalogItem[]                                           mPdahpcItems           = new CatalogItem[0];
  @NonNull
  private Map< Dictionary.DictionaryId, DictionaryPrice >         mInAppPrices           = new HashMap<>();
  @NonNull
  private Map< Dictionary.DictionaryId, List< DictionaryPrice > > mSubscriptions         = new HashMap<>();
  @NonNull
  private Map< Dictionary.DictionaryId, SubscriptionPurchase >    mSubscriptionPurchases = new HashMap<>();
  @NonNull
  private List< Dictionary >                                      mAvailableDictionaries = new ArrayList<>();
  @NonNull
  private List<DictionaryPack>                                    mAvailableDictionaryPacks = new ArrayList<>();
  @VisibleForTesting
  @Nullable
  protected final TrialManagerAPI   mTrialManager;
  @Nullable
  private final UserCoreManagerAPI mUserCoreManager;
  @NonNull
  private final ScreenOpenerAPI    mScreenOpener;
  @Nullable
  private DictionaryAndDirection mDictionaryAndDirectionSelectedByUser = null;
  private PublishSubject<Boolean> mDictionaryAndDirectionChangeSubject = PublishSubject.create();
  private Boolean mIsSelectAll;
  @Nullable
  private DictionaryAndDirection mDefaultDictionaryAndDirection = null;
  @NonNull
  private Collection<ProductLicense> mUserCoreLicenses = Collections.emptySet();
  @NonNull
  private final Observer<List<ProductLicense>> mUserCoreLicensesObserver = new UserCoreLicensesObserver();

  private boolean mIsDictionariesInit = false;

  private final BehaviorSubject< Boolean > updateStateSubject = BehaviorSubject.createDefault(false);
  private final Subject< Boolean > updateTrialStateSubject = PublishSubject.create();
  private final Subject< Boolean > updateFinishedAfterLicenseChangeSubject = PublishSubject.create();

  /**
   * Stores state trace of the dictionary manager at the moment when it notified observers about its changes.
   * It is used to check is the dictionary manager state really was changed from the last notification or not. If
   * state was not changed then it skips notification in {@link #notifyObservers() notifyObservers} method for list
   * of {@link IDictionaryListObserver} observers.
   */
  private String previousNotificationStateTrace = "";

  public ParagonDictionaryManager( @NonNull final Context context, @NonNull final LicenseManager licenseManager,
                                   @NonNull IDictionaryManagerFactory factory,
                                   @NonNull SettingsManagerAPI settingsManager, @NonNull SearchEngineAPI searchEngine,
                                   @NonNull Dictionary.DictionaryId[] dictionaryId, @NonNull TrialManagerAPI.Builder trialBuilder,
                                   @NonNull DownloadLibraryBuilder downloadLibraryBuilder,
                                   @Nullable UserCoreManagerAPI userCoreManager,
                                   @NonNull ScreenOpenerAPI screenOpener )
      throws InitDictionaryManagerException
  {
    this(context, licenseManager, factory, settingsManager, searchEngine, dictionaryId, new IDictionaryListObserver[0],
         trialBuilder, downloadLibraryBuilder, userCoreManager, screenOpener);
  }

  protected ParagonDictionaryManager( @NonNull final Context context, @NonNull final LicenseManager licenseManager, @NonNull IDictionaryManagerFactory factory,
                                      @NonNull SettingsManagerAPI settingsManager, @NonNull SearchEngineAPI searchEngine,
                                      @NonNull Dictionary.DictionaryId[] dictionaryId, @NonNull IDictionaryListObserver[] observers,
                                      @Nullable TrialManagerAPI.Builder trialBuilder,
                                      @NonNull DownloadLibraryBuilder downloadLibraryBuilder,
                                      @Nullable UserCoreManagerAPI userCoreManager,
                                      @NonNull ScreenOpenerAPI screenOpener) throws InitDictionaryManagerException
  {
    try
    {
      synchronized ( this )
      {
        mApplicationContext = context.getApplicationContext();
        mPdahpcItems = getCatalogItemsFromFactory(context, factory, dictionaryId);
        mSettingsManager = settingsManager;
        mDictionaryLibrary = new LocalDictionaryLibrary(context);
        mLicenseManager = licenseManager;
        if ( mLicenseManager != null )
          mLicenseManager.registerNotifier(mLicenseManagerListener);
        registerDictionaryListObservers(observers);
        searchEngine.registerDictionaryManager(this);
        mFilterFactory = new FilterFactory(this, mSettingsManager, searchEngine);
        componentLocationResolver = new DictionaryLocationResolver(mDictionaryLibrary, context);
        mInitializer = new DictionariesInitializer(mLicenseManager, componentLocationResolver);
        if ( trialBuilder != null ) {
          trialBuilder.setPurchaseInfoGetter(new LicensePurchaseInfoGetter(mLicenseManager));
          mTrialManager = trialBuilder.build(context);
          mTrialManager.registerOnUpdateListener(mTrialManagerListener);
        } else {
          mTrialManager = null;
        }
        this.mDownloadLibraryBuilder = downloadLibraryBuilder;
        mUserCoreManager = userCoreManager;
        mScreenOpener = screenOpener;
        if (mUserCoreManager != null) {
          mUserCoreManager.getProductLicenses().subscribe(mUserCoreLicensesObserver);
        }
        ConnectivityReceiver.addOnConnectedListener(context, this);
      }
      updateDictionariesInBackground(new Runnable()
      {
        @Override
        public void run()
        {
          mIsDictionariesInit = true;
        }
      });
    }
    catch ( InitDictionaryLibraryException e )
    {
      throw new InitDictionaryManagerException(e);
    }
    (new Thread(() -> LanguageStringsImpl.init(mApplicationContext))).start();
  }

  void registerDictionaryListObservers( @NonNull IDictionaryListObserver[] dictionaryListObservers )
  {
    synchronized ( mDictionaryListObservers )
    {
      for ( IDictionaryListObserver observer : dictionaryListObservers )
      {
        if ( !mDictionaryListObservers.contains(observer) )
        {
          mDictionaryListObservers.add(observer);
        }
      }
    }
  }

  private void updateDictionariesSynchronous(@Nullable Runnable runnable) {
    updateStateSubject.onNext(true);
    CatalogItem[] pdahpcItemsClone = cloneCatalogItems(mPdahpcItems);
    Map<Dictionary.DictionaryId, DictionaryPrice> inAppPrices = mInitializer.prepareInAppPrices(pdahpcItemsClone);
    Map<Dictionary.DictionaryId, List<DictionaryPrice>> subscriptions =
            mInitializer.prepareSubscriptionPrices(pdahpcItemsClone);
    Map<Dictionary.DictionaryId, SubscriptionPurchase> subscriptionPurchases =
            mInitializer.prepareSubscriptionPurchases(pdahpcItemsClone);
    updateUserCoreAndTrialStatus(pdahpcItemsClone);
    updateDictionariesUIThread(pdahpcItemsClone, subscriptions, subscriptionPurchases, inAppPrices,runnable);
  }

  private static CatalogItem[] cloneCatalogItems(CatalogItem[] catalogItems )
  {
    CatalogItem[] clonedItems = new CatalogItem[catalogItems.length];
    for (int i = 0; i < catalogItems.length; i++ )
    {
      clonedItems[i] = catalogItems[i].cloneItem();
    }
    return clonedItems;
  }

  @VisibleForTesting
  protected void replaceSubscriptionPurchases( Map< Dictionary.DictionaryId, SubscriptionPurchase > purchases )
  {
    synchronized ( this )
    {
      mSubscriptionPurchases = new HashMap<>(purchases);
    }
  }

  private void replaceInAppPrices( @NonNull final Map< Dictionary.DictionaryId, DictionaryPrice > inAppPrices )
  {
    synchronized ( this )
    {
      mInAppPrices = new HashMap<>(inAppPrices);
    }
  }

  private void replaceSubscriptions( Map< Dictionary.DictionaryId, List< DictionaryPrice > > subscriptions )
  {
    synchronized ( this )
    {
      mSubscriptions = new HashMap<>(subscriptions);
    }
  }

  private void updateUserCoreAndTrialStatus(CatalogItem[] catalogItems) {
      Date now = new Date();
      for(CatalogItem catalogItem : catalogItems)
          if(Dictionary.DICTIONARY_STATUS.DEMO.equals(catalogItem.getStatus())) {
              FeatureName featureName = catalogItem.getPurchaseFeatureName();
              if((!Dictionary.DICTIONARY_STATUS.DEMO.equals(catalogItem.getStatus())) && (catalogItem instanceof Dictionary)) {
                Dictionary dictionary = (Dictionary) catalogItem;
                mInitializer.assignLocationToDictionary(dictionary, false);
                if (null == dictionary.getDictionaryLocation())
                    mInitializer.assignLocationToDictionary(dictionary, true);
              }
          }
  }

  private boolean isUserCoreLicenseActive(String id, Date now) {
      boolean res = false;
      for(ProductLicense license : mUserCoreLicenses)
          if (license.getProductId().equals(id)) {
              Date expirationDate = license.getEndDate();
              if(expirationDate != null)
                  res = expirationDate.after(now);
              break;
          }
      return res;
  }

  private void updateDictionariesUIThread( final CatalogItem[] pdahpcItems,
                                           final Map<Dictionary.DictionaryId, List<DictionaryPrice>> subscriptions,
                                           final Map<Dictionary.DictionaryId, SubscriptionPurchase> subscriptionPurchases,
                                           final Map<Dictionary.DictionaryId, DictionaryPrice> inAppPrices,
                                           @Nullable final Runnable runnable)
  {
    getUiHandler().post(new Runnable()
    {
      @Override
      public void run()
      {
        mPdahpcItems = pdahpcItems;
        replaceSubscriptions(subscriptions);
        replaceSubscriptionPurchases(subscriptionPurchases);
        replaceInAppPrices(inAppPrices);

        Set<Dictionary.DictionaryId> knownDictionaryIds = new HashSet<>();
        knownDictionaryIds.addAll(subscriptions.keySet());
        knownDictionaryIds.addAll(subscriptionPurchases.keySet());
        knownDictionaryIds.addAll(inAppPrices.keySet());
        replaceAvailableCatalogItems(pdahpcItems, knownDictionaryIds);
        if(runnable != null)
        {
            runnable.run();
        }
        notifyObservers();
      }
    });
  }

  private void replaceAvailableCatalogItems(CatalogItem[] catalogItems,
                                            Set< Dictionary.DictionaryId > knownDictionaryIds )
  {
    synchronized (this)
    {
      mAvailableDictionaries = new ArrayList<>(catalogItems.length);
      mAvailableDictionaryPacks = new ArrayList<>(catalogItems.length);
      // knownDictionaryIds is empty if licenseManager cant obtain billing information.
      // Without billing information all dictionaries is available
      for (CatalogItem catalogItem : catalogItems) {
        boolean alreadyPurchased = false;
        boolean hasPrice = knownDictionaryIds.contains(catalogItem.getId()) || knownDictionaryIds.isEmpty();
        boolean onSale = !catalogItem.isRemovedFromSale();
        if (alreadyPurchased || (hasPrice && onSale))
          if (catalogItem instanceof Dictionary)
            mAvailableDictionaries.add((Dictionary) catalogItem);
          else if ((catalogItem instanceof DictionaryPack) && ((DictionaryPack) catalogItem).checkAvailabilityCondition(catalogItems))
            mAvailableDictionaryPacks.add((DictionaryPack) catalogItem);
      }
      sortPurchasedOnTop(mAvailableDictionaries);
    }
  }

  private static void sortPurchasedOnTop( List< Dictionary > list )
  {
    Comparator< ? super Dictionary > comparator = new Comparator< Dictionary >()
    {
      @Override
      public int compare( Dictionary d1, Dictionary d2 )
      {
//        int left = d1.getStatus().isPurchased() ? 0 : 1;
//        int right = d2.getStatus().isPurchased() ? 0 : 1;
//        return left - right;
        return 0;
      }
    };
    Collections.sort(list, comparator);
  }

  @SuppressWarnings( "WeakerAccess" )
  @VisibleForTesting
  void notifyObservers()
  {
    updateStateSubject.onNext(false);
    synchronized ( mDictionaryListObservers )
    {
      String stateTrace = this.toString();
      if ( !previousNotificationStateTrace.equals(stateTrace) )
      {
        for ( IDictionaryListObserver observer : new ArrayList<>(mDictionaryListObservers) )
        {
          observer.onDictionaryListChanged();
        }
        previousNotificationStateTrace = stateTrace;
      }
    }
  }

  @VisibleForTesting
  ParagonDictionaryManager( @NonNull final Context context, @NonNull final LicenseManager licenseManager,
                            @NonNull IDictionaryManagerFactory factory, @NonNull SettingsManagerAPI settingsManager,
                            @NonNull SearchEngineAPI searchEngine, @NonNull Dictionary.DictionaryId[] dictionaryId,
                            @NonNull IDictionaryListObserver[] observers, @NonNull ILocalDictionaryLibrary library,
                            @Nullable TrialManagerAPI.Builder trialBuilder,
                            DownloadLibraryBuilder downloadLibraryBuilder,
                            @Nullable UserCoreManagerAPI userCoreManager,
                            @NonNull ScreenOpenerAPI screenOpener)
  {
    synchronized ( this )
    {
      mApplicationContext = context.getApplicationContext();
      mPdahpcItems = getCatalogItemsFromFactory(context, factory, dictionaryId);
      mSettingsManager = settingsManager;
      mDictionaryLibrary = library;
      mLicenseManager = licenseManager;
      if ( mLicenseManager != null )
        mLicenseManager.registerNotifier(mLicenseManagerListener);
      registerDictionaryListObservers(observers);
      mFilterFactory = new FilterFactory(this, mSettingsManager, searchEngine);
      componentLocationResolver = new DictionaryLocationResolver(mDictionaryLibrary, context);
      mInitializer = new DictionariesInitializer(mLicenseManager, componentLocationResolver);
      if ( trialBuilder != null) {
        trialBuilder.setPurchaseInfoGetter(new LicensePurchaseInfoGetter(mLicenseManager));
        mTrialManager = trialBuilder.build(context);
        mTrialManager.registerOnUpdateListener(mTrialManagerListener);
      } else {
        mTrialManager = null;
      }
      mDownloadLibraryBuilder = downloadLibraryBuilder;
      mUserCoreManager = userCoreManager;
      mScreenOpener = screenOpener;
      if (mUserCoreManager != null) {
        mUserCoreManager.getProductLicenses().subscribe(mUserCoreLicensesObserver);
      }
    }
    updateDictionariesInBackground(() -> mIsDictionariesInit = true);
    (new Thread(() -> LanguageStringsImpl.init(mApplicationContext))).start();
  }

  @Override
  public void registerDictionaryManagerUI( @Nullable Class buyActivity, @Nullable Class mainActivity)
  {
    mUIData.buyActivityClass = buyActivity;
    mUIData.mainActivity = mainActivity;
  }

  @NonNull
  @Override
  public List< Dictionary > getDictionaries()
  {
    synchronized ( this )
    {
      return new ArrayList<>(mAvailableDictionaries);
    }
  }

  @NonNull
  @Override
  public List<DictionaryPack> getDictionaryPacks() {
    synchronized (this) {
      return new ArrayList<>(mAvailableDictionaryPacks);
    }
  }

  @Override
  public Dictionary getDictionaryById(@Nullable Dictionary.DictionaryId dictionaryId)
  {
    if (dictionaryId != null)
      synchronized ( this ) {
        Dictionary dictionary;
        for (int i = 0; i < mAvailableDictionaries.size(); i++) {
          dictionary = mAvailableDictionaries.get(i);
          if (dictionary.getId().equals(dictionaryId)) {
            return dictionary;
          }
        }
      }
    return null;
  }

  @Override
  public void registerDictionaryListObserver( @NonNull IDictionaryListObserver dictionaryListObserver )
  {
    synchronized ( mDictionaryListObservers )
    {
      if ( !mDictionaryListObservers.contains(dictionaryListObserver) )
      {
        mDictionaryListObservers.add(dictionaryListObserver);
      }
    }
  }

  @Override
  public void unRegisterDictionaryListObserver( @NonNull IDictionaryListObserver dictionaryListObserver )
  {
    synchronized ( mDictionaryListObservers )
    {
      mDictionaryListObservers.remove(dictionaryListObserver);
    }
  }

  @Nullable
  @Override
  public synchronized DictionaryPrice getDictionaryPrice( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    return mInAppPrices.get(dictionaryId);
  }

  @Override
  public Observable< Boolean > getUpdateStateObservable()
  {
    return updateStateSubject;
  }

  @Override
  public Observable< Boolean > getTrialUpdateObservable()
  {
    return updateTrialStateSubject;
  }

  @Override
  public Observable< Boolean > getUpdateFinishedAfterLicenseChangeObservable()
  {
    return updateFinishedAfterLicenseChangeSubject;
  }

  @Override
  public List< DictionaryPrice > getSubscriptionPrices( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    List< DictionaryPrice > prices = mSubscriptions.get(dictionaryId);
    return null == prices ? Collections.emptyList() : prices;
  }

  @Override
  public ErrorType activationEnd( int requestCode, int resultCode, @NonNull Intent intent )
  {
    try
    {
      return mLicenseManager != null && mLicenseManager.activationEnd(requestCode, resultCode, intent) ? ErrorType.OK : null;
    }
    catch ( ActivateException e )
    {
      return ErrorsUtils.fromException(e);
    }
  }

  @Override
  public ErrorType buyCatalogItem(@NonNull Activity activity, @NonNull Dictionary.DictionaryId dictionaryId,
                                  @Nullable DictionaryPrice.PeriodSubscription subscriptionPeriod )
  {
    CatalogItem catalogItem = null;
    for (CatalogItem item : mPdahpcItems)
      if (dictionaryId.equals(item.getId())) {
        catalogItem = item;
        break;
      }
    if (catalogItem == null)
      return ErrorType.UNDEFINED;
    try
    {
      if ( mLicenseManager != null ) {
        if (null == subscriptionPeriod)
        {
          mLicenseManager.activationBegin(activity, catalogItem.getPurchaseFeatureName(), mInitializer.getInAppSku(catalogItem));
        }
        else
        {
          mLicenseManager.activationBegin(activity, catalogItem.getPurchaseFeatureName(), mInitializer.getSubscriptionSku(catalogItem, subscriptionPeriod));
        }
      }
      return ErrorType.OK;
    }
    catch ( ItemUnavailableException e )
    {
      return ErrorType.PURCHASE_ITEM_UNAVAILABLE;
    }
    catch ( ItemAlreadyOwnedException e )
    {
      return ErrorType.PURCHASE_ITEM_ALREADY_OWNED;
    }
    catch ( UserAccountManagementException e )
    {
      return ErrorType.ACCOUNT_MANAGEMENT_EXCEPTION;
    }
    catch ( Exception e )
    {
      return ErrorType.UNDEFINED;
    }
  }

  @Override
  public void buy( @NonNull Activity activity, @NonNull final Dictionary.DictionaryId dictionaryId )
  {
    if ( mUIData.buyActivityClass != null && Activity.class.isAssignableFrom(mUIData.buyActivityClass) )
    {
      Intent intent = new Intent(activity, mUIData.buyActivityClass);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY, (Parcelable) dictionaryId);
      activity.startActivity(intent);
    }
  }

  @Override
  public void openDictionaryForSearch( @NonNull Context context,
                                       @NonNull Dictionary.DictionaryId dictionaryId,
                                       @Nullable Dictionary.Direction direction,
                                       @Nullable String text )
  {
    if ( mUIData.mainActivity != null && Activity.class.isAssignableFrom(mUIData.mainActivity) )
    {
      Intent intent = new Intent(context, mUIData.mainActivity);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY, DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_SEARCH);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY, (Parcelable) dictionaryId);
      if(direction != null)
        intent.putExtra(DictionaryManagerAPI.UI_EXTRA_DIRECTION_KEY, direction);
      if(!TextUtils.isEmpty(text))
        intent.putExtra(DictionaryManagerAPI.UI_EXTRA_SEARCH_STRING_KEY, text);
      intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
      context.startActivity(intent);
    }
  }

  @Override
  public void openDictionaryForPreview(@NonNull Context context, @NonNull Dictionary.DictionaryId dictionaryId)
  {
    if ( mUIData.mainActivity != null && Activity.class.isAssignableFrom(mUIData.mainActivity) )
    {
      Intent intent = new Intent(context, mUIData.mainActivity);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY, DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_SEARCH);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY, (Parcelable) dictionaryId);
      intent.putExtra(BundleKeys.BUNDLE_KEY_IS_OPEN_FREE_PREVIEW, true);
      intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
      context.startActivity(intent);
    }
  }

  @Override
  public void openDictionaryDescription() {
    mScreenOpener.openScreen( ScreenType.DictionaryDescription );
  }

  @Override
  public void openMyDictionariesUI( @NonNull Context context, @NonNull Dictionary.DictionaryId dictionaryId )
  {
    if ( mUIData.mainActivity != null && Activity.class.isAssignableFrom(mUIData.mainActivity) )
    {
      Intent intent = new Intent(context, mUIData.mainActivity);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY, DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_CATALOG);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY, (Parcelable) dictionaryId);
      intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
      context.startActivity(intent);
    }
  }

  @Override
  public void openDownloadManagerUI( @NonNull Context context, @Nullable Dictionary.DictionaryId dictionaryId )
  {
    if ( mUIData.mainActivity != null && Activity.class.isAssignableFrom(mUIData.mainActivity) )
    {
      Intent intent = new Intent(context, mUIData.mainActivity);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY, DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_DOWNLOAD);
      if (dictionaryId != null)
        intent.putExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY, (Parcelable) dictionaryId);
      intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
      context.startActivity(intent);
    }
  }

  @Override
  public void openCatalogueAndRestorePurchase( @NonNull Context context ) {
    if ( mUIData.mainActivity != null && Activity.class.isAssignableFrom(mUIData.mainActivity) )
    {
      Intent intent = new Intent(context, mUIData.mainActivity);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY, DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_CATALOG);
      intent.putExtra(BundleKeys.BUNDLE_KEY_IS_RESTORE_PURCHASE, true);
      intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
      context.startActivity(intent);
    }
  }

  /**
   * @return {@link ErrorType#OK} - Consume request successfully send. <br/>Possible errors: {@link ErrorType#PURCHASE_ITEM_UNAVAILABLE}, {@link ErrorType#PURCHASE_ITEM_NOT_OWNED}
   */
  @Override
  public ErrorType consume( @NonNull Context appContext, @NonNull Dictionary dictionary )
  {
    try
    {
      FeatureName featureName = dictionary.getPurchaseFeatureName();
      if ( mLicenseManager != null )
        mLicenseManager.consume(appContext, featureName);
    }
    catch ( ActivateException e )
    {
      return ErrorsUtils.fromException(e);
    }
    return ErrorType.OK;
  }

  @Nullable
  @Override
  public DictionaryControllerAPI createController( String name )
  {
    if ( null == name || name.length() <= 0 )
    {
      return new DictionaryController(this, componentLocationResolver);
    }
    else
    {
      DictionaryControllerAPI controller = controllers.get(name);
      if ( null == controller )
      {
        controller = new DictionaryController(this, componentLocationResolver);
        controllers.put(name, controller);
      }
      return controller;
    }
  }

  @NonNull
  @Override
  public IFilterFactory getFilterFactory()
  {
    return mFilterFactory;
  }

  @Override
  void freeController( @NonNull DictionaryControllerAPI dictionaryControllerAPI )
  {
  }

  @Nullable
  @Override
  public IDictionaryDiscount getDictionaryDiscount( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    DictionaryPrice dictionaryPrice = getDictionaryPrice(dictionaryId);
    if ( null == dictionaryPrice )
    {
      return null;
    }
    MarketingData marketingData = null;
    for ( Dictionary dictionary : getDictionaries() )
    {
      if ( dictionaryId.equals(dictionary.getId()) )
      {
        marketingData = dictionary.getMarketingData();
      }
    }
    if ( null == marketingData )
    {
      return null;
    }
    Long defaultPrice = marketingData.getDefaultPriceValue(dictionaryPrice.getCurrency());
    return DictionaryDiscount.create(defaultPrice, dictionaryPrice.getPrice());
  }

  @Override
  public void updateLicensesStatus()
  {
    if ( mLicenseManager != null )
      mLicenseManager.update(mApplicationContext);
  }

  @Override
  public void onInternetConnected()
  {
    updateLicensesStatus();
  }


  @Override
  public void updateDictionariesInBackground()
  {
    updateDictionariesInBackground(null);
  }

  private void updateDictionariesInBackground(@Nullable final Runnable runnable)
  {
    final FutureTask< Void > task = new FutureTask<>(() ->
    {
      try
      {
        updateDictionariesSynchronous(runnable);
        return null;
      }
      catch ( Exception e )
      {
        e.printStackTrace();
      }
      return null;
    });
    mUpdateThreadPool.execute(task);
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "ParagonDictionaryManager{" + "mAllDictionaries=" + Arrays.toString(mPdahpcItems) + ", mInAppPrices="
        + mInAppPrices + ", mSubscriptions=" + mSubscriptions + ", mSubscriptionPurchases=" + mSubscriptionPurchases
        + ", mAvailableDictionaries=" + mAvailableDictionaries
        + ", mAvailableDictionaryPacks=" + mAvailableDictionaryPacks
            +  ", mIsDictionariesInit=" + mIsDictionariesInit + '}';
  }

  /**
   * Class to store registered UI data of the dictionary manager.
   */
  private static class UIData
  {
    @Nullable
    Class buyActivityClass = null;
    @Nullable
    Class mainActivity = null;
  }

  @VisibleForTesting
  protected final class LicenseManagerListener implements LicenseManager.ErrorNotifier
  {
    @Override
    public void onChange()
    {
      updateTrialManager();
      updateDictionariesInBackground(() -> updateFinishedAfterLicenseChangeSubject.onNext(true));
    }

    @Override
    public void onErrors( @NonNull LicenseManager.OPERATION_TYPE type, @NonNull ActivateException[] errors )
    {
//      Log.e("shdd", "onErrors() called with: type = [" + type + "], errors = [" + Arrays.toString(errors) + "]");
      notifyErrorObserversUiThread(ErrorsUtils.from(type), ErrorsUtils.fromException(errors));
    }

    private void updateTrialManager() {
      for(CatalogItem catalogItem : mPdahpcItems) {
        FeatureName featureName = catalogItem.getPurchaseFeatureName();
        if (mTrialManager != null) // Manager is null in debug
          mTrialManager.updatePurchaseStatus(mApplicationContext, featureName, mLicenseManager != null && LicenseFeature.FEATURE_STATE.ENABLED.equals(mLicenseManager.checkFeature(featureName)));
      }
    }
  }

  private final class TrialManagerListener implements TrialManagerAPI.OnUpdateListener {
      @Override
      public void onUpdate() {
        updateDictionariesInBackground(() ->  updateTrialStateSubject.onNext(true));
      }
  }

  private final class UserCoreLicensesObserver implements Observer<List<ProductLicense>>, Runnable
  {
      @Override
      public void onSubscribe(Disposable d) { }

      @Override
      public void onNext(List<ProductLicense> productLicenses) {
          getUiHandler().removeCallbacks(this);

          mUserCoreLicenses = productLicenses;
          if(isDictionariesInit())
            updateDictionariesInBackground();

          Date minDate = null;
          Date now = new Date();
          for(ProductLicense productLicense : productLicenses) {
            Date curDate = productLicense.getEndDate();
            if(curDate != null)
              if(curDate.after(now))
                if(minDate == null)
                  minDate = curDate;
                else if(curDate.before(minDate))
                  minDate = curDate;
          }
          if(minDate != null)
            getUiHandler().postDelayed(this, minDate.getTime() - now.getTime());
      }

      @Override
      public void onError(Throwable e) { }

      @Override
      public void onComplete() { }

      @Override
      public void run() {
        updateDictionariesInBackground();
      }
  }

  @VisibleForTesting
  protected void notifyErrorObserversUiThread( @NonNull final OperationType operationType, @NonNull final List< ErrorType > error )
  {
    getUiHandler().post(() -> notifyErrorObservers(operationType, error));
  }

  @VisibleForTesting
  private void notifyErrorObservers( @NonNull OperationType operationType, @NonNull List< ErrorType > error )
  {
    for ( IErrorObserver errorObserver : new ArrayList<>(mErrorObservers) )
    {
      errorObserver.onError(operationType, error);
    }
  }

  @Override
  public void registerErrorObserver( @NonNull IErrorObserver errorObserver )
  {
    mErrorObservers.add(errorObserver);
  }

  @Override
  public void unRegisterErrorObserver( @NonNull IErrorObserver errorObserver )
  {
    mErrorObservers.remove(errorObserver);
  }

  @Override
  public SubscriptionPurchase getSubscriptionPurchase( @NonNull Dictionary dictionary )
  {
    return mSubscriptionPurchases.get(dictionary.getId());
  }

  @NonNull
  @Override
  Map< Dictionary.DictionaryId, SubscriptionPurchase > getSubscriptionPurchases()
  {
    return new HashMap<>(mSubscriptionPurchases);
  }

  @Override
  public boolean isTrialAvailable(@NonNull Dictionary.DictionaryId dictionaryId) {
    boolean res = false;
    FeatureName featureName = findFeatureName(dictionaryId);
    if(featureName != null && mTrialManager != null)
      res = mTrialManager.isTrialAvailable(featureName);
    return res;
  }

  @Override
  public boolean isTrialExpired(@NonNull Dictionary.DictionaryId dictionaryId) {
    boolean res = false;
    FeatureName featureName = findFeatureName(dictionaryId);
    if (featureName != null && mTrialManager != null)
      res = mTrialManager.isTrialExpired(featureName);
    return res;
  }

  @Override
  public boolean isDictionariesInit() {
    return mIsDictionariesInit;
  }

  @Override
  public boolean isProductDemoFtsPromise() {
    Catalog catalog = PDAHPCDataParser.parseCatalog(mApplicationContext);
    if(catalog == null || catalog.getGenericData() == null)
      return false;

    return catalog.getGenericData().isDemoFts() || catalog.getGenericData().isPromiseFTSinDemo();
  }

  @Override
  public int getTrialCount() {
    return mTrialManager != null ? mTrialManager.getTrialCount() : 0;
  }

  @Override
  public boolean isTrialAvailable(@NonNull Dictionary.DictionaryId dictionaryId, int n) {
    boolean res = false;
    FeatureName featureName = findFeatureName(dictionaryId);
    if(featureName != null && mTrialManager != null)
      res = mTrialManager.isTrialAvailable(featureName, n);
    return res;
  }

  @Override
  public int getTrialLengthInMinutes( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    int res = 0;
    FeatureName featureName = findFeatureName(dictionaryId);
    if(featureName != null && mTrialManager != null)
      res = mTrialManager.getTrialLengthInMinutes(featureName);
    return res;
  }

  @Override
  public long getTrialEndTime( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    long res = 0;
    FeatureName featureName = findFeatureName(dictionaryId);
    if(featureName != null && mTrialManager != null)
      res = mTrialManager.getEndTime(featureName);
    return res;
  }

  @Nullable
  @Override
  public Date getUserCoreEndTime(@NonNull Dictionary.DictionaryId dictionaryId) {
    Date now = new Date();
    Date res = now;
    FeatureName featureName = findFeatureName(dictionaryId);
    if (featureName != null)
      for (ProductLicense productLicense : mUserCoreLicenses)
        if (featureName.toString().equals(productLicense.getProductId())
                && productLicense.getEndDate() != null
                && productLicense.getEndDate().after(res)) {
            res = productLicense.getEndDate();
        }
    return res.equals(now) ? null : res;
  }

  @Nullable
  @Override
  public TrialControllerAPI createTrialController(Context context, int n, int baseRequestCode, @NonNull final PendingIntentFactory pendingIntentFactory) {
    TrialControllerAPI res = null;
    TrialManagerAPI.PendingIntentFactory coreFactory = new TrialManagerAPI.PendingIntentFactory() {
      @Nullable
      @Override
      public PendingIntent createPendingIntent(@NonNull Fragment fragment, int requestCode) {
        return pendingIntentFactory.createPendingIntent(fragment, requestCode);
      }
    };
    TrialComplexAPI core =  mTrialManager != null ? mTrialManager.createTrialComplex(context, n, baseRequestCode, coreFactory) : null;
    if(core != null)
      res = new TrialController(core, this);
    return res;
  }

  @Nullable
  @Override
  public Dictionary.DictionaryId getDictionaryIdFromTrialNotificationIntent(@Nullable Intent intent) {
    Dictionary.DictionaryId res = null;
    FeatureName featureName = mTrialManager != null ? mTrialManager.getFeatureNameFromNotificationIntent(intent) : null;
    if(featureName != null)
      for(CatalogItem catalogItem : mPdahpcItems) {
        FeatureName dictionaryFeatureName = catalogItem.getPurchaseFeatureName();
        if(featureName.equals(dictionaryFeatureName)) {
          res = catalogItem.getId();
          break;
        }
      }
    return res;
  }

  @Override
  DownloadLibraryBuilder getDownloadLibraryBuilder()
  {
    return mDownloadLibraryBuilder;
  }

  @Nullable
  @Override
  public synchronized DictionaryAndDirection getDictionaryAndDirectionSelectedByUser()
  {
    if ( mDictionaryAndDirectionSelectedByUser == null )
    {
      DictionaryAndDirection def =
          new DictionaryAndDirection(new Dictionary.DictionaryId("!!!!"), new Dictionary.Direction(0, 0, null));
      DictionaryAndDirection loaded = def;
      try
      {
        loaded = mSettingsManager.load(DICTIONARY_AND_DIRECTION_SELECTED_BY_USER_KEY, def);
      }
      catch ( Exception ignore )
      {
      }
      if ( !def.equals(loaded) )
      {
        mDictionaryAndDirectionSelectedByUser = loaded;
      }
      else
      {
        mDictionaryAndDirectionSelectedByUser = mDefaultDictionaryAndDirection;
      }
    }
    if ( mDictionaryAndDirectionSelectedByUser != null )
    {
      Dictionary.Direction direction = mDictionaryAndDirectionSelectedByUser.getDirection();
      if ( null == direction.getIcon() )
      {
        Dictionary.DictionaryId dictionaryId = mDictionaryAndDirectionSelectedByUser.getDictionaryId();
        mDictionaryAndDirectionSelectedByUser =
            new DictionaryAndDirection(dictionaryId, resolveDirection(dictionaryId, direction));
      }
      if ( !checkDictionaryAndDirection(mDictionaryAndDirectionSelectedByUser) )
      {
        mDictionaryAndDirectionSelectedByUser = null;
      }
    }
    return mDictionaryAndDirectionSelectedByUser;
  }

  /**
   * Try search direction in available dictionaries cuz deserialized direction has transient field(icon)
   * @return Direction with available icon or deserializedDirection if cant find dictionary with specified direction
   */
  @NonNull
  private Dictionary.Direction resolveDirection( @NonNull Dictionary.DictionaryId dictionaryId,
                                                 @NonNull Dictionary.Direction deserializedDirection )
  {
    boolean res = false;
    Dictionary availableDictionary = null;
    CatalogItem[] catalogItems = mPdahpcItems;
    for ( CatalogItem catalogItem : catalogItems )
    {
      if ( dictionaryId.equals(catalogItem.getId()) && (catalogItem instanceof Dictionary) )
      {
        availableDictionary = (Dictionary) catalogItem;
        break;
      }
    }

    Dictionary.Direction availableDirection = deserializedDirection;
    if ( availableDictionary != null )
    {
      Collection< Dictionary.Direction > directions = availableDictionary.getDirections();
      if ( directions != null )
      {
        for ( Dictionary.Direction direction : directions )
        {
          if ( deserializedDirection.equals(direction) )
          {
            availableDirection = direction;
            break;
          }
        }
      }
    }
    return availableDirection;
  }

  @Override
  public synchronized void setDictionaryAndDirectionSelectedByUser(@NonNull DictionaryAndDirection dictionaryAndDirection) {
    if(!checkDictionaryAndDirection(dictionaryAndDirection)) {
      dictionaryAndDirection = null;
    }
    if (dictionaryAndDirection != null && !dictionaryAndDirection.equals(mDictionaryAndDirectionSelectedByUser)) {
      try {
        mSettingsManager.save(DICTIONARY_AND_DIRECTION_SELECTED_BY_USER_KEY, dictionaryAndDirection, true);
      }
      catch (Exception ignore) { }
      mDictionaryAndDirectionSelectedByUser = dictionaryAndDirection;
      mDictionaryAndDirectionChangeSubject.onNext(true);
    }
  }

  @Override
  public Observable<Boolean> getDictionaryAndDirectionChangeObservable() {
    return mDictionaryAndDirectionChangeSubject;
  }

  @Override
  public synchronized void setIsSelectAllDictionaries(boolean isSelectAll) {
    if ( mIsSelectAll != isSelectAll ) {
      try {
        mSettingsManager.save(IS_SELECTED_ALL_DICTIONARIES, isSelectAll, true);
      } catch (Exception ignore) { }
      mIsSelectAll = isSelectAll;
      mDictionaryAndDirectionChangeSubject.onNext(true);
    }
  }

  @Override
  public boolean isSelectAllDictionaries() {
    if ( mIsSelectAll == null ) {
      try {
        mIsSelectAll = mSettingsManager.load(IS_SELECTED_ALL_DICTIONARIES,  mDefaultDictionaryAndDirection == null);
      } catch (WrongTypeException | ManagerInitException e) {
        mIsSelectAll = mDefaultDictionaryAndDirection == null;
      }
    }
    return mIsSelectAll;
  }

  @Override
  public void setDefaultDictionaryAndDirection( @Nullable Dictionary.DictionaryId dictionaryId,
                                                @Nullable Dictionary.Direction direction )
  {
    Dictionary dictionary = null;
    for (CatalogItem catalogItem : mPdahpcItems)
      if (catalogItem.getId().equals(dictionaryId) && (catalogItem instanceof Dictionary)) {
        dictionary = (Dictionary) catalogItem;
        break;
      }
    if ( dictionary != null )
    {
      Dictionary.Direction resDirection = findDirection(dictionary, direction);
      if ( resDirection == null && !dictionary.getDirections().isEmpty() )
        resDirection = dictionary.getDirections().iterator().next();
      if ( resDirection != null )
        mDefaultDictionaryAndDirection = new DictionaryAndDirection(dictionaryId, resDirection);
    }
  }

  @Override
  public boolean isExternalBaseDownloaded(@Nullable Dictionary.DictionaryId currentDictionaryId, @Nullable String baseId) {
    for(Dictionary dict : getDictionaries())
    {
      if(dict.getId().equals(currentDictionaryId))
      {
        for(DictionaryComponent dictComp : dict.getDictionaryComponents())
        {
          if(dictComp.getSdcId().equalsIgnoreCase(baseId) && !dictComp.isDemo())
          {
            return componentLocationResolver.isDownloaded(dictComp);
          }
        }
      }
    }

    return false;
  }

  @Override
  public void openSignIn(@NonNull Activity activity, int requestCode)
  {
    if(mUserCoreManager != null)
      mUserCoreManager.login(activity, requestCode);
  }

  @Override
  public void openSignIn(@NonNull Fragment fragment, int requestCode)
  {
    if(mUserCoreManager != null)
      mUserCoreManager.login(fragment, requestCode);
  }

  @Override
  public void openSignIn(@NonNull FragmentManager fragmentManager) {
    if(mUserCoreManager != null)
      mUserCoreManager.login(fragmentManager);
  }

  @Override
  public void resetUserPassword(@NonNull FragmentManager fragmentManager) {
    if(mUserCoreManager != null)
      mUserCoreManager.resetUserPassword(fragmentManager);
  }

  @Override
  public void signOut(@NonNull Context context) {
    if(mUserCoreManager != null)
      mUserCoreManager.logout(context);
  }

  @Override
  public LanguageStrings getLanguageStrings(@NonNull Context context, int language) {
    return LanguageStringsImpl.get(context, language);
  }

  @Override
  public void loadOnlineDictionaryStatusInformation( @NonNull FragmentActivity activity )
  {
    if ( mLicenseManager != null )
      mLicenseManager.update(activity.getApplicationContext());
    if (mTrialManager != null)
      mTrialManager.startLoaders(activity);
  }

  @Override
  public void loadOnlineDictionaryStatusInformation( @NonNull Fragment fragment )
  {
    Activity activity = fragment.getActivity();
    if ( null != activity && mLicenseManager != null )
    {
      mLicenseManager.update(activity.getApplicationContext());
    }
    if (mTrialManager != null)
      mTrialManager.startLoaders(fragment);
  }

  @Override
  public void updateOnlineTrials( @NonNull Activity mainActivity )
  {
    if (mTrialManager != null)
      mTrialManager.updateOnlineTrials(mainActivity);
  }

  @Nullable
  @Override
  public FeatureName findFeatureName( @Nullable Dictionary.DictionaryId dictionaryId )
  {
    FeatureName res = null;
    for ( CatalogItem catalogItem : mPdahpcItems)
    {
      if ( catalogItem != null )
      {
        if ( catalogItem.getId().equals(dictionaryId) )
        {
          res = catalogItem.getPurchaseFeatureName();
          break;
        }
      }
    }
    return res;
  }

  @NonNull
  @Override
  public Object getScreenOpener() {
    return mScreenOpener;
  }

  private boolean checkDictionaryAndDirection(@NonNull DictionaryAndDirection dictionaryAndDirection) {
    Dictionary neededDictionary = getDictionaryById(dictionaryAndDirection.getDictionaryId());
    return findDirection(neededDictionary, dictionaryAndDirection.getDirection()) != null;
  }

  private @Nullable Dictionary.Direction findDirection( @Nullable Dictionary dictionary, @Nullable Dictionary.Direction directionToFind )
  {
    if ( dictionary != null && dictionary.getDirections() != null )
      for( Dictionary.Direction direction : dictionary.getDirections() )
        if( direction != null && direction.equals(directionToFind) )
          return direction;
    return null;
  }

  private static Handler __uiHandler = null;
  @NonNull
  private static synchronized Handler getUiHandler() {
    if(__uiHandler == null)
      __uiHandler = new Handler(Looper.getMainLooper());
    return __uiHandler;
  }

  private static CatalogItem[] getCatalogItemsFromFactory(Context context, @NonNull IDictionaryManagerFactory factory, @NonNull Dictionary.DictionaryId[] ids) {
    Dictionary[] dictionaries = factory.getDictionaries(context, ids);
    DictionaryPack[] dictionaryPacks = factory.getDictionaryPacks(context, ids);
    CatalogItem[] res = new CatalogItem[dictionaries.length + dictionaryPacks.length];
    System.arraycopy(dictionaries, 0, res, 0, dictionaries.length);
    System.arraycopy(dictionaryPacks, 0, res, dictionaries.length, dictionaryPacks.length);
    return res;
  }
}
