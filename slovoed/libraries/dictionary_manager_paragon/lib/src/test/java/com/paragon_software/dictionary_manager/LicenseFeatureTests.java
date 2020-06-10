package com.paragon_software.dictionary_manager;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.components.WordBaseComponent;
import com.paragon_software.dictionary_manager.exceptions.NotALocalResourceException;
import com.paragon_software.dictionary_manager.local_dictionary_library.LocalDictionaryLibrary;
import com.paragon_software.license_manager_api.LicenseFeature;
import com.paragon_software.license_manager_api.LicenseManager;
import com.paragon_software.license_manager_api.PeriodShdd;
import com.paragon_software.license_manager_api.SKU;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.stubs.MockTrialManager;
import com.paragon_software.trial_manager.TrialManagerAPI;
import com.paragon_software.user_core_manager.MockUserCoreManager;
import com.paragon_software.utils_slovoed.text.LocalizedString;

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
import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Currency;
import java.util.EnumSet;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import edu.emory.mathcs.backport.java.util.Arrays;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.withSettings;

@RunWith( RobolectricTestRunner.class )
public class LicenseFeatureTests
{
  @Rule
  public MockitoRule rule = MockitoJUnit.rule();

  private static final String SKU_SUFFIX = "_sku";

  private static final Dictionary.DictionaryId DICT1_ID = new Dictionary.DictionaryId("dict1");
  private static final Dictionary.DictionaryId DICT2_ID = new Dictionary.DictionaryId("dict2");

  private static final long PRICE                = 1000L;
  private static final long PRICE_SUBSCRIPTION_1 = 200L;
  private static final long PRICE_SUBSCRIPTION_2 = 500L;
  private static final long PRICE_SUBSCRIPTION_3 = 800L;

  @Mock
  private Context mContext;
  @Mock
  private Resources mResources;
  private DictionaryManagerAPI mDictionaryManager = null;
  @Mock
  private SettingsManagerAPI settingsManagerAPI;
  @Mock
  private SearchEngineAPI    searchEngine;
  @Mock
  private AssetManager       assetManager;
  @Mock
  private ScreenOpenerAPI    screenOpener;
  private String[] stringArray = new String[]{};

  @Before
  public void init() throws Exception
  {
    doReturn(mResources).when(mContext).getResources();
    doReturn(1).when(mResources).getIdentifier(Mockito.matches("catalog"), Mockito.anyString(), Mockito.anyString());
    doReturn(2).when(mResources).getIdentifier(Mockito.matches("sou"), Mockito.anyString(), Mockito.anyString());
    when(mResources.openRawResource(1)).thenReturn(new ByteArrayInputStream("".getBytes()));
    when(mResources.openRawResource(2)).thenReturn(new ByteArrayInputStream("<data><strings locale='English'><lang_translations dicts='English' dir_from='English' dir_to='English' local_name='English' name='English'/><dicts_pattern text='%s vocabularies'/></strings><lanuages_map></lanuages_map></data>".getBytes()));
    doReturn(assetManager).when(mContext).getAssets();
    doReturn(stringArray).when(assetManager).list(anyString());
    doReturn(new File("./")).when(mContext).getFilesDir();
    when(mContext.getPackageName()).thenReturn("com.paragon.test");
    when(mContext.getApplicationContext()).thenReturn(mContext);

    Dictionary.DictionaryId[] dictionaryIds = { DICT1_ID, DICT2_ID };
    DictionaryFactory factory = Mockito.mock(DictionaryFactory.class, withSettings()
            .defaultAnswer(CALLS_REAL_METHODS));
    List< FeatureName > features = getFeatureNamesByDictionaryComponents(dictionaryIds, factory);
    FeatureSKUProvider featureSKUProvider = Mockito.mock(FeatureSKUProvider.class, withSettings()
            .defaultAnswer(CALLS_REAL_METHODS));
    LicenseManager licenseManager = Mockito.mock(MockLicenseManager.class, withSettings()
            .useConstructor(featureSKUProvider, features).defaultAnswer(CALLS_REAL_METHODS));

    mDictionaryManager = getDmSync(licenseManager, mContext, factory, dictionaryIds);
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();
  }

  @NonNull
  private List< FeatureName > getFeatureNamesByDictionaryComponents( Dictionary.DictionaryId[] dictionaryIds,
                                                                     DictionaryFactory factory )
  {
    List< FeatureName > features = new ArrayList<>();

    for ( Dictionary d : factory.getDictionaries(mContext, dictionaryIds) )
    {
      for ( DictionaryComponent component : d.getDictionaryComponents() )
      {
        if ( component.isDemo() )
        {
          continue;
        }
        features.add(component.getFeatureName());
      }
    }
    return features;
  }

  private ParagonDictionaryManager getDmSync( @NonNull LicenseManager licenseManager, @NonNull final Context context,
                                              @NonNull IDictionaryManagerFactory factory,
                                              @NonNull Dictionary.DictionaryId[] dictionaryId ) throws Exception
  {
    final CountDownLatch syncLatch = new CountDownLatch(1);
    final DictionaryManagerAPI.IDictionaryListObserver observer = new DictionaryManagerAPI.IDictionaryListObserver()
    {
      @Override
      public void onDictionaryListChanged()
      {
        syncLatch.countDown();
      }
    };
    final DictionaryManagerAPI.IDictionaryListObserver[] observers =
        new DictionaryManagerAPI.IDictionaryListObserver[1];
    observers[0] = observer;

    TrialManagerAPI.Builder builder = Mockito.mock(MockTrialManager.Builder.class,
            withSettings().defaultAnswer(CALLS_REAL_METHODS));
    MockUserCoreManager userCoreManager = Mockito.spy(MockUserCoreManager.class);
    final ParagonDictionaryManager paragonDictionaryManager =
        new ParagonDictionaryManager(context, licenseManager, factory, settingsManagerAPI, searchEngine, dictionaryId,
                                     observers, new LocalDictionaryLibrary(context), builder,
                                     null, userCoreManager, screenOpener);
    try
    {
      syncLatch.await(2, TimeUnit.SECONDS);
    }
    catch ( InterruptedException ignore )
    {
    }
    return paragonDictionaryManager;
  }

  @Test
  public void testGetFeaturePrice() throws Exception
  {
    DictionaryPrice dictionaryPrice = new DictionaryPrice(PRICE, Currency.getInstance("USD"));
    assertEquals(dictionaryPrice, mDictionaryManager.getDictionaryPrice(DICT1_ID));
    assertEquals(dictionaryPrice, mDictionaryManager.getDictionaryPrice(DICT2_ID));

    List< DictionaryPrice > subscriptionPrices = mDictionaryManager.getSubscriptionPrices(DICT1_ID);
    assertNotNull(subscriptionPrices);
    assertEquals(3, subscriptionPrices.size());
    assertTrue(
        subscriptionPrices.contains(new DictionaryPrice(PRICE_SUBSCRIPTION_1, Currency.getInstance("USD"))));
    assertTrue(
        subscriptionPrices.contains(new DictionaryPrice(PRICE_SUBSCRIPTION_2, Currency.getInstance("USD"))));
    assertTrue(
        subscriptionPrices.contains(new DictionaryPrice(PRICE_SUBSCRIPTION_3, Currency.getInstance("USD"))));

    List< DictionaryPrice > subscriptionPrices1 = mDictionaryManager.getSubscriptionPrices(DICT2_ID);
    assertNotNull(subscriptionPrices1);
    assertEquals(3, subscriptionPrices1.size());
  }

  private abstract static class FeatureSKUProvider implements MockLicenseManager.FeatureSKUProvider
  {

    @NonNull
    @Override
    public LicenseFeature getLicenceFeatures( @NonNull FeatureName featureName )
    {
      // init InApp SKU
      LicenseFeature licenseFeature;
      licenseFeature =
          new LicenseFeature(new LicenseFeature(featureName), new SKU(featureName + SKU_SUFFIX, "SOURCE", Currency.getInstance("USD"), PRICE));
      // init subscriptions SKU
      for ( long price : new long[]{ PRICE_SUBSCRIPTION_1, PRICE_SUBSCRIPTION_2, PRICE_SUBSCRIPTION_3 } )
      {
        SKU sku = new SKU(featureName + SKU_SUFFIX + price, "SOURCE", Currency.getInstance("USD"), price, null, new PeriodShdd(0, 3, 0));
        licenseFeature = new LicenseFeature(licenseFeature, sku);
      }
      return licenseFeature;
    }
  }

  private abstract static class DictionaryFactory implements IDictionaryManagerFactory
  {

    @NonNull
    @Override
    public Dictionary[] getDictionaries( @NonNull Context context, @NonNull Dictionary.DictionaryId[] dictionaryIds )
    {
      Dictionary[] res = new Dictionary[dictionaryIds.length];
      int i = 0;
      for ( Dictionary.DictionaryId dictionaryId : dictionaryIds )
      {
        List< DictionaryComponent > components = Arrays.asList(new DictionaryComponent[] {
            new WordBaseComponent("", 1234, true, "", "", "", "5NNN", "100"),
            new WordBaseComponent("", 123456, false, "", "", "", "5NNN", "100"),
            });

        DictionaryIcon icon = Mockito.mock(DictionaryIcon.class, withSettings().defaultAnswer(CALLS_REAL_METHODS));
        res[i++] = ( new Dictionary.Builder(dictionaryId
                , LocalizedString.from("TITLE" + Integer.toString(i))
                , LocalizedString.from("DESCRIPTION"), icon) )
                .setStatus(Dictionary.DICTIONARY_STATUS.DEMO)
                .setMarketingData(new MarketingData(EnumSet.of(MarketingData.Category.BESTSELLER), Arrays.asList(new Shopping[]{
                        new Shopping(components.get(1).getFeatureName().toString() + SKU_SUFFIX, Collections.singletonMap(Currency.getInstance("USD"), 1L), null),
                        new Shopping(components.get(1).getFeatureName().toString() + SKU_SUFFIX + PRICE_SUBSCRIPTION_1, Collections.singletonMap(Currency.getInstance("USD"), 1L), null),
                        new Shopping(components.get(1).getFeatureName().toString() + SKU_SUFFIX + PRICE_SUBSCRIPTION_2, Collections.singletonMap(Currency.getInstance("USD"), 1L), null),
                        new Shopping(components.get(1).getFeatureName().toString() + SKU_SUFFIX + PRICE_SUBSCRIPTION_3, Collections.singletonMap(Currency.getInstance("USD"), 1L), null)
                }), false))
                .setDictionaryComponents(components).build();
      }
      return res;
    }

    @NonNull
    @Override
    public DictionaryPack[] getDictionaryPacks(@NonNull Context context, @NonNull Dictionary.DictionaryId[] dictionariesId) {
      return new DictionaryPack[0];
    }
  }

  private abstract static class DictionaryIcon implements IDictionaryIcon
  {

    @Nullable
    @Override
    public Bitmap getBitmap()
    {
      return null;
    }

    @Override
    public int getResourceId() throws NotALocalResourceException
    {
      return 0;
    }
  }
}
