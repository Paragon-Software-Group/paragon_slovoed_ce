package com.paragon_software.dictionary_manager_factory;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import android.text.TextUtils;
import android.util.Pair;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.google.gson.stream.JsonReader;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryPack;
import com.paragon_software.dictionary_manager.Language;
import com.paragon_software.dictionary_manager.MarketingData;
import com.paragon_software.dictionary_manager.Shopping;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.trial_manager.TrialManagerAPI;
import com.paragon_software.utils_slovoed.map.MapUtils;
import com.paragon_software.utils_slovoed.text.LocalizedString;
import com.paragon_software.utils_slovoed.text.LocalizedStringImpl;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.ref.SoftReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Currency;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.regex.Matcher;

/**
 * Implementation of {@link CatalogDictionariesCreator} interface to create array of
 * {@link Dictionary} objects initialized by data from raw/catalog.json
 * and raw/descriptions.json files.
 */
public class BaseCatalogDictionariesCreator implements CatalogDictionariesCreator
{

  private static final String FULL_BASES_ASSET_PATH = "full";

  private static final String DEMO_BASES_ASSET_PATH = "demo";

  private static final String DICTIONARY_BASES_FILE_EXTENSION = ".sdc";

  /**
   * List of *.sdc files in file:///android_asset/full/ directory.
   */
  @NonNull
  private final Collection< String > fullAssetBases;
  /**
   * List of *.sdc files in file:///android_asset/demo/ directory.
   */
  @NonNull
  private final Collection< String > demoAssetBases;
  @NonNull
  private final Context mContext;

  public BaseCatalogDictionariesCreator( @NonNull Context context )
  {
    mContext = context;
    fullAssetBases = getListOfAssetFiles(context, FULL_BASES_ASSET_PATH);
    demoAssetBases = getListOfAssetFiles(context, DEMO_BASES_ASSET_PATH);
  }

  @Override
  public Dictionary[] createDictionaries()
  {
    final List< Dictionary > dictionariesList = new ArrayList<>();
    Catalog catalog = parseCatalog();
    if ( catalog != null )
    {
      Map< String, Map< String, String > > productsDescriptionsMap = parseDescriptions();
      for ( Map.Entry<String, Product> productEntry : catalog.getProductsMap().entrySet() )
      {
        Product product = productEntry.getValue();
        String productId = productEntry.getKey();

        if(!Product.COLLECTION.equals(product.getType())) {
          Dictionary.DictionaryId dictionaryId = productIdToDictionaryId(productId);
          ParagonDictionaryManagerFactory.DictionaryIcon dictionaryIcon = createDictionaryIcon(productId);
          Dictionary.DICTIONARY_STATUS edition = createDictionaryEdition(product);
          List<DictionaryComponent> dictionaryComponents = new ComponentsCreator().create(product);
          Dictionary.DICTIONARY_TYPE type = Dictionary.DICTIONARY_TYPE.fromName(product.getType());
          Collection<Dictionary.Direction> directions = createDictionaryDirections(product, edition, type);
          LocalizedString title = getLocalizedStringForProduct(product.getNamesMap(), catalog.getLocale().getDefault());
          LocalizedString description = getLocalizedStringForProduct(productsDescriptionsMap.get(productId), catalog.getLocale().getDefault());
          MarketingData marketingData = createMarketingData(product, productId);

          dictionariesList.add(new Dictionary.Builder(dictionaryId, title, description, dictionaryIcon)
                  .setMarketingData(marketingData)
                  .setDirections(directions)
                  .setType(type)
                  .setStatus(edition)
                  .setDictionaryComponents(dictionaryComponents)
                  .setIsRemovedFromSale(product.getRemovedFromSale())
                  .setIsDemoFts(product.getGenericData().isDemoFts())
                  .setIsPromiseFtsInDemo(product.getGenericData().isPromiseFTSinDemo())
                  .build());
        }
      }
    }
    return dictionariesList.toArray(new Dictionary[0]);
  }

  @Override
  public DictionaryPack[] createDictionaryPacks() {
    final List< DictionaryPack > dictionaryPacks = new ArrayList<>();
    Catalog catalog = parseCatalog();
    if ( catalog != null )
    {
      Map< String, Map< String, String > > productsDescriptionsMap = parseDescriptions();
      for ( Map.Entry<String, Product> productEntry : catalog.getProductsMap().entrySet() )
      {
        Product product = productEntry.getValue();
        String productId = productEntry.getKey();
        String fullpackInfo = product.getGenericData().getFullpack();
        if(Product.COLLECTION.equals(product.getType()) && !TextUtils.isEmpty(fullpackInfo)) {
          Matcher matcher = GenericData.FULLPACK_PATTERN.matcher(fullpackInfo);
          if(matcher.matches() && (matcher.groupCount()) >= 4) {
            String boughtRange = matcher.group(1);
            String productIds = matcher.group(4);
            if ((!TextUtils.isEmpty(boughtRange)) && (!TextUtils.isEmpty(productIds))) {
              int min, max;
              int dashIdx = boughtRange.indexOf('-');
              if(dashIdx < 0)
                min = max = parseInt(boughtRange);
              else {
                min = parseInt(boughtRange.substring(0, dashIdx));
                max = parseInt(boughtRange.substring(dashIdx + 1));
              }
              List<Dictionary.DictionaryId> productIdList = new LinkedList<>();
              for(String s : productIds.split(",")) {
                int id = parseInt(s);
                if(id >= 0)
                  productIdList.add(productIdToDictionaryId(Integer.toString(id)));
              }
              if((min >= 0) && (max >= 0) && (!productIdList.isEmpty())) {
                Dictionary.DictionaryId dictionaryId = productIdToDictionaryId(productId);
                ParagonDictionaryManagerFactory.DictionaryIcon dictionaryIcon = createDictionaryIcon(productId);
                LocalizedString title = getLocalizedStringForProduct(product.getNamesMap(), catalog.getLocale().getDefault());
                LocalizedString description = getLocalizedStringForProduct(productsDescriptionsMap.get(productId), catalog.getLocale().getDefault());
                MarketingData marketingData = createMarketingData(product, productId);
                dictionaryPacks.add(new DictionaryPack(dictionaryId, title, description,
                        dictionaryIcon, Dictionary.DICTIONARY_STATUS.DEMO, marketingData, Boolean.TRUE.equals(product.getRemovedFromSale()),
                        min, max, productIdList));
              }
            }
          }
        }
      }
    }
    return dictionaryPacks.toArray(new DictionaryPack[0]);
  }

  @Override
  public void addFeaturesToTrialBuilder(@NonNull TrialManagerAPI.Builder trialBuilder,
                                        @NonNull Map< Dictionary.DictionaryId, Dictionary > allDictionaries) {
    Catalog catalog = parseCatalog();
    int trialLengthInMinutes = 0;
    if ( catalog != null) {
      if (catalog.getTrialLengthInDays() != null)
        trialLengthInMinutes = Integer.parseInt(catalog.getTrialLengthInDays()) * 24 * 60;

      for (Map.Entry<String, Product> productEntry : catalog.getProductsMap().entrySet()) {
        String productId = productEntry.getKey();
        Dictionary.DictionaryId dictionaryId = productIdToDictionaryId(productId);
        Dictionary dictionary = allDictionaries.get(dictionaryId);
        if(dictionary != null)
          for(DictionaryComponent dictionaryComponent : dictionary.getDictionaryComponents())
            if(dictionaryComponent != null)
              if(DictionaryComponent.Type.WORD_BASE.equals(dictionaryComponent.getType())) {
                trialBuilder.addFeature(dictionaryComponent.getFeatureName(), dictionary.getTitle(), trialLengthInMinutes, Long.valueOf(productId), null, null);
                break;
              }
      }
    }
  }

  private static int parseInt(String s) {
    try {
      if(TextUtils.isEmpty(s))
        throw new NumberFormatException();
      s = s.trim();
      if(TextUtils.isEmpty(s))
        throw new NumberFormatException();
      return Integer.parseInt(s);
    }
    catch (NumberFormatException e) {
      return -1;
    }
  }

  @NonNull
  private static Collection<String> getListOfAssetFiles( Context context, String dir ) {
    Collection<String> res = Collections.emptySet();
    try {
      String[] list = context.getResources().getAssets().list(dir);
      if(list != null)
        res = new TreeSet<>(Arrays.asList(list));
    }
    catch ( IOException | NullPointerException ignore ) { }
    return res;
  }

  @NonNull
  private SoftReference<Catalog> __catalog = new SoftReference<>(null);

  @Nullable
  private Catalog parseCatalog() {
    Catalog res = __catalog.get();
    if(res == null) {
        res = parseRawJsonFile("catalog", new TypeToken<Catalog>() { } );
        __catalog = new SoftReference<>(res);
    }
    return res;
  }

  @NonNull
  private SoftReference<Map<String, Map<String, String>>> __descriptions = new SoftReference<>(null);

  @NonNull
  private Map< String, Map< String, String > > parseDescriptions()
  {
    Map< String, Map< String, String > > productsDescriptionsMap = __descriptions.get();
    if(productsDescriptionsMap == null) {
      productsDescriptionsMap = parseRawJsonFile("descriptions", new TypeToken<Map<String, Map<String, String>>>() { });
      if(productsDescriptionsMap == null)
        productsDescriptionsMap = new HashMap< String, Map< String, String > >();
      __descriptions = new SoftReference<>(productsDescriptionsMap);
    }
    return productsDescriptionsMap;
  }

  private @Nullable <T> T parseRawJsonFile( String fileName, @NonNull TypeToken<T> typeToken )
  {
    T result = null;
    int fileResId = mContext.getResources().getIdentifier(fileName, "raw",
                                                             mContext.getPackageName());
    if ( fileResId != 0 )
    {
      InputStream fileInputStream = mContext.getResources().openRawResource(fileResId);
      JsonReader fileJsonReader = new JsonReader(new InputStreamReader(fileInputStream));
      result = new Gson().fromJson(fileJsonReader, typeToken.getType());
      try
      {
        fileJsonReader.close();
      }
      catch ( IOException exception )
      {
        exception.printStackTrace();
      }
    }
    return result;
  }

  private ParagonDictionaryManagerFactory.DictionaryIcon createDictionaryIcon( String productId )
  {
    ParagonDictionaryManagerFactory.DictionaryIcon dictionaryIcon = null;
    int iconId = mContext.getResources().getIdentifier("product_" + productId,
                                                       "drawable", mContext.getPackageName());
    if ( iconId != 0 )
    {
      dictionaryIcon = new ParagonDictionaryManagerFactory.DictionaryIcon(iconId);
    }
    return dictionaryIcon;
  }

  private LocalizedString getLocalizedStringForProduct(final Map<String, String> catalogStringsMap, final String defaultLanguage )
  {
    return new LocalizedStringImpl<String>() {
      @NonNull
      @Override
      public String get() {
        return super.get()
                .replace("&amp;", "&")
                .replace("&lt;", "<")
                .replace("&gt;", ">")
                .replace("&quot;", "\"")
                .replace("&apos;", "\'");
      }

      @Nullable
      @Override
      protected String getCurrentLocale() {
        return Locale.getDefault().getLanguage();
      }

      @Nullable
      @Override
      protected String getDefaultLocale() {
        return defaultLanguage;
      }

      @Nullable
      @Override
      protected String getEnglishLocale() {
        return "en";
      }

      @Nullable
      @Override
      protected String getStringByKey(@NonNull String key) {
        String res = null;
        if(!MapUtils.isEmpty(catalogStringsMap))
          res = catalogStringsMap.get(key);
        return res;
      }

      @Nullable
      @Override
      protected String getAnyString() {
        String res = null;
        if(!MapUtils.isEmpty(catalogStringsMap))
          res = catalogStringsMap.values().iterator().next();
        return res;
      }
    };
  }

  @NonNull
  private Dictionary.DICTIONARY_STATUS createDictionaryEdition( @NonNull Product product )
  {
    Dictionary.DICTIONARY_STATUS edition = Dictionary.DICTIONARY_STATUS.DEMO;
//    else if ( demoAssetBases.contains(product.getComponents().getDemoDict().getPrcId() + DICTIONARY_BASES_FILE_EXTENSION) )
//    {
//      edition = Dictionary.DICTIONARY_STATUS.DEMO;
//    }
    return edition;
  }

  @NonNull
  private Collection<Dictionary.Direction> createDictionaryDirections( @NonNull Product product,
                                                                       @NonNull Dictionary.DICTIONARY_STATUS edition,
                                                                       @NonNull Dictionary.DICTIONARY_TYPE type ) {
    List<Dictionary.Direction> directions = new ArrayList<>(2);
    Pair<Integer, Integer> direction = createDictionaryDirection(product, edition);
    if ( type.equals(Dictionary.DICTIONARY_TYPE.BILINGUAL) ) {
      directions.add(createDictionaryDirection(direction.first, direction.second));
      directions.add(createDictionaryDirection(direction.second, direction.first));
    }
    else
      directions.add(createDictionaryDirection(direction.first, direction.second));
    return directions;
  }

  @NonNull
  private Pair<Integer, Integer> createDictionaryDirection(@NonNull Product product,
                                                           @NonNull Dictionary.DICTIONARY_STATUS edition ) {
    Pair<String, String> res;
    Dict dict = product.getComponents().getDemoDict();
    if ( dict != null )
      res = new Pair<>(dict.getLangFromShort(), dict.getLangToShort());
    else
      res = new Pair<>(product.getLangFromShort(), product.getLangToShort());
    return new Pair<>(Language.getLangCodeFromShortForm(res.first), Language.getLangCodeFromShortForm(res.second));
  }

  @NonNull
  private Dictionary.Direction createDictionaryDirection(int from, int to) {
    return new Dictionary.Direction(from, to, ParagonDictionaryManagerFactory.createDirectionIcon(from, to));
  }


  @NonNull
  private MarketingData createMarketingData( Product product, String productId )
  {
    Set< MarketingData.Category > categories = EnumSet.noneOf(MarketingData.Category.class);
    // TODO : for bestsellerLanguages need to handle situation when user locale has changed in runtime (when dictionary_manager has already initialized)
    Collection<String> bestsellerLanguages = product.getBestsellerLanguages();
    if ( bestsellerLanguages != null && bestsellerLanguages.contains(Locale.getDefault().getLanguage()) )
    {
      categories.add(MarketingData.Category.BESTSELLER);
    }
    if ( product.isNew() )
    {
      categories.add(MarketingData.Category.NEW);
    }
    // TODO : check 'featured' property, it may be defined differently in catalog.json than I suppose
    if ( product.isFeatured() )
    {
      System.out.println("FEATURED");
      categories.add(MarketingData.Category.FEATURED);
    }
    List< Shopping > shoppings = createShoppings(product);
    checkShoppings(shoppings, productId);
    return new MarketingData(categories, shoppings, isContentFree(product.getGenericData().getContent()));
  }

  /**
   * For each product: <br/>
   * - one sku for inApp <br/>
   */
  private void checkShoppings( List< Shopping > shoppings, String productId )
  {
    List< String > inAppSkuList = new ArrayList<>();
    for ( Shopping shopping : shoppings )
    {
      if ( !shopping.isSubscription() )
      {
        StringBuilder priceStr = new StringBuilder();
        Map< Currency, Long > price = shopping.getPrice();
        for ( Currency currency : price.keySet() )
        {
          double humanReadablePrice = ( (double) price.get(currency) ) / 1000000D;
          priceStr.append(humanReadablePrice).append(currency).append(" ");
        }
        inAppSkuList.add(shopping.getSku() + "(" + priceStr.toString().trim() + ")");
      }
    }
    if ( inAppSkuList.size() > 1 )
    {
      System.out.println("Product : " + productId + " has more that one inApp sku : " + inAppSkuList);
    }
  }

  @NonNull
  private List< Shopping > createShoppings( Product product )
  {
    if (product.getShoppings().isEmpty() && (!isContentFree(product.getGenericData().getContent())))
    {
      throw new IllegalArgumentException("Shoppings for product cannot be empty");
    }

    Map<String, List< com.paragon_software.dictionary_manager_factory.Shopping>> groupped;
    groupped = com.paragon_software.dictionary_manager_factory.Shopping.groupByIdInShop(product.getShoppings());

    List< Shopping > shoppingForDictionaryList = new ArrayList<>();
    for ( String idInShop : groupped.keySet() )
    {
      Map< Currency, Long > price = com.paragon_software.dictionary_manager_factory.Shopping.toPriceMap(groupped.get(idInShop));
      Integer subscriptionPeriod = com.paragon_software.dictionary_manager_factory.Shopping.parseSubscriptionPeriod(groupped.get(idInShop).get(0));
      shoppingForDictionaryList.add(new Shopping(idInShop, price, subscriptionPeriod));
    }
    return shoppingForDictionaryList;
  }

  private static boolean isContentFree(@Nullable Collection<String> content) {
    boolean res = false;
    if (content != null)
      res = content.contains(GenericData.FREE);
    return res;
  }

  @NonNull
  private static Dictionary.DictionaryId productIdToDictionaryId(String productId) {
    return new Dictionary.DictionaryId(productId);
  }
}
