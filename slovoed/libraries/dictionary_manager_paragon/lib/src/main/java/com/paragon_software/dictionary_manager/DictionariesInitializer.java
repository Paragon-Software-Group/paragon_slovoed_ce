package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.*;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.extbase.MorphoInfo;
import com.paragon_software.dictionary_manager.extbase.PictureInfo;
import com.paragon_software.dictionary_manager.extbase.SoundInfo;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.license_manager_api.*;

class DictionariesInitializer
{
  private LicenseManager             mLicenseManager;
  private DictionaryLocationResolver componentLocationResolver;

  DictionariesInitializer( LicenseManager licenseManager, DictionaryLocationResolver componentLocationResolver )
  {
    this.componentLocationResolver = componentLocationResolver;
    this.mLicenseManager = licenseManager;
  }

  Map<Dictionary.DictionaryId, List<DictionaryPrice>> prepareSubscriptionPrices( CatalogItem[] catalogItems ) {
    Map<Dictionary.DictionaryId, List<DictionaryPrice>> subscriptions = new HashMap<>(catalogItems.length);
    for (CatalogItem catalogItem : catalogItems) {
      LicenseFeature feature = null;
      if ( mLicenseManager != null)
        feature = mLicenseManager.getFeature(catalogItem.getPurchaseFeatureName());
      List<DictionaryPrice> subscriptionPrices = Collections.emptyList();
      if((feature != null) && (!feature.getState().equals(LicenseFeature.FEATURE_STATE.DISABLED)))
        subscriptionPrices = getCatalogItemPrices(catalogItem, true);
      if (!subscriptionPrices.isEmpty())
        subscriptions.put(catalogItem.getId(), subscriptionPrices);
    }
    return subscriptions;
  }

  @NonNull
  private List<DictionaryPrice> getCatalogItemPrices(CatalogItem catalogItem, boolean isSubscription)
  {
    List< DictionaryPrice > prices = new ArrayList<>();
    SKU[] sku = null;
    FeatureName featureName = catalogItem.getPurchaseFeatureName();
    if(!featureName.isEmpty()) {
        LicenseFeature feature = null;
        if ( mLicenseManager != null)
          feature = mLicenseManager.getFeature(featureName);
        if(feature != null)
          sku = feature.getSKU();
    }
    Set<String> itemSkus = new TreeSet<>();
    MarketingData marketingData = catalogItem.getMarketingData();
    if(marketingData != null)
        for(Shopping shopping : marketingData.getShoppings())
            itemSkus.add(shopping.getSku());
    if ( (null != sku) && (!itemSkus.isEmpty()) )
    {
      for ( SKU skuItem : sku )
      {
        if ( null != skuItem )
        {
          if ((skuItem.isSubscription() == isSubscription) && itemSkus.contains(skuItem.getId()))
          {
            DictionaryPrice price;
            if ( skuItem.isSubscription() )
            {
              DictionaryPrice.PeriodSubscription period = convertPeriod(skuItem.getPeriod());
              price =
                  new DictionaryPrice(skuItem.getPriceValue(), skuItem.getPriceCurrency(), period);
            }
            else
            {
              price = new DictionaryPrice(skuItem.getPriceValue(), skuItem.getPriceCurrency());
            }
            prices.add(price);
          }
        }
      }
    }
    return prices;
  }

  @NonNull
  private static List<DictionaryComponent> getDictionaryMorphoBaseComponents(Dictionary dictionary)
  {
    return getDictionaryComponentsByType(dictionary, DictionaryComponent.Type.MORPHO);
  }

  @NonNull
  private static List<DictionaryComponent> getDictionarySoundBaseComponents(Dictionary dictionary)
  {
    return getDictionaryComponentsByType(dictionary, DictionaryComponent.Type.SOUND);
  }

  @NonNull
  private static List<DictionaryComponent> getDictionaryPictureBaseComponents(Dictionary dictionary)
  {
    return getDictionaryComponentsByType(dictionary, DictionaryComponent.Type.PICT);
  }

  @NonNull
  private static List<DictionaryComponent> getDictionaryComponentsByType(Dictionary dictionary,
                                                                         @NonNull DictionaryComponent.Type type)
  {
    List <DictionaryComponent> components = new ArrayList<>();
    if (null != dictionary)
    {
      for ( DictionaryComponent component : dictionary.getDictionaryComponents() )
      {
        if ( type.equals(component.getType()) )
        {
          components.add(component);
        }
      }
    }
    return components;
  }

  /**
   * Convert common period to subscription period.
   * <p>Supported subscriptions periods: 1 weak, 1 month, 3 months, 6 months, 1 year</p>
   */
  private static DictionaryPrice.PeriodSubscription convertPeriod( PeriodShdd period )
  {
    int quantity = 0;
    DictionaryPrice.TypePeriodSubscription type = DictionaryPrice.TypePeriodSubscription.WEEK;
    if ( null == period )
    {
      return new DictionaryPrice.PeriodSubscription(quantity, type);
    }

    if ( period.getYears() > 0 )
    {
      type = DictionaryPrice.TypePeriodSubscription.YEAR;
      quantity = period.getYears();
    }
    else if ( period.getMonths() > 0 )
    {
      type = DictionaryPrice.TypePeriodSubscription.MONTH;
      quantity = period.getMonths();
    }
    else if ( period.getDays() > 0 && period.getDays() / PeriodShdd.DAYS_IN_WEAK > 0 )
    {
      type = DictionaryPrice.TypePeriodSubscription.WEEK;
      quantity = period.getDays() / PeriodShdd.DAYS_IN_WEAK;
    }
    return new DictionaryPrice.PeriodSubscription(quantity, type);
  }

  private static PeriodShdd convertPeriod( @NonNull DictionaryPrice.TypePeriodSubscription type, int quantity )
  {
    if ( quantity <= 0 )
    {
      throw new IllegalArgumentException(
          "Cant convert period for quantity less or equals zero. " + type + ", " + quantity);
    }

    switch ( type )
    {
      case WEEK:
        return new PeriodShdd(0, 0, quantity * PeriodShdd.DAYS_IN_WEAK);
      case MONTH:
        return new PeriodShdd(0, quantity, 0);
      case YEAR:
        return new PeriodShdd(quantity, 0, 0);
      default:
        return null;
    }
  }

  SKU getSubscriptionSku( @NonNull CatalogItem catalogItem,
                          @NonNull DictionaryPrice.PeriodSubscription subscriptionPeriod )
  {
    LicenseFeature feature = null;
    FeatureName featureName = catalogItem.getPurchaseFeatureName();
    if (!featureName.isEmpty() && mLicenseManager != null)
      feature = mLicenseManager.getFeature(featureName);
    Set<String> itemSkus = new TreeSet<>();
    MarketingData marketingData = catalogItem.getMarketingData();
    if (marketingData != null)
      for (Shopping shopping : marketingData.getShoppings())
        itemSkus.add(shopping.getSku());
    if ((null != feature) && (!itemSkus.isEmpty()))
    {
      SKU purchasedSku = feature.getSku(feature.getPurchase());
      boolean disabledAutoRenewingSubs = null != purchasedSku && purchasedSku.isSubscription() && !feature.getPurchase().isAutoRenewing();
      if ( LicenseFeature.FEATURE_STATE.AVAILABLE.equals(feature.getState()) || disabledAutoRenewingSubs)
      {
        PeriodShdd convertedPeriod = convertPeriod(subscriptionPeriod.type, subscriptionPeriod.quantity);
        SKU[] sku = feature.getSKU();
        if ( null != sku )
        {
          for ( SKU skuItem : sku )
          {
            if ((null != skuItem) && skuItem.isSubscription() && itemSkus.contains(skuItem.getId()))
            {
              //noinspection ConstantConditions
              if ( skuItem.getPeriod().equals(convertedPeriod) )
              {
                return skuItem;
              }
            }
          }
        }
      }
    }
    return null;
  }

  Map< Dictionary.DictionaryId, DictionaryPrice > prepareInAppPrices( @NonNull CatalogItem[] catalogItems )
  {
    HashMap< Dictionary.DictionaryId, DictionaryPrice > result = new HashMap<>(catalogItems.length);
    for (CatalogItem catalogItem : catalogItems)
    {
      catalogItem.setStatus(Dictionary.DICTIONARY_STATUS.DEMO);

      if (catalogItem instanceof Dictionary) {
        Dictionary dictionary = (Dictionary) catalogItem;
        if (dictionary.getDictionaryLocation() == null)
          assignLocationToDictionary(dictionary, true);
      }

      LicenseFeature feature = null;
      FeatureName featureName = catalogItem.getPurchaseFeatureName();
      if(!featureName.isEmpty() && mLicenseManager != null)
        feature = mLicenseManager.getFeature(featureName);
      if(feature != null) {
        LicenseFeature.FEATURE_STATE state = feature.getState();

        Dictionary.DICTIONARY_STATUS newStatus = Dictionary.DICTIONARY_STATUS.DEMO;
        catalogItem.setStatus(newStatus);

        if (catalogItem instanceof Dictionary) {
          Dictionary dictionary = (Dictionary) catalogItem;
          if (state.equals(LicenseFeature.FEATURE_STATE.ENABLED))
            assignLocationToDictionary(dictionary, false);
          if ((!state.equals(LicenseFeature.FEATURE_STATE.ENABLED)) || dictionary.getDictionaryLocation() == null)
            assignLocationToDictionary(dictionary, true);
        }

        if (!state.equals(LicenseFeature.FEATURE_STATE.DISABLED)) {
          List<DictionaryPrice> inAppPrices = getCatalogItemPrices(catalogItem, false);
          if (!inAppPrices.isEmpty())
            result.put(catalogItem.getId(), inAppPrices.get(0));
        }
      }
    }
    return result;
  }

  void assignLocationToDictionary( Dictionary dictionary, boolean demo )
  {
    DictionaryComponent component = dictionary.getDictionaryWordBaseComponent(demo);
    if ( null != component )
    {
      DictionaryLocation location = componentLocationResolver.getLocation(component);
      dictionary.setDictionaryLocation(location);
    }

    assignLocationToDictionaryMorphoBaseComponents(dictionary);
    assignLocationToDictionarySoundBaseComponents(dictionary);
    assignLocationToDictionaryPictureBaseComponents(dictionary);
  }

  private void assignLocationToDictionaryMorphoBaseComponents( Dictionary dictionary )
  {
    List <DictionaryComponent> morphoBaseComponents = getDictionaryMorphoBaseComponents(dictionary);
    List<MorphoInfo> morphoInfoList = new ArrayList<>();
    for (DictionaryComponent morphoComponent : morphoBaseComponents)
    {
      DictionaryLocation location = componentLocationResolver.getLocation(morphoComponent);
      if ( location != null )
      {
        morphoInfoList.add(new MorphoInfo(Language.getLangCodeFromShortForm(morphoComponent.getLangFrom()), location));
      }
    }
    dictionary.setMorphoInfoList(morphoInfoList);
  }

  private void assignLocationToDictionarySoundBaseComponents( Dictionary dictionary )
  {
    List<DictionaryComponent> soundBaseComponents = getDictionarySoundBaseComponents(dictionary);
    List<SoundInfo> soundInfoList = new ArrayList<>();
    List<SoundInfo> soundInfoListDemo = new ArrayList<>();
    for (DictionaryComponent soundComponent : soundBaseComponents)
    {
      DictionaryLocation location = componentLocationResolver.getLocation(soundComponent);
      if ( location != null )
      {
        if (soundComponent.isDemo()) {
          soundInfoListDemo.add(new SoundInfo(Language.getLangCodeFromShortForm(soundComponent.getLangFrom()), location, soundComponent.getSdcId()));
        } else {
          soundInfoList.add(new SoundInfo(Language.getLangCodeFromShortForm(soundComponent.getLangFrom()), location, soundComponent.getSdcId()));
        }
      }
    }
    for (SoundInfo soundInfoDemo : soundInfoListDemo) {
      boolean contains = false;
      for (SoundInfo soundInfo : soundInfoList) {
        if (soundInfo.getSdcId().equals(soundInfoDemo.getSdcId())) contains = true;
      }
      if (!contains) soundInfoList.add(soundInfoDemo);
    }
    dictionary.setSoundInfoList(soundInfoList);
  }

  private void assignLocationToDictionaryPictureBaseComponents( Dictionary dictionary )
  {
    List<DictionaryComponent> pictureBaseComponents = getDictionaryPictureBaseComponents(dictionary);
    List< PictureInfo > pictureInfoList = new ArrayList<>();
    List<PictureInfo> pictureInfoListDemo = new ArrayList<>();
    for (DictionaryComponent pictureComponent : pictureBaseComponents)
    {
      DictionaryLocation location = componentLocationResolver.getLocation(pictureComponent);
      if ( location != null )
      {
        if (pictureComponent.isDemo()) {
          pictureInfoListDemo.add(new PictureInfo(Language.getLangCodeFromShortForm(pictureComponent.getLangFrom()), location, pictureComponent.getSdcId()));
        } else {
          pictureInfoList.add(new PictureInfo(Language.getLangCodeFromShortForm(pictureComponent.getLangFrom()), location, pictureComponent.getSdcId()));
        }
      }
    }
    for (PictureInfo pictureInfoDemo : pictureInfoListDemo) {
      boolean contains = false;
      for (PictureInfo pictureInfo : pictureInfoList) {
        if (pictureInfo.getSdcId().equals(pictureInfoDemo.getSdcId())) contains = true;
      }
      if (!contains) pictureInfoList.add(pictureInfoDemo);
    }
    dictionary.setPictureInfoList(pictureInfoList);
  }

  SKU getInAppSku( @NonNull CatalogItem catalogItem )
  {
    LicenseFeature feature = null;
    FeatureName featureName = catalogItem.getPurchaseFeatureName();
    if (!featureName.isEmpty() && mLicenseManager != null)
      feature = mLicenseManager.getFeature(featureName);
    Set<String> itemSkus = new TreeSet<>();
    MarketingData marketingData = catalogItem.getMarketingData();
    if (marketingData != null)
      for (Shopping shopping : marketingData.getShoppings())
        itemSkus.add(shopping.getSku());
    if ((null != feature) && (!itemSkus.isEmpty()))
    {
      SKU purchasedSku = feature.getSku(feature.getPurchase());
      boolean disabledAutoRenewingSubs = null != purchasedSku && purchasedSku.isSubscription() && !feature.getPurchase().isAutoRenewing();
      if (LicenseFeature.FEATURE_STATE.AVAILABLE.equals(feature.getState()) || disabledAutoRenewingSubs)
      {
        SKU[] sku = feature.getSKU();
        if ( null != sku )
        {
          for ( SKU skuItem : sku )
          {
            if ((null != skuItem) && (!skuItem.isSubscription()) && itemSkus.contains(skuItem.getId()))
            {
              return skuItem;
            }
          }
        }
      }
    }
    return null;
  }

  Map<Dictionary.DictionaryId, SubscriptionPurchase> prepareSubscriptionPurchases(CatalogItem[] catalogItems)
  {
    Map< Dictionary.DictionaryId, SubscriptionPurchase > subscriptionPurchaseMap = new HashMap<>();
    for ( CatalogItem catalogItem : catalogItems )
    {
      FeatureName featureName = catalogItem.getPurchaseFeatureName();
      if ( !featureName.isEmpty() )
      {
        SubscriptionPurchase subscriptionPurchase = getSubscriptionPurchase(featureName);
        if ( null != subscriptionPurchase )
        {
          subscriptionPurchaseMap.put(catalogItem.getId(), subscriptionPurchase);
        }
      }
    }
    return subscriptionPurchaseMap;
  }

  @Nullable
  private SubscriptionPurchase getSubscriptionPurchase( FeatureName featureName )
  {
    SubscriptionPurchase subscriptionPurchase = null;
    LicenseFeature feature = null;
    if ( mLicenseManager != null)
      feature = mLicenseManager.getFeature(featureName);
    if ( null != feature )
    {
      Purchase purchase = feature.getPurchase();
      if ( null != purchase )
      {
        final SKU[] skus = feature.getSKU();
        if ( null != skus )
        {
          for ( SKU skuItem : skus )
          {
            if ( null != skuItem )
            {
              if ( purchase.getSkuId().equals(skuItem.getId()) && skuItem.isSubscription() && null != skuItem
                  .getPeriod() )
              {
                subscriptionPurchase =
                    new SubscriptionPurchase(purchase.getPurchaseTime(), purchase.isAutoRenewing(),
                                             skuItem.getPriceValue(), skuItem.getPriceCurrency(),
                                             convertPeriod(skuItem.getPeriod()), skuItem.getId());
              }
            }
          }
        }
      }
    }
    return subscriptionPurchase;
  }
}
