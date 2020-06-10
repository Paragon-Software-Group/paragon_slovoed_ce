package com.paragon_software.dictionary_manager;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.content.res.Resources;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.errors.ErrorType;
import com.paragon_software.dictionary_manager.errors.OperationType;
import com.paragon_software.dictionary_manager.filter.FilterTypeSimple;
import com.paragon_software.dictionary_manager.filter.IFilterFactory;
import com.paragon_software.dictionary_manager.filter.external.ILanguageFilter;
import com.paragon_software.dictionary_manager.filter.external.IRecentlyOpenedFilter;
import com.paragon_software.dictionary_manager.filter.external.ISearchFilter;
import com.paragon_software.dictionary_manager.local_dictionary_library.ILocalDictionaryLibrary;
import com.paragon_software.license_manager_api.LicenseFeature;
import com.paragon_software.license_manager_api.LicenseManager;
import com.paragon_software.license_manager_api.PeriodShdd;
import com.paragon_software.license_manager_api.Purchase;
import com.paragon_software.license_manager_api.SKU;
import com.paragon_software.license_manager_api.exceptions.ActivateException;
import com.paragon_software.license_manager_api.exceptions.ItemAlreadyOwnedException;
import com.paragon_software.license_manager_api.exceptions.ItemNotOwnedException;
import com.paragon_software.license_manager_api.exceptions.ItemUnavailableException;
import com.paragon_software.license_manager_api.exceptions.UserAccountManagementException;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.MockSettingsManager;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.stubs.DictionaryListObserverWithLatch;
import com.paragon_software.stubs.MockObserver;
import com.paragon_software.stubs.MockTrialManager;
import com.paragon_software.stubs.SearchAllResultMock;
import com.paragon_software.user_core_manager.MockUserCoreManager;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.shadows.ShadowLooper;

import java.io.ByteArrayInputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Currency;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import io.reactivex.Observable;
import io.reactivex.observers.TestObserver;

import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_CATALOG;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_DOWNLOAD;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_SEARCH;
import static com.paragon_software.dictionary_manager.ParagonDictionaryManager.IS_SELECTED_ALL_DICTIONARIES;
import static com.paragon_software.dictionary_manager.errors.ErrorType.ACCOUNT_MANAGEMENT_EXCEPTION;
import static com.paragon_software.dictionary_manager.errors.ErrorType.PURCHASE_ITEM_ALREADY_OWNED;
import static com.paragon_software.dictionary_manager.errors.ErrorType.PURCHASE_ITEM_NOT_OWNED;
import static com.paragon_software.dictionary_manager.errors.ErrorType.PURCHASE_ITEM_UNAVAILABLE;
import static com.paragon_software.dictionary_manager.errors.ErrorType.UNDEFINED;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.anyString;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.withSettings;

/**
 * Created by belyshov on 26/03/2018.
 */
@RunWith( RobolectricTestRunner.class )
public class ParagonDictionaryManagerTest
{
  @Rule
  public MockitoRule rule = MockitoJUnit.rule();

  private static final Dictionary.DictionaryId DICT1_ID = new Dictionary.DictionaryId("111");
  private static final String FEATURE_NAME = "feature-001";
  private static final String[] STRING_IDS = new String[]{ "1", "2", "3" };
  private static final Dictionary.DictionaryId[] DICTIONARY_IDS = MockDictFactory.getDictionariyIds(STRING_IDS);
  private static final Dictionary.DictionaryId EXISTING_ID = new Dictionary.DictionaryId(STRING_IDS[0]);
  private static final String UNSUPPORTED_ID = "Unsupported_ID";

  @Mock
  private Context   mockContext;
  @Mock
  private Resources mockResources;
  @Mock
  private AssetManager mockAssetManager;
  @Mock
  private Activity  activity;

  @Mock
  private LicenseManager          mockLicenseManager;
  @Mock
  private ILocalDictionaryLibrary mockLocalDictionaryLibrary;
  @Mock
  private SearchEngineAPI         searchEngine;
  @Mock
  private ScreenOpenerAPI         screenOpener;

  private SettingsManagerAPI      mockSettingsManager;

  private MockDictFactory         mockDictFactory;

  private MockTrialManager.Builder mockTrialBuilder;

  private ParagonDictionaryManager pDictionaryManager;

  @Before
  public void setUp() throws Exception
  {
    initMocks();

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
  }

  private void initMocks() throws Exception
  {
    doAnswer(invocation -> {
      String text = invocation.getArgument(0);
      return Mockito.mock(SearchAllResultMock.class, withSettings().useConstructor(text)
              .defaultAnswer(CALLS_REAL_METHODS));
    }).when(searchEngine).searchAll(anyString(), anyInt());

    when(mockContext.getApplicationContext()).thenReturn(mockContext);
    when(mockContext.getResources()).thenReturn(mockResources);
    when(mockContext.getAssets()).thenReturn(mockAssetManager);
    when(mockResources.openRawResource(Mockito.anyInt())).thenReturn(new ByteArrayInputStream("<data><strings locale='English'><lang_translations dicts='English' dir_from='English' dir_to='English' local_name='English' name='English'/><dicts_pattern text='%s vocabularies'/></strings><lanuages_map></lanuages_map></data>".getBytes()));
    when(mockAssetManager.list(Mockito.anyString())).thenReturn(new String[0]);

    mockSettingsManager = Mockito.spy(MockSettingsManager.class);
    mockTrialBuilder = Mockito.spy(MockTrialManager.Builder.class);

    Map< String, Dictionary.DictionaryId > featuresToDictionary = new HashMap<>();
    featuresToDictionary.put(FEATURE_NAME, EXISTING_ID);
    mockDictFactory = Mockito.mock(MockDictFactory.class, withSettings()
            .useConstructor(featuresToDictionary).defaultAnswer(CALLS_REAL_METHODS));
  }

  @Test
  public void getDictionaryTest()
  {
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    List<Dictionary> dict = pDictionaryManager.getDictionaries();
    assertNotEquals(dict.size(), 0);
    for (int i = 0; i < DICTIONARY_IDS.length; i++)
    {
      Dictionary.DictionaryId dictId = dict.get(i).getId();
      Dictionary.DictionaryId staticId = DICTIONARY_IDS[i];
      assertEquals(dictId, staticId);
    }
  }

  @Test
  public void findDictionaryByIdTest()
  {
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    List<Dictionary> dict = pDictionaryManager.getDictionaries();
    assertNotEquals(dict.size(), 0);

    for (Dictionary.DictionaryId staticDictId : DICTIONARY_IDS)
    {
      Dictionary managerDictById = pDictionaryManager.getDictionaryById(staticDictId);
      assertNotNull(managerDictById);
      assertEquals(managerDictById.getId(), staticDictId);
    }

    Dictionary.DictionaryId nonExistentDictId = new Dictionary.DictionaryId(UNSUPPORTED_ID);
    Dictionary managerDictById = pDictionaryManager.getDictionaryById(nonExistentDictId);
    assertNull(managerDictById);
  }

  @Test
  public void updateStateObservableTest()
  {
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    Observable<Boolean> updateStateObservable = pDictionaryManager.getUpdateStateObservable();

    TestObserver<Boolean> testObserver = new TestObserver<>();
    updateStateObservable.subscribe(testObserver);

//    Default value
    testObserver.assertValue(false);

    pDictionaryManager.notifyObservers();
    testObserver.assertValues(false, false);

    pDictionaryManager.updateDictionariesInBackground();
    testObserver.awaitCount(3);
    testObserver.assertValues(false, false, true);
  }

  @Test
  public void trialUpdateObservableTest() {
    Observable<Boolean> updateStateObservable = pDictionaryManager.getTrialUpdateObservable();
    assertNotNull(updateStateObservable);
  }

  @Test
  public void updateAfterLicenseChangeObservableTest() {
    Observable<Boolean> updateStateObservable = pDictionaryManager.getUpdateFinishedAfterLicenseChangeObservable();
    assertNotNull(updateStateObservable);
  }

  @Test
  public void buyFeatureTest() throws ActivateException
  {
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    LicenseFeature feature = createAvailableFeature(FEATURE_NAME);
    doReturn(Arrays.asList(feature)).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(FEATURE_NAME, LicenseFeature.FEATURE_STATE.AVAILABLE);
    prepareAnswerForGetFeature(FEATURE_NAME, feature);

    Dictionary.DictionaryId id = pDictionaryManager.getDictionaries().get(0).getId();

    ErrorType errorType = pDictionaryManager.buyCatalogItem(activity, id, null);
    verify(mockLicenseManager, times(1)).activationBegin(eq(activity), eq(feature.getName()), any(SKU.class));
    assertEquals(errorType, ErrorType.OK);

//    Check method exceptions
    doThrow(new ItemUnavailableException()).when(mockLicenseManager).activationBegin(any(Activity.class), any(FeatureName.class), any(SKU.class));
    errorType = pDictionaryManager.buyCatalogItem(activity, id, null);
    assertEquals(errorType, PURCHASE_ITEM_UNAVAILABLE);

    doThrow(new ItemAlreadyOwnedException()).when(mockLicenseManager).activationBegin(any(Activity.class), any(FeatureName.class), any(SKU.class));
    errorType = pDictionaryManager.buyCatalogItem(activity, id, null);
    assertEquals(errorType, PURCHASE_ITEM_ALREADY_OWNED);

    doThrow(new UserAccountManagementException()).when(mockLicenseManager).activationBegin(any(Activity.class), any(FeatureName.class), any(SKU.class));
    errorType = pDictionaryManager.buyCatalogItem(activity, id, null);
    assertEquals(errorType, ACCOUNT_MANAGEMENT_EXCEPTION);

    doThrow(new NullPointerException()).when(mockLicenseManager).activationBegin(any(Activity.class), any(FeatureName.class), any(SKU.class));
    errorType = pDictionaryManager.buyCatalogItem(activity, id, null);
    assertEquals(errorType, UNDEFINED);
  }

  @Test
  public void activationEndTest() throws ActivateException
  {
    Intent intent = new Intent();
    int checkCode = 10;

    when(mockLicenseManager.activationEnd(checkCode, checkCode, intent)).thenReturn(true);
    ErrorType errorType = pDictionaryManager.activationEnd(checkCode, checkCode, intent);
    verify(mockLicenseManager, times(1)).activationEnd(eq(checkCode), eq(checkCode), eq(intent));
    assertEquals(errorType, ErrorType.OK);

    when(mockLicenseManager.activationEnd(checkCode, checkCode, intent)).thenReturn(false);
    errorType = pDictionaryManager.activationEnd(checkCode, checkCode, intent);
    verify(mockLicenseManager, times(2)).activationEnd(eq(checkCode), eq(checkCode), eq(intent));
    assertNull(errorType);

//    Check method exception
    doThrow(new ActivateException()).when(mockLicenseManager).activationEnd(any(Integer.class), any(Integer.class), any(Intent.class));
    errorType = pDictionaryManager.activationEnd(checkCode, checkCode, intent);
    assertEquals(errorType, UNDEFINED);
  }

  @Test
  public void openDictionaryForSearchTest()
  {
    String testText = "Hello test";
    Dictionary.Direction testDirection = new Dictionary.Direction(1, 2, null);
    doAnswer(invocation -> {
      Intent intent = (Intent) invocation.getArguments()[0];
      assertNotNull(intent);
      assertEquals(UI_EXTRA_FRAGMENT_VALUE_SEARCH, intent.getStringExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY));
      assertEquals(DICT1_ID, intent.getParcelableExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY));
      assertEquals(testDirection, intent.getSerializableExtra(DictionaryManagerAPI.UI_EXTRA_DIRECTION_KEY));
      assertEquals(testText, intent.getStringExtra(DictionaryManagerAPI.UI_EXTRA_SEARCH_STRING_KEY));
      return null;
    }).when(mockContext).startActivity(any(Intent.class));

    pDictionaryManager.registerDictionaryManagerUI(Activity.class, Activity.class);
    pDictionaryManager.openDictionaryForSearch(mockContext, DICT1_ID, testDirection, testText);
    verify(mockContext, times(1)).startActivity(any(Intent.class));

//    In dictionaryManager mainActivity is null, check zero calls
    pDictionaryManager.registerDictionaryManagerUI(null, null);
    pDictionaryManager.openDictionaryForSearch(mockContext, DICT1_ID, testDirection, testText);
    verify(mockContext, times(1)).startActivity(any(Intent.class));

//    In dictionaryManager mainActivity is Context, check zero calls
    pDictionaryManager.registerDictionaryManagerUI(Context.class, Context.class);
    pDictionaryManager.openDictionaryForSearch(mockContext, DICT1_ID, testDirection, testText);
    verify(mockContext, times(1)).startActivity(any(Intent.class));
  }

  @Test
  public void openMyDictionariesTest()
  {
    doAnswer(invocation -> {
      Intent intent = (Intent) invocation.getArguments()[0];
      assertNotNull(intent);
      assertEquals(UI_EXTRA_FRAGMENT_VALUE_CATALOG, intent.getStringExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY));
      assertEquals(DICT1_ID, intent.getParcelableExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY));
      return null;
    }).when(mockContext).startActivity(any(Intent.class));

    pDictionaryManager.registerDictionaryManagerUI(Activity.class, Activity.class);
    pDictionaryManager.openMyDictionariesUI(mockContext, DICT1_ID);
    verify(mockContext, times(1)).startActivity(any(Intent.class));

//    In dictionaryManager mainActivity is null, check zero calls
    pDictionaryManager.registerDictionaryManagerUI(null, null);
    pDictionaryManager.openMyDictionariesUI(mockContext, DICT1_ID);
    verify(mockContext, times(1)).startActivity(any(Intent.class));

//    In dictionaryManager mainActivity is Context, check zero calls
    pDictionaryManager.registerDictionaryManagerUI(Context.class, Context.class);
    pDictionaryManager.openMyDictionariesUI(mockContext, DICT1_ID);
    verify(mockContext, times(1)).startActivity(any(Intent.class));
  }

  @Test
  public void openDownloadManagerTest()
  {
    doAnswer(invocation -> {
      Intent intent = (Intent) invocation.getArguments()[0];
      assertNotNull(intent);
      assertEquals(UI_EXTRA_FRAGMENT_VALUE_DOWNLOAD, intent.getStringExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY));
      assertEquals(DICT1_ID, intent.getParcelableExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY));
      return null;
    }).when(mockContext).startActivity(any(Intent.class));

    pDictionaryManager.registerDictionaryManagerUI(Activity.class, Activity.class);
    pDictionaryManager.openDownloadManagerUI(mockContext, DICT1_ID);
    verify(mockContext, times(1)).startActivity(any(Intent.class));

//    In dictionaryManager mainActivity is null, check zero calls
    pDictionaryManager.registerDictionaryManagerUI(null, null);
    pDictionaryManager.openDownloadManagerUI(mockContext, DICT1_ID);
    verify(mockContext, times(1)).startActivity(any(Intent.class));

//    In dictionaryManager mainActivity is Context, check zero calls
    pDictionaryManager.registerDictionaryManagerUI(Context.class, Context.class);
    pDictionaryManager.openDownloadManagerUI(mockContext, DICT1_ID);
    verify(mockContext, times(1)).startActivity(any(Intent.class));
  }

  @Test
  public void consumeTest() throws Exception
  {
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    List<Dictionary> dict = pDictionaryManager.getDictionaries();
    assertNotEquals(dict.size(), 0);
    Dictionary dictionary = dict.get(0);

    ErrorType errorType = pDictionaryManager.consume(mockContext, dictionary);
    assertEquals(errorType, ErrorType.OK);
    verify(mockLicenseManager, times(1))
            .consume(eq(mockContext), any(FeatureName.class));

    //    Check methods exception
    doThrow(new ItemUnavailableException()).when(mockLicenseManager).consume(any(Context.class), any(FeatureName.class));
    errorType = pDictionaryManager.consume(mockContext, dictionary);
    assertEquals(errorType, PURCHASE_ITEM_UNAVAILABLE);
    doThrow(new ItemNotOwnedException()).when(mockLicenseManager).consume(any(Context.class), any(FeatureName.class));
    errorType = pDictionaryManager.consume(mockContext, dictionary);
    assertEquals(errorType, PURCHASE_ITEM_NOT_OWNED);
  }

  @Test
  public void getDictionaryDiscount()
  {
    IDictionaryDiscount dictionaryDiscount = pDictionaryManager.getDictionaryDiscount(EXISTING_ID);
    assertNull(dictionaryDiscount);

    LicenseFeature feature = createAvailableFeature(FEATURE_NAME);
    doReturn(Arrays.asList(feature)).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(FEATURE_NAME, LicenseFeature.FEATURE_STATE.AVAILABLE);
    prepareAnswerForGetFeature(FEATURE_NAME, feature);

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    dictionaryDiscount = pDictionaryManager.getDictionaryDiscount(EXISTING_ID);
    assertNotNull(dictionaryDiscount);
  }

  @Test
  public void updateLicensesStatusTest()
  {
    pDictionaryManager.updateLicensesStatus();
    verify(mockLicenseManager, times(1)).update(any(Context.class));
  }

  @Test
  public void onInternetConnectedTest()
  {
    pDictionaryManager.onInternetConnected();
    verify(mockLicenseManager, times(1)).update(any(Context.class));
  }

  @Test
  public void errorObserverTest()
  {
    DictionaryManagerAPI.IErrorObserver errorObserver = Mockito.spy(DictionaryManagerAPI.IErrorObserver.class);
    ArrayList<ErrorType> mockErrorTypes = new ArrayList<>(1);
    mockErrorTypes.add(UNDEFINED);
    OperationType mockOperationType = OperationType.UNDEFINED;

    pDictionaryManager.notifyErrorObserversUiThread(mockOperationType, mockErrorTypes);
    verify(errorObserver, times(0))
            .onError(any(OperationType.class), any(List.class));

    pDictionaryManager.registerErrorObserver(errorObserver);
    pDictionaryManager.notifyErrorObserversUiThread(mockOperationType, mockErrorTypes);
    verify(errorObserver, times(1))
            .onError(any(OperationType.class), any(List.class));

    pDictionaryManager.unRegisterErrorObserver(errorObserver);
    pDictionaryManager.notifyErrorObserversUiThread(mockOperationType, mockErrorTypes);
    verify(errorObserver, times(1))
            .onError(any(OperationType.class), any(List.class));
  }

  @Test
  public void licenseManagerListenerTest()
  {
    pDictionaryManager.mLicenseManagerListener.onChange();

    verify(pDictionaryManager.mTrialManager, times(DICTIONARY_IDS.length))
            .updatePurchaseStatus(any(Context.class), any(FeatureName.class), anyBoolean());
  }

  @Test
  public void getSubscriptionPurchaseTest() throws Exception
  {
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    if (DICTIONARY_IDS.length < 2)
      throw new Exception("This test requires 3 or more DICTIONARY_ID");

    HashMap<Dictionary.DictionaryId, SubscriptionPurchase> mockSubscrPurchases = new HashMap<>();
    SubscriptionPurchase mock1 = Mockito.mock(SubscriptionPurchase.class);
    mockSubscrPurchases.put(DICTIONARY_IDS[0], mock1);
    SubscriptionPurchase mock2 = Mockito.mock(SubscriptionPurchase.class);
    mockSubscrPurchases.put(DICTIONARY_IDS[1], mock2);
    pDictionaryManager.replaceSubscriptionPurchases(mockSubscrPurchases);

    assertEquals(pDictionaryManager.getSubscriptionPurchases(), mockSubscrPurchases);

    Dictionary dictionaryById = pDictionaryManager.getDictionaryById(DICTIONARY_IDS[0]);
    SubscriptionPurchase subscriptionPurchase = pDictionaryManager.getSubscriptionPurchase(dictionaryById);
    assertEquals(subscriptionPurchase, mock1);

    Dictionary dictionaryById1 = pDictionaryManager.getDictionaryById(DICTIONARY_IDS[1]);
    SubscriptionPurchase subscriptionPurchase1 = pDictionaryManager.getSubscriptionPurchase(dictionaryById1);
    assertEquals(subscriptionPurchase1, mock2);

    Dictionary dictionaryById2 = pDictionaryManager.getDictionaryById(DICTIONARY_IDS[2]);
    SubscriptionPurchase subscriptionPurchase2 = pDictionaryManager.getSubscriptionPurchase(dictionaryById2);
    assertNull(subscriptionPurchase2);
  }

  @Test
  public void isTrialAvailableTest()
  {
    pDictionaryManager.isTrialAvailable(DICTIONARY_IDS[0]);
    verify(pDictionaryManager.mTrialManager, times(1))
            .isTrialAvailable(any(FeatureName.class));

    pDictionaryManager.isTrialAvailable(DICTIONARY_IDS[0], 0);
    verify(pDictionaryManager.mTrialManager, times(1))
            .isTrialAvailable(any(FeatureName.class), anyInt());

    boolean isTrialAvailable = pDictionaryManager
            .isTrialAvailable(new Dictionary.DictionaryId(UNSUPPORTED_ID));
    assertFalse(isTrialAvailable);

    isTrialAvailable = pDictionaryManager
            .isTrialAvailable(new Dictionary.DictionaryId(UNSUPPORTED_ID), 0);
//    False is default value
    assertFalse(isTrialAvailable);
  }

  @Test
  public void isTrialExpiredTest()
  {
    pDictionaryManager.isTrialExpired(DICTIONARY_IDS[0]);
    verify(pDictionaryManager.mTrialManager, times(1))
            .isTrialExpired(any(FeatureName.class));

    boolean isTrialExpired = pDictionaryManager
            .isTrialExpired(new Dictionary.DictionaryId(UNSUPPORTED_ID));
//    False is default value
    assertFalse(isTrialExpired);
  }

  @Test
  public void isDictionariesInitTest()
  {
    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
    assertFalse(pDictionaryManager.isDictionariesInit());

    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();
    assertTrue(pDictionaryManager.isDictionariesInit());
  }

  @Test
  public void getTrialCountTest()
  {
    int trialCount = 8;
    when(pDictionaryManager.mTrialManager.getTrialCount()).thenReturn(trialCount);
    assertEquals(pDictionaryManager.getTrialCount(), trialCount);
  }

  @Test
  public void getTrialLengthTest()
  {
    pDictionaryManager.getTrialLengthInMinutes(DICTIONARY_IDS[0]);
    verify(pDictionaryManager.mTrialManager, times(1))
            .getTrialLengthInMinutes(any(FeatureName.class));

    int trialMinutes = pDictionaryManager
            .getTrialLengthInMinutes(new Dictionary.DictionaryId(UNSUPPORTED_ID));
//    Zero is default value
    assertEquals(0, trialMinutes);
  }

  @Test
  public void getTrialEndTimeTest()
  {
    pDictionaryManager.getTrialEndTime(DICTIONARY_IDS[0]);
    verify(pDictionaryManager.mTrialManager, times(1))
            .getEndTime(any(FeatureName.class));

    long endDate = pDictionaryManager
            .getTrialEndTime(new Dictionary.DictionaryId(UNSUPPORTED_ID));
//    Zero is default value
    assertEquals(0, endDate);
  }

  @Test
  public void getUserCoreEndTimeTest()
  {
    Date userCoreEndTime = pDictionaryManager.getUserCoreEndTime(new Dictionary.DictionaryId(UNSUPPORTED_ID));
//    Null is default value
    assertNull(userCoreEndTime);
  }

  @Test
  public void getDictionaryIdFromTrialIntentTest()
  {
    Dictionary.DictionaryId dictId = pDictionaryManager.getDictionaryIdFromTrialNotificationIntent(new Intent());
    assertNull(dictId);

    LicenseFeature feature = createAvailableFeature(FEATURE_NAME);
    when(pDictionaryManager.mTrialManager.getFeatureNameFromNotificationIntent(any(Intent.class)))
            .thenReturn(feature.getName());

    dictId = pDictionaryManager.getDictionaryIdFromTrialNotificationIntent(new Intent());
    assertNotNull(dictId);
  }

  @Test
  public void isSelectAllDictionariesTest() throws ManagerInitException, WrongTypeException, LocalResourceUnavailableException {
    assertTrue(pDictionaryManager.isSelectAllDictionaries());
    verify(mockSettingsManager, times(1)).load(eq(IS_SELECTED_ALL_DICTIONARIES), eq(true));
    pDictionaryManager.setIsSelectAllDictionaries(false);
    verify(mockSettingsManager, times(1)).save(eq(IS_SELECTED_ALL_DICTIONARIES), eq(false), eq(true));
    assertFalse(pDictionaryManager.isSelectAllDictionaries());

    pDictionaryManager.setIsSelectAllDictionaries(true);
    verify(mockSettingsManager, times(1)).save(eq(IS_SELECTED_ALL_DICTIONARIES), eq(true), eq(true));
    assertTrue(pDictionaryManager.isSelectAllDictionaries());

    verify(mockSettingsManager, times(1)).load(eq(IS_SELECTED_ALL_DICTIONARIES), any(Boolean.class));
  }

  @Test
  public void findFeatureNameTest()
  {
      FeatureName featureName = pDictionaryManager.findFeatureName(DICTIONARY_IDS[0]);
      assertEquals(featureName.toString(), FEATURE_NAME);
  }

  @Test
  public void findDictionaryTest()
  {
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();
    Dictionary.DictionaryId firstDictId = DICTIONARY_IDS[0];
    Dictionary dictionary = pDictionaryManager.getDictionaryById(firstDictId);
    assertNotNull(dictionary);
    assertEquals(dictionary.getId(), firstDictId);

    dictionary = pDictionaryManager.getDictionaryById(new Dictionary.DictionaryId(UNSUPPORTED_ID));
    assertNull(dictionary);
  }

  @Test
  public void initializeAndGetDictionaryWhenSubscriptionsFeatureListIsEmpty() throws Exception
  {
    LicenseFeature feature = createEnabledFeature(FEATURE_NAME);
    doReturn(Arrays.asList(feature)).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(FEATURE_NAME, LicenseFeature.FEATURE_STATE.AVAILABLE);
    prepareAnswerForGetFeature(FEATURE_NAME, feature);

    List< DictionaryPrice > subscriptionPrices = pDictionaryManager.getSubscriptionPrices(EXISTING_ID);
    assertNotNull(subscriptionPrices);
    assertEquals(0, subscriptionPrices.size());
  }

  @Test
  public void initializeAndGetDictionaryWhenSubscriptionsFeatureExists() throws Exception
  {
    LicenseFeature feature = createEnabledFeatureWithSkuEndDate(FEATURE_NAME);
    doReturn(Arrays.asList(feature)).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(FEATURE_NAME, LicenseFeature.FEATURE_STATE.AVAILABLE);
    prepareAnswerForGetFeature(FEATURE_NAME, feature);

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    List< DictionaryPrice > subscriptionPrices = pDictionaryManager.getSubscriptionPrices(EXISTING_ID);
    assertNotNull(subscriptionPrices);
    assertEquals(1, subscriptionPrices.size());
  }

  @Test
  public void initializeAndGetDictionaryWhenFeatureListIsEmpty() throws Exception
  {
    doReturn(Collections.< LicenseFeature >emptyList()).when(mockLicenseManager).getFeatures();
    doReturn(LicenseFeature.FEATURE_STATE.DISABLED).when(mockLicenseManager)
            .checkFeature(FeatureName.getEmpty());

    Collection< Dictionary > dicts = pDictionaryManager.getDictionaries();
    assertTrue(dicts.isEmpty());
  }

  @Test
  public void initializeAndGetDictionaryWhenFeatureListContainsExisting() throws Exception
  {
    LicenseFeature feature = createEnabledFeature(FEATURE_NAME);
    doReturn(Arrays.asList(feature)).when(mockLicenseManager).getFeatures();
    prepareAnswerForGetFeature(FEATURE_NAME, feature);

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    Collection< Dictionary > dicts = pDictionaryManager.getDictionaries();
    assertEquals(1, dicts.size());
    assertEquals(( dicts.toArray(new Dictionary[0])[0].getId() ), EXISTING_ID);
  }

  @Test
  public void initializeAndGetDictionaryWhenFeatureListContainsOnlyUnExisting() throws Exception
  {
    String firstId = STRING_IDS[0];
    doReturn(Arrays.asList(createEnabledFeature(firstId))).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(firstId, LicenseFeature.FEATURE_STATE.ENABLED);

    Collection< Dictionary > dicts = pDictionaryManager.getDictionaries();
    assertEquals(0, dicts.size());
  }

  @Test
  public void initializeAndGetDictionaryWhenFeatureListContainsExistingDisabled() throws Exception
  {
    String firstId = STRING_IDS[0];
    doReturn(Arrays.asList(createDisabledFeature(firstId))).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(firstId, LicenseFeature.FEATURE_STATE.DISABLED);

    Collection< Dictionary > dicts = pDictionaryManager.getDictionaries();
    assertEquals(0, dicts.size());
  }

  @Test
  public void initializeAndGetDictionaryWhenFeatureListContainsExistingAvailable() throws Exception
  {
    LicenseFeature feature = createAvailableFeature(FEATURE_NAME);
    doReturn(Arrays.asList(feature)).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(FEATURE_NAME, LicenseFeature.FEATURE_STATE.AVAILABLE);
    prepareAnswerForGetFeature(FEATURE_NAME, feature);

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    Collection< Dictionary > dicts = pDictionaryManager.getDictionaries();
    assertEquals(1, dicts.size());
    assertEquals(( dicts.toArray(new Dictionary[0])[0].getId() ), EXISTING_ID);
  }

  @Test
  public void dontNotifyAfterRegisterObserver() throws Exception
  {
    DictionaryManagerAPI.IDictionaryListObserver spyObserver = Mockito.mock(MockObserver.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    doNothing().when(spyObserver).onDictionaryListChanged();

    pDictionaryManager.registerDictionaryListObserver(spyObserver);
    verify(spyObserver, times(0)).onDictionaryListChanged();
  }

  @Test
  public void NotifyAfterRegisterObserverInTimeOfLongInitialization() throws Exception
  {
    DictionaryManagerAPI.IDictionaryListObserver spyObserver = Mockito.mock(MockObserver.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    doNothing().when(spyObserver).onDictionaryListChanged();

    pDictionaryManager.registerDictionaryListObserver(spyObserver);
    verify(spyObserver, times(0)).onDictionaryListChanged();
  }

  @Test
  public void registerAndNotifyObservers() throws Exception
  {
    DictionaryManagerAPI.IDictionaryListObserver spyObserver = Mockito.mock(MockObserver.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    doNothing().when(spyObserver).onDictionaryListChanged();

    pDictionaryManager.registerDictionaryListObserver(spyObserver);
    pDictionaryManager.notifyObservers();
    verify(spyObserver, times(1)).onDictionaryListChanged();
  }

  @Test
  public void registerAndNotifyTwoObservers() throws Exception
  {
    DictionaryManagerAPI.IDictionaryListObserver spyObserver = Mockito.mock(MockObserver.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    DictionaryManagerAPI.IDictionaryListObserver spyObserver2 = Mockito.mock(MockObserver.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    doNothing().when(spyObserver).onDictionaryListChanged();

    pDictionaryManager.registerDictionaryListObserver(spyObserver);
    pDictionaryManager.registerDictionaryListObserver(spyObserver2);
    pDictionaryManager.notifyObservers();
    verify(spyObserver, times(1)).onDictionaryListChanged();
    verify(spyObserver2, times(1)).onDictionaryListChanged();
  }

  @Test
  public void notifyWithoutObservers() throws Exception
  {
    pDictionaryManager.notifyObservers();
    assertTrue(true);
  }

  @Test
  public void registerAndUnregisterObserver() throws Exception
  {
    DictionaryManagerAPI.IDictionaryListObserver spyObserver = Mockito.mock(MockObserver.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    doNothing().when(spyObserver).onDictionaryListChanged();

    pDictionaryManager.registerDictionaryListObserver(spyObserver);
    pDictionaryManager.unRegisterDictionaryListObserver(spyObserver);
    pDictionaryManager.notifyObservers();
    verify(spyObserver, times(0)).onDictionaryListChanged();
  }

  @Test
  public void registerTwoAndUnregisterOneObserver() throws Exception
  {
    DictionaryManagerAPI.IDictionaryListObserver spyObserver = Mockito.mock(MockObserver.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    DictionaryManagerAPI.IDictionaryListObserver spyObserver2 = Mockito.mock(MockObserver.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    doNothing().when(spyObserver).onDictionaryListChanged();

    pDictionaryManager.registerDictionaryListObserver(spyObserver);
    pDictionaryManager.registerDictionaryListObserver(spyObserver2);
    pDictionaryManager.unRegisterDictionaryListObserver(spyObserver);
    pDictionaryManager.notifyObservers();
    verify(spyObserver, times(0)).onDictionaryListChanged();
    verify(spyObserver2, times(1)).onDictionaryListChanged();
  }

  @Test
  public void getPriceForUnexisting() throws Exception
  {
    String firstId = STRING_IDS[0];
    doReturn(Arrays.asList(createEnabledFeature(firstId))).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(firstId, LicenseFeature.FEATURE_STATE.ENABLED);

    DictionaryPrice price = pDictionaryManager.getDictionaryPrice(new Dictionary.DictionaryId(UNSUPPORTED_ID));
    assertNull(price);
  }

  @Test
  public void getPriceForExistingWhenPriceIsNull() throws Exception
  {
    String firstId = STRING_IDS[0];
    doReturn(Arrays.asList(createEnabledFeature(firstId))).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(firstId, LicenseFeature.FEATURE_STATE.ENABLED);

    DictionaryPrice price = pDictionaryManager.getDictionaryPrice(new Dictionary.DictionaryId(firstId));
    assertNull(price);
  }

  @Test
  public void getPriceForExistingWhenPriceNotNull() throws Exception
  {
    LicenseFeature feature = createAvailableFeature(FEATURE_NAME);
    doReturn(Arrays.asList(feature)).when(mockLicenseManager).getFeatures();
    prepareAnswerForCheckFeature(FEATURE_NAME, LicenseFeature.FEATURE_STATE.AVAILABLE);
    prepareAnswerForGetFeature(FEATURE_NAME, feature);

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    DictionaryPrice price = pDictionaryManager.getDictionaryPrice(EXISTING_ID);
    assertNotNull(price);
  }

  @Test
  public void getFilterFactory() throws Exception
  {
    assertNotNull(pDictionaryManager.getFilterFactory());
  }

  @Test
  public void getSimpleFiltersDictionary() throws Exception
  {
    IFilterFactory filterFactory = pDictionaryManager.getFilterFactory();
    assertNotNull(filterFactory);
    for ( FilterTypeSimple type : FilterTypeSimple.values() )
    {
      assertNotNull(filterFactory.createByType(type));
    }
  }

  @Test
  public void getFiltersByClassDictionary() throws Exception
  {
    IFilterFactory filterFactory = pDictionaryManager.getFilterFactory();
    assertNotNull(filterFactory);
    ILanguageFilter languageFilter = filterFactory.createByClass(ILanguageFilter.class);
    IRecentlyOpenedFilter recentlyOpenedFilter = filterFactory.createByClass(IRecentlyOpenedFilter.class);
    ISearchFilter nameFilter = filterFactory.createByClass(ISearchFilter.class);
    assertNotNull(languageFilter);
    assertNotNull(recentlyOpenedFilter);
    assertNotNull(nameFilter);
  }

  @Test
  public void languageFilterDictionary() throws Exception
  {
    IFilterFactory filterFactory = pDictionaryManager.getFilterFactory();
    assertNotNull(filterFactory);
    ILanguageFilter languageFilter = filterFactory.createByClass(ILanguageFilter.class);
    assertNotNull(languageFilter);
    assertNull(languageFilter.getInitialLanguageFrom());
    assertNull(languageFilter.getInitialLanguageTo());
    languageFilter.setLanguages(888, 999);
    assertEquals(888, (int) languageFilter.getInitialLanguageFrom());
    assertEquals(999, (int) languageFilter.getInitialLanguageTo());
  }

  @Test
  public void filterRecentlyOpenedDictionary() throws Exception
  {
    IFilterFactory filterFactory = pDictionaryManager.getFilterFactory();
    IRecentlyOpenedFilter recentlyOpenedFilter = filterFactory.createByClass(IRecentlyOpenedFilter.class);
    assertNotNull(recentlyOpenedFilter);
    assertTrue(recentlyOpenedFilter.getRecentlyOpened().isEmpty());
    recentlyOpenedFilter.addRecentlyOpened(new Dictionary.DictionaryId(STRING_IDS[0]));
    assertFalse(recentlyOpenedFilter.getRecentlyOpened().isEmpty());
    assertEquals(1, recentlyOpenedFilter.getRecentlyOpened().size());
    assertEquals(new Dictionary.DictionaryId(STRING_IDS[0]), recentlyOpenedFilter.getRecentlyOpened().get(0));
    // insert existing id
    recentlyOpenedFilter.addRecentlyOpened(new Dictionary.DictionaryId(STRING_IDS[0]));
    assertEquals(1, recentlyOpenedFilter.getRecentlyOpened().size());
    assertEquals(new Dictionary.DictionaryId(STRING_IDS[0]), recentlyOpenedFilter.getRecentlyOpened().get(0));
    // insert another id
    recentlyOpenedFilter.addRecentlyOpened(new Dictionary.DictionaryId(STRING_IDS[1]));
    assertEquals(2, recentlyOpenedFilter.getRecentlyOpened().size());
    // last added - on top
    assertEquals(new Dictionary.DictionaryId(STRING_IDS[1]), recentlyOpenedFilter.getRecentlyOpened().get(0));
    assertEquals(new Dictionary.DictionaryId(STRING_IDS[0]), recentlyOpenedFilter.getRecentlyOpened().get(1));
  }

  @Test
  public void installLanguageFilterDictionary() throws Exception
  {
    Map< String, Dictionary.DictionaryId > featuresToDictionary;
    {
      String[] dictionaryIds = { "1", "2", "3", "4", "5" };
      String [] features = new String [] { "feature-001", "feature-002", "feature-003", "feature-004", "feature-005"};
      featuresToDictionary = getFeatureToDictionaryIdMap(dictionaryIds, features);
    }

    Dictionary.DictionaryId[] DICTIONARY_IDS =
        new HashSet<>(featuresToDictionary.values()).toArray(new Dictionary.DictionaryId[]{});

    MockDictFactory mockDictFactory = Mockito.mock(MockDictFactory.class, withSettings()
          .useConstructor(featuresToDictionary).defaultAnswer(CALLS_REAL_METHODS));

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    DictionaryControllerAPI controller = pDictionaryManager.createController(null);
    assertNotNull(controller);
    assertNotNull(pDictionaryManager.getDictionaries());
    assertEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);

    IFilterFactory filterFactory = pDictionaryManager.getFilterFactory();
    assertNotNull(filterFactory);
    ILanguageFilter languageFilter = filterFactory.createByClass(ILanguageFilter.class);
    assertNotNull(languageFilter);

    controller.installFilter(languageFilter);
    assertEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);
    languageFilter.setLanguages(888, 999);
    assertEquals(0, controller.getDictionaries().length);

    Dictionary existing = pDictionaryManager.getDictionaries().iterator().next();
    languageFilter.setLanguages(existing.getDirections().iterator().next().getLanguageFrom(), existing.getDirections().iterator().next().getLanguageTo());
    assertNotEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);
    assertNotEquals(0, controller.getDictionaries().length);

    controller.uninstallFilter(languageFilter);
    assertEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);
  }

  @Test
  public void installEmptySearchFilterDictionary() throws Exception
  {
    Map< String, Dictionary.DictionaryId > featuresToDictionary;
    {
      String[] dictionaryIds = { "1", "2", "3", "4", "5" };
      String [] features = new String [] { "feature-001", "feature-002", "feature-003", "feature-004", "feature-005"};
      featuresToDictionary = getFeatureToDictionaryIdMap(dictionaryIds, features);
    }

    Dictionary.DictionaryId[] array = {};
    Dictionary.DictionaryId[] DICTIONARY_IDS = new HashSet<>(featuresToDictionary.values()).toArray(array);

    MockDictFactory mockDictFactory = Mockito.mock(MockDictFactory.class, withSettings()
          .useConstructor(featuresToDictionary).defaultAnswer(CALLS_REAL_METHODS));

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);

    DictionaryControllerAPI controller = pDictionaryManager.createController(null);
    assertNotNull(controller);
    assertNotNull(pDictionaryManager.getDictionaries());

    IFilterFactory filterFactory = pDictionaryManager.getFilterFactory();
    assertNotNull(filterFactory);
    ISearchFilter searchFilter = filterFactory.createByClass(ISearchFilter.class);
    assertNotNull(searchFilter);
    searchFilter.setSearchString("");
    controller.installFilter(searchFilter);
    assertEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);
    searchFilter.setSearchString("");
    assertEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);
  }

  @Test
  public void installSearchFilterDictionary() throws Exception
  {
    String[] dictionaryIds = { "1", "2", "3", "4", "5" };
    Map< String, Dictionary.DictionaryId > featuresToDictionary;
    {
      String [] features = new String [] { "feature-001", "feature-002", "feature-003", "feature-004", "feature-005"};
      featuresToDictionary = getFeatureToDictionaryIdMap(dictionaryIds, features);
    }

    Dictionary.DictionaryId[] DICTIONARY_IDS =
        new HashSet<>(featuresToDictionary.values()).toArray(new Dictionary.DictionaryId[]{});

    MockDictFactory mockDictFactory = Mockito.mock(MockDictFactory.class, withSettings()
          .useConstructor(featuresToDictionary).defaultAnswer(CALLS_REAL_METHODS));

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();

    DictionaryControllerAPI controller = pDictionaryManager.createController(null);
    assertNotNull(controller);
    assertNotNull(pDictionaryManager.getDictionaries());

    IFilterFactory filterFactory = pDictionaryManager.getFilterFactory();
    assertNotNull(filterFactory);
    ISearchFilter searchFilter = filterFactory.createByClass(ISearchFilter.class);
    assertNotNull(searchFilter);
    controller.installFilter(searchFilter);
    assertEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);
    assertEquals(dictionaryIds.length, controller.getDictionaries().length);
    searchFilter.setSearchString("2;4;11;1");
    Thread.sleep(2000);
    assertEquals(3, controller.getDictionaries().length);

    searchFilter.setSearchString("2;4;11;1");
    Thread.sleep(800);
    searchFilter.setSearchString("5;11;3");
    Thread.sleep(2000);
    assertEquals(2, controller.getDictionaries().length);
    searchFilter.setSearchString("");
    assertEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);
  }

  @Test
  public void installLanguageFilterDictionaryWithObserver() throws Exception
  {
    Map< String, Dictionary.DictionaryId > featuresToDictionary;
    {
      String[] dictionaryIds = { "1", "2", "3", "4", "5" };
      String[] features = new String[]{ "feature-001", "feature-002", "feature-003", "feature-004", "feature-005" };
      featuresToDictionary = getFeatureToDictionaryIdMap(dictionaryIds, features);
    }

    Dictionary.DictionaryId[] DICTIONARY_IDS =
        new HashSet<>(featuresToDictionary.values()).toArray(new Dictionary.DictionaryId[]{});

    MockDictFactory mockDictFactory = Mockito.mock(MockDictFactory.class, withSettings()
          .useConstructor(featuresToDictionary).defaultAnswer(CALLS_REAL_METHODS));

    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, mockDictFactory, DICTIONARY_IDS);

    DictionaryControllerAPI controller = pDictionaryManager.createController(null);
    assertNotNull(controller);

    IFilterFactory filterFactory = pDictionaryManager.getFilterFactory();
    assertNotNull(filterFactory);
    ILanguageFilter languageFilter = filterFactory.createByClass(ILanguageFilter.class);
    assertNotNull(languageFilter);

    final CountDownLatch latch = new CountDownLatch(1);

    DictionaryListObserverWithLatch observer = Mockito.mock(DictionaryListObserverWithLatch.class,
            withSettings().useConstructor(latch).defaultAnswer(CALLS_REAL_METHODS));
    controller.registerObserver(observer);
    controller.installFilter(languageFilter);
    assertTrue(latch.await(2, TimeUnit.SECONDS));

    CountDownLatch latch2 = new CountDownLatch(1);
    observer.setLatch(latch2);
    controller.uninstallFilter(languageFilter);
    assertTrue(latch2.await(2, TimeUnit.SECONDS));

    controller.unregisterObserver(observer);
    CountDownLatch latch3 = new CountDownLatch(1);
    observer.setLatch(latch3);
    controller.installFilter(languageFilter);
    assertFalse(latch3.await(1, TimeUnit.SECONDS));

    controller.registerObserver(observer);
    CountDownLatch latch4 = new CountDownLatch(1);
    observer.setLatch(latch4);
    languageFilter.setLanguages(888, 999);
    assertTrue(latch4.await(2, TimeUnit.SECONDS));

    CountDownLatch latch5 = new CountDownLatch(1);
    observer.setLatch(latch5);

    CountDownLatch latch6 = new CountDownLatch(1);
    DictionaryManagerAPI.IDictionaryListObserver observer2 = Mockito.mock(DictionaryListObserverWithLatch.class,
            withSettings().useConstructor(latch6).defaultAnswer(CALLS_REAL_METHODS));
    controller.registerObserver(observer2);

    languageFilter.setLanguages(888, 999);
    assertTrue(latch5.await(2, TimeUnit.SECONDS) && latch6.await(2, TimeUnit.SECONDS));

    controller.uninstallFilter(languageFilter);
    controller.unregisterObserver(observer);
    controller.unregisterObserver(observer2);
    assertEquals(pDictionaryManager.getDictionaries().size(), controller.getDictionaries().length);
  }

  @Test
  public void testStartBuyActivity() throws Exception
  {
    pDictionaryManager.registerDictionaryManagerUI(Activity.class, Activity.class);
    doAnswer(invocation -> {
      Intent intent = (Intent) invocation.getArguments()[0];
      assertNotNull(intent);
      assertEquals(DICT1_ID, intent.getParcelableExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY));
      return null;
    }).when(activity).startActivity(any(Intent.class));
    pDictionaryManager.buy(activity, DICT1_ID);
    verify(activity, times(1)).startActivity(any(Intent.class));
    pDictionaryManager.registerDictionaryManagerUI(null, null);
    pDictionaryManager.buy(activity, DICT1_ID);
    verify(activity, times(1)).startActivity(any(Intent.class));
    pDictionaryManager.registerDictionaryManagerUI(Context.class, Context.class);
    pDictionaryManager.buy(activity, DICT1_ID);
    verify(activity, times(1)).startActivity(any(Intent.class));
  }

  @Test
  public void testGetDictionaryAndDirectionSelectedByUserNull() throws Exception
  {
    assertNull(pDictionaryManager.getDictionaryAndDirectionSelectedByUser());
  }

  @Test
  public void testSetDictionaryAndDirectionSelectedByUser() throws Exception
  {
    String firstId = STRING_IDS[0];
    assertNull(pDictionaryManager.getDictionaryAndDirectionSelectedByUser());
    int russ = Language.getLangCodeFromShortForm("russ");
    int engl = Language.getLangCodeFromShortForm("germ");
    DictionaryAndDirection dictionaryAndDirection
            = new DictionaryAndDirection(new Dictionary.DictionaryId(firstId)
              , new Dictionary.Direction(russ, engl, null));
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();
    pDictionaryManager.setDictionaryAndDirectionSelectedByUser(dictionaryAndDirection);
    Assert.assertEquals(dictionaryAndDirection, pDictionaryManager.getDictionaryAndDirectionSelectedByUser());
  }

  @Test
  public void testSetDictionaryAndDirectionSelectedByUserPersistent() throws Exception
  {
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();
    String firstId = STRING_IDS[0];
    assertNull(pDictionaryManager.getDictionaryAndDirectionSelectedByUser());
    int russ = Language.getLangCodeFromShortForm("russ");
    int engl = Language.getLangCodeFromShortForm("germ");
    DictionaryAndDirection dictionaryAndDirection
            = new DictionaryAndDirection(new Dictionary.DictionaryId(firstId)
              , new Dictionary.Direction(russ, engl, null));
    pDictionaryManager.setDictionaryAndDirectionSelectedByUser(dictionaryAndDirection);
    MockDictFactory factory2 = Mockito.spy(MockDictFactory.class);
    pDictionaryManager = getDmSync(mockLicenseManager, mockContext, factory2, DICTIONARY_IDS);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();
    Assert.assertEquals(dictionaryAndDirection, pDictionaryManager.getDictionaryAndDirectionSelectedByUser());
  }

  @NonNull
  private Map< String, Dictionary.DictionaryId > getFeatureToDictionaryIdMap( String[] dictionaryIds,
                                                                              String[] features )
  {
    Map< String, Dictionary.DictionaryId > featuresToDictionary;
    List< LicenseFeature > licenceFeatures = new ArrayList<>();
    featuresToDictionary = new HashMap<>();

    for ( int i = 0 ; i < features.length ; i++ )
    {
      String FEATURE_NAME = features[i];
      LicenseFeature feature = createAvailableFeature(FEATURE_NAME);
      featuresToDictionary.put(FEATURE_NAME, new Dictionary.DictionaryId(dictionaryIds[i]));
      licenceFeatures.add(feature);
    }

    prepareAnswerForCheckFeature(Arrays.asList(features), LicenseFeature.FEATURE_STATE.AVAILABLE);
    prepareAnswerForGetFeature(Arrays.asList(features), licenceFeatures);
    doReturn(licenceFeatures).when(mockLicenseManager).getFeatures();
    return featuresToDictionary;
  }

  private ParagonDictionaryManager getDmSync( @NonNull LicenseManager licenseManager,
                                              @NonNull final Context context,
                                              @NonNull IDictionaryManagerFactory factory,
                                              @NonNull Dictionary.DictionaryId[] dictionaryId )
  {
    final CountDownLatch syncLatch = new CountDownLatch(1);
    final DictionaryManagerAPI.IDictionaryListObserver observer = syncLatch::countDown;
    final DictionaryManagerAPI.IDictionaryListObserver[] observers =
        new DictionaryManagerAPI.IDictionaryListObserver[]{ observer };

    MockUserCoreManager userCoreManager = Mockito.spy(MockUserCoreManager.class);

    final ParagonDictionaryManager paragonDictionaryManager = new ParagonDictionaryManager(context,
            licenseManager, factory, mockSettingsManager, searchEngine, dictionaryId, observers,
            mockLocalDictionaryLibrary, mockTrialBuilder, null, userCoreManager,
            screenOpener);
    try
    {
      syncLatch.await(1, TimeUnit.SECONDS);
    }
    catch ( InterruptedException ignore )
    {
    }
    return paragonDictionaryManager;
  }

  private void prepareAnswerForCheckFeature( @NonNull final String id,
                                             @NonNull final LicenseFeature.FEATURE_STATE state )
  {
    doAnswer(invocation -> {
      FeatureName featureId = invocation.getArgument(0);
      if ( featureId.toString().equals(id) )
      {
        return state;
      }
      return LicenseFeature.FEATURE_STATE.DISABLED;
    }).when(mockLicenseManager).checkFeature(any(FeatureName.class));
  }

  private void prepareAnswerForGetFeature( @NonNull final String id, @NonNull final LicenseFeature feature )
  {
    doAnswer(invocation -> {
      FeatureName featureId = invocation.getArgument(0);
      if ( featureId.toString().equals(id) )
      {
        return feature;
      }
      return null;
    }).when(mockLicenseManager).getFeature(any(FeatureName.class));
  }

  private void prepareAnswerForGetFeature( final List< String > FEATURE_NAMEs,
                                           final List< LicenseFeature > licenceFeatures )
  {
    doAnswer(invocation -> {
      FeatureName featureId = invocation.getArgument(0);
      int index = FEATURE_NAMEs.indexOf(featureId.toString());
      if ( index >= 0 && index < licenceFeatures.size() )
      {
        return licenceFeatures.get(index);
      }
      return null;
    }).when(mockLicenseManager).getFeature(any(FeatureName.class));
  }

  private void prepareAnswerForCheckFeature( final List< String > FEATURE_NAMEs,
                                             final LicenseFeature.FEATURE_STATE state )
  {
    doAnswer(invocation -> {
      FeatureName featureId = invocation.getArgument(0);
      if ( FEATURE_NAMEs.contains(featureId.toString()) )
      {
        return state;
      }
      return LicenseFeature.FEATURE_STATE.DISABLED;
    }).when(mockLicenseManager).checkFeature(new FeatureName("any"));
  }

  private LicenseFeature createAvailableFeature(@NonNull final String name)
  {
    return new LicenseFeature(createDisabledFeature(name), new SKU("id", "source", Currency.getInstance("USD"), 1));
  }

  private LicenseFeature createEnabledFeatureWithSkuEndDate(@NonNull final String name)
  {
    PeriodShdd period = PeriodShdd.parse("P1M");
    LicenseFeature licenseFeature =
        new LicenseFeature(createDisabledFeature(name), new SKU("id", "source", Currency.getInstance("USD"), 499, new Date(3016, 9, 26), period));
    return licenseFeature;
  }

  private LicenseFeature createEnabledFeature(@NonNull final String name)
  {
    return new LicenseFeature(createAvailableFeature(name), new Purchase("1", "1", 1, "1", "1", name));
  }

  private LicenseFeature createDisabledFeature(@NonNull final String name)
  {
    return new LicenseFeature(new FeatureName(name));
  }
}
