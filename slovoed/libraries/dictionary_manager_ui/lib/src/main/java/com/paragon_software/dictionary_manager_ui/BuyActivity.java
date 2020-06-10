/*
 * dictionary manager ui
 *
 *  Created on: 16.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager_ui;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.Html;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.recyclerview.widget.RecyclerView;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.BuyClickAfterExpiredTrialNotification;
import com.paragon_software.analytics_manager.events.OpenAppClickExpiredTrialNotification;
import com.paragon_software.analytics_manager.events.ScreenName;
import com.paragon_software.analytics_manager.events.TrialScreen;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.DictionaryPrice;
import com.paragon_software.dictionary_manager.DictionaryPrice.TypePeriodSubscription;
import com.paragon_software.dictionary_manager.FeatureName;
import com.paragon_software.dictionary_manager.SubscriptionPurchase;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.errors.ErrorType;
import com.paragon_software.dictionary_manager.errors.OperationType;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.utils_slovoed_ui.StringsUtils;
import com.paragon_software.utils_slovoed_ui_common.activities.ParagonFragmentActivity;
import com.paragon_software.utils_slovoed_ui_common.fragments.SimpleDialog;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public final class BuyActivity extends ParagonFragmentActivity implements View.OnClickListener, BuyClickListener,
                                                                          DictionaryManagerAPI.IDictionaryListObserver,
                                                                          DictionaryManagerAPI.IErrorObserver,
                                                                          SubscriptionExplanation.UserAction,
                                                                          SimpleDialog.Target
{
  public static final  String STATE_LOAD_LICENCES_ON_RESUME =
      BuyActivity.class.getName() + ".STATE_LOAD_LICENCES_ON_RESUME";
  public static final  String STATE_SUBS_EXPLANATION        = BuyActivity.class.getName() + ".STATE_SUBS_EXPLANATION";
  private static final String ACTIVATION_END_BUNDLE_ARG     =
      BuyActivity.class.getName() + ".ACTIVATION_END_BUNDLE_ARG";
  private static final int    REQ_CODE_ACTIVATION_END       = 1919;
  private static final String ACTIVATION_END_ERROR_TYPE     =
      BuyActivity.class.getName() + ".ACTIVATION_END_ERROR_TYPE";
  private static final String CONFIRMATION_DIALOG_TAG = BuyActivity.class.getName() + ".ConfirmationDialog";

  @Nullable
  private DictionaryManagerAPI    mDictionaryManager = null;
  private Dictionary.DictionaryId mDictionaryId      = null;
  private Serializable mArticleItem;
  private boolean loadLicencesOnResume                = false;
  private boolean subscriptionExplanationDialogShowed = false;
  private boolean mOpenFromTrialNotification          = false;

  @Override
  protected void onCreate( @Nullable Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    mDictionaryManager = DictionaryManagerHolder.getManager();

    if (null != savedInstanceState)
    {
      loadLicencesOnResume = savedInstanceState.getBoolean(STATE_LOAD_LICENCES_ON_RESUME, false);
      subscriptionExplanationDialogShowed = savedInstanceState.getBoolean(STATE_SUBS_EXPLANATION, false);
    }
    ActionBar actionBar = getSupportActionBar();
    if ( actionBar != null )
    {
      actionBar.setDisplayHomeAsUpEnabled(true);
      actionBar.setTitle("");
    }
    if(parseIntentAndInit(getIntent(), true)) {
      mDictionaryManager.registerDictionaryListObserver(this);
      mDictionaryManager.registerErrorObserver(this);
      mDictionaryManager.loadOnlineDictionaryStatusInformation(this);
    }
  }

  @Override
  protected void onNewIntent(Intent intent) {
    super.onNewIntent(intent);
    parseIntentAndInit(intent, false);
  }

  @Override
  protected void onResume()
  {
    super.onResume();
    if (loadLicencesOnResume)
    {
      loadLicencesOnResume = false;
      mDictionaryManager.loadOnlineDictionaryStatusInformation(this);
    }
  }

  private boolean parseIntentAndInit(@Nullable Intent intent, boolean needToSetContentView) {
    Dictionary dictionary = null;
    if(intent != null) {
      mDictionaryId = mDictionaryManager.getDictionaryIdFromTrialNotificationIntent(intent);
      if (null != mDictionaryId)
      {
        mOpenFromTrialNotification = true;
        AnalyticsManagerAPI.get().logEvent(new OpenAppClickExpiredTrialNotification(mDictionaryId.toString()));
      }
      else
      {
        mOpenFromTrialNotification = false;
      }
      mArticleItem = null;
      if(mDictionaryId == null) {
        mArticleItem = intent.getSerializableExtra(DictionaryManagerAPI.UI_EXTRA_ARTICLE_ITEM_KEY);
        mDictionaryId = intent.getParcelableExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY);
      }
      if (mDictionaryId != null) {
        Collection<Dictionary> dictionaries = mDictionaryManager.getDictionaries();
        for (Dictionary d : dictionaries)
          if (mDictionaryId.equals(d.getId())) {
            dictionary = d;
            break;
          }
      }
    }
    boolean res = (dictionary != null);
    if ( res ) {
      if(needToSetContentView)
        setContentView(R.layout.buy_activity);
      populateViews(dictionary);
      showExplanationForSubscriptions(dictionary);
    }
    else
      finish();
    return res;
  }

  private void showExplanationForSubscriptions( Dictionary dictionary )
  {
    SubscriptionPurchase purchase = mDictionaryManager.getSubscriptionPurchase(dictionary);
    if (!subscriptionExplanationDialogShowed && null != purchase)
    {
      if (purchase.isAutoRenewing())
      {
        SubscriptionExplanation.showDialog(getSupportFragmentManager(), purchase, getPackageName());
        subscriptionExplanationDialogShowed = true;
      }
    }
  }

  private void populateViews( Dictionary dictionary )
  {
    TextView dictionaryTitle = findViewById(R.id.dictionary_description_label_text_view);
    TextView dictionaryDescription = findViewById(R.id.dictionary_description_content_text_view);
    ImageView dictionaryIcon = findViewById(R.id.dictionary_icon_image_view);
    Button previewButton = findViewById(R.id.preview_dictionary_button);
    previewButton.setText(
        StringsUtils.createTrialLengthString(this, mDictionaryManager.getTrialLengthInMinutes(dictionary.getId())));
    Button sampleButton = findViewById(R.id.sample);
    Button openButton = findViewById(R.id.dictionary_open_button);
    Button openMyDictionariesButton = findViewById(R.id.go_to_my_dictionaries_button);
    RecyclerView subscriptionList = findViewById(R.id.subscribe_button_list);
    dictionaryTitle.setText(dictionary.getTitle().get());
    dictionaryDescription.setText(Html.fromHtml(dictionary.getDescription().get()));
    Utils.setDictionaryIcon(dictionaryIcon, dictionary.getIcon());
    previewButton.setOnClickListener(this);
    sampleButton.setOnClickListener(this);
    openButton.setOnClickListener(this);
    openMyDictionariesButton.setOnClickListener(this);
    List< DictionaryPrice > subscriptionPrices = getSubscriptionPrices(dictionary);
    subscriptionPrices = excludePurchased(subscriptionPrices, mDictionaryManager.getSubscriptionPurchase(dictionary));
    subscriptionPrices = sortByPrice(subscriptionPrices);
    subscriptionList.setAdapter(new SubscriptionAdapter(subscriptionPrices, this));
    subscriptionList.setNestedScrollingEnabled(false);
    View buyDictionaryButton = findViewById(R.id.buy_dictionary_button);
    DictionaryPrice dictionaryPrice = mDictionaryManager.getDictionaryPrice(dictionary.getId());
    if ( null != dictionaryPrice )
    {
      buyDictionaryButton.setOnClickListener(createBuyButtonClickListener(dictionaryPrice));
      assignTextToBuyButton(buyDictionaryButton, dictionaryPrice);
    }
    else
    {
      buyDictionaryButton.setOnClickListener(createBuyButtonClickListenerBillingUnavailable());
    }
    adjustButtonsVisibility(dictionary, previewButton, sampleButton, subscriptionList, buyDictionaryButton, openButton, openMyDictionariesButton);
  }

  private void adjustButtonsVisibility( Dictionary dictionary, Button previewButton, Button sampleButton,
                                        RecyclerView subscriptionList, View buyDictionaryButton, Button openButton,
                                        Button openMyDictionariesButton)
  {
    Dictionary.DICTIONARY_STATUS status = dictionary.getStatus();
    boolean isDownloaded = isDownloaded(dictionary);
    boolean canFullBaseBeUsed = false;
    boolean isTrialAvailable = mDictionaryManager.isTrialAvailable(dictionary.getId());
    SubscriptionPurchase purchase = mDictionaryManager.getSubscriptionPurchase(dictionary);
    previewButton.setVisibility((canFullBaseBeUsed || (!isTrialAvailable)) ? View.GONE : View.VISIBLE);
    sampleButton.setVisibility(canFullBaseBeUsed ? View.GONE : View.VISIBLE);
    openButton.setVisibility((canFullBaseBeUsed && isDownloaded) ? View.VISIBLE : View.GONE);
    openMyDictionariesButton.setVisibility((canFullBaseBeUsed && !isDownloaded) ? View.VISIBLE : View.GONE);
    boolean subscriptionButtonsVisible = status.canBePurchased();
    subscriptionList.setVisibility(subscriptionButtonsVisible ? View.VISIBLE : View.GONE);
    boolean buyButtonVisibility =
        null != mDictionaryManager.getDictionaryPrice(dictionary.getId()) && status.canBePurchased();
    buyDictionaryButton.setVisibility(buyButtonVisibility ? View.VISIBLE : View.GONE);
  }

  private boolean isDownloaded( Dictionary dictionary )
  {
    DictionaryControllerAPI controller = mDictionaryManager.createController(null);
    if ( null != controller )
    {
      DictionaryComponent fullWordBaseComponent = getWordBaseComponent(dictionary);
      if ( null != fullWordBaseComponent )
      {
        return controller.isDownloaded(fullWordBaseComponent);
      }
    }
    return false;
  }

  private DictionaryComponent getWordBaseComponent( Dictionary dictionary )
  {
    for ( DictionaryComponent component : dictionary.getDictionaryComponents() )
    {
      if ( !component.isDemo() && DictionaryComponent.Type.WORD_BASE.equals(component.getType()) )
      {
        return component;
      }
    }
    return null;
  }

  private List< DictionaryPrice > excludePurchased( @NonNull List< DictionaryPrice > subscriptionPrices,
                                                    @Nullable SubscriptionPurchase purchase )
  {
    if (null != purchase && purchase.isAutoRenewing())
    {
      List<DictionaryPrice> prices = new ArrayList<>(subscriptionPrices);
      for ( DictionaryPrice price : subscriptionPrices )
      {
        if (purchase.equalsPriceAndPeriod(price))
        {
          prices.remove(price);
        }
      }
      return prices;
    }
    else
    {
      return subscriptionPrices;
    }
  }

  private View.OnClickListener createBuyButtonClickListenerBillingUnavailable()
  {
    return new View.OnClickListener()
    {
      @Override
      public void onClick( View view )
      {
        showErrorMessage(getString(R.string.dictionary_manager_ui_unavailable_connection_with_market));
      }
    };
  }

  @Override
  protected void onDestroy()
  {
    super.onDestroy();
    mDictionaryManager.unRegisterDictionaryListObserver(this);
    mDictionaryManager.unRegisterErrorObserver(this);
  }

  private List< DictionaryPrice > sortByPrice( List< DictionaryPrice > subscriptionPrices )
  {
    ArrayList< DictionaryPrice > result = new ArrayList<>(subscriptionPrices);
    Comparator< ? super DictionaryPrice > comparator = new Comparator< DictionaryPrice >()
    {
      @Override
      public int compare( DictionaryPrice right, DictionaryPrice left )
      {
        return (int) (right.getPrice() - left.getPrice());
      }
    };
    Collections.sort(result, comparator);
    return result;
  }

  private void assignTextToBuyButton( @NonNull View button, @NonNull DictionaryPrice dictionaryPrice )
  {
    if ( !( button instanceof TextView ) )
    {
      return;
    }

    String priceWithCurrency =
        Utils.convertPriceToString(dictionaryPrice.getPrice()) + dictionaryPrice.getCurrency().getSymbol();
    ( (TextView) button ).setText(getString(R.string.dictionary_manager_ui_buy_button_text_with_price, priceWithCurrency));
  }

  private View.OnClickListener createBuyButtonClickListener( @NonNull final DictionaryPrice dictionaryPrice )
  {
    return new View.OnClickListener()
    {
      PreventDoubleClick preventDoubleClick = new PreventDoubleClick();
      @Override
      public void onClick( View view )
      {
        if ( preventDoubleClick.isDoubleClick() )
        {
          return;
        }
        onBuyClick(dictionaryPrice.getPeriod());
      }
    };
  }

  private List< DictionaryPrice > getSubscriptionPrices( Dictionary dictionary )
  {
    List< DictionaryPrice > subscriptionPrices = mDictionaryManager.getSubscriptionPrices(dictionary.getId());
    return null == subscriptionPrices ? new ArrayList< DictionaryPrice >(0) : subscriptionPrices;
  }

  @Override
  public boolean onOptionsItemSelected( MenuItem item )
  {
    boolean res;
    switch ( item.getItemId() )
    {
      case android.R.id.home:
        onBackPressed();
        res = true;
        break;

      default:
        res = super.onOptionsItemSelected(item);
        break;
    }
    return res;
  }

  @Override
  public void onClick( View v )
  {
    if ( v.getId() == R.id.preview_dictionary_button )
    {
      AnalyticsManagerAPI.get().logEvent(new TrialScreen(ScreenName.PRODUCT_DESCRIPTION_AND_PRICES_SCREEN));
      requireConsentDialogHandler().showDialog(this, mDictionaryId);
    }
    else if ( v.getId() == R.id.sample || v.getId() == R.id.dictionary_open_button )
    {
      new RecentlyViewedHandler(getApplicationContext()).addRecentlyOpened(mDictionaryId, mDictionaryManager);
      ScreenOpenerAPI screenOpener = (ScreenOpenerAPI) mDictionaryManager.getScreenOpener();
      if ( mArticleItem instanceof ArticleItem && screenOpener != null )
      {
        screenOpener.showArticleActivity((ArticleItem) mArticleItem, this);
      }
      else
      {
        mDictionaryManager.openDictionaryForSearch(this, mDictionaryId, null, null);
      }
    }
    else if (v.getId() == R.id.go_to_my_dictionaries_button)
    {
      mDictionaryManager.openMyDictionariesUI(this, mDictionaryId);
    }
  }

  @Override
  public void onBuyClick( DictionaryPrice.PeriodSubscription subscriptionPeriod )
  {
    ErrorType errorType = mDictionaryManager.buyCatalogItem((Activity) this, mDictionaryId, subscriptionPeriod);
    if ( mOpenFromTrialNotification && null != mDictionaryId && ErrorType.OK.equals(errorType) )
    {
      AnalyticsManagerAPI.get().logEvent(new BuyClickAfterExpiredTrialNotification(mDictionaryId.toString()));
    }
    showErrorMessage(errorType);
  }

  private void showErrorMessage( String message )
  {
    DialogFragmentError.showDialog(getSupportFragmentManager(), message);
  }

  @Override
  public void onDictionaryListChanged()
  {
    if ( null != mDictionaryId )
    {
      boolean dictIdAvailable = false;
      for ( Dictionary dictionary : mDictionaryManager.getDictionaries() )
      {
        if ( dictionary.getId().equals(mDictionaryId) )
        {
          dictIdAvailable = true;
          populateViews(dictionary);
          break;
        }
      }
      if ( !dictIdAvailable )
      {
        this.finish();
      }
    }
  }

  @Override
  protected void onActivityResult( int requestCode, int resultCode, Intent data )
  {
    ErrorType errorType;
    if(data==null)
    {
      errorType =  ErrorType.UNDEFINED;
    }
    else {
      errorType = mDictionaryManager.activationEnd(requestCode, resultCode, data);
    }
    if ( errorType != null )
    {
      Bundle bundle = new Bundle();
      bundle.putSerializable(ACTIVATION_END_ERROR_TYPE, errorType);
      PendingIntent pi = createFragmentSafePendingIntent(this, REQ_CODE_ACTIVATION_END);
      Intent intent = new Intent();
      intent.putExtra(ACTIVATION_END_BUNDLE_ARG, bundle);
      try
      {
        pi.send(this, 0, intent);
      }
      catch ( PendingIntent.CanceledException ignore )
      {
      }
    }
    else
    {
      super.onActivityResult(requestCode, resultCode, data);
    }
  }

  @Override
  protected void onFragmentResult( int requestCode, int resultCode, Intent data )
  {
    if ( REQ_CODE_ACTIVATION_END == requestCode && null != data )
    {
      Bundle bundleExtra = data.getBundleExtra(ACTIVATION_END_BUNDLE_ARG);
      if ( null != bundleExtra )
      {
        ErrorType errorType = (ErrorType) bundleExtra.getSerializable(ACTIVATION_END_ERROR_TYPE);
        if ( null != errorType )
        {
          handleLicenceManagerError(errorType);
        }
      }
    }
  }

  private void handleLicenceManagerError( @NonNull ErrorType errorType )
  {
    if ( ErrorType.OK.equals(errorType) )
    {
      mDictionaryManager.openMyDictionariesUI(this, mDictionaryId);
    }
    else
    {
      showErrorMessage(errorType);
    }
  }

  private void showErrorMessage( @NonNull ErrorType errorType ) {
    switch ( errorType ) {
      case OK:
      case PURCHASE_USER_CANCEL:
        break;
      case UNDEFINED:
        showErrorMessage(getString(R.string.dictionary_manager_ui_undefined_billing_error));
        break;
      case PURCHASE_ITEM_ALREADY_OWNED:
        showErrorMessage(getString(R.string.dictionary_manager_ui_purchase_item_already_owned));
        break;
      case PURCHASE_ITEM_UNAVAILABLE:
        showErrorMessage(getString(R.string.dictionary_manager_ui_purchase_item_unavailable));
        break;
      case BILLING_UNAVAILABLE:
        showErrorMessage(getString(R.string.dictionary_manager_ui_unavailable_connection_with_market));
        break;
      default:
        showErrorMessage(getString(R.string.dictionary_manager_ui_undefined_billing_error));
        break;
    }
  }

  @Override
  public void onError( @NonNull OperationType operationType, @NonNull List< ErrorType > error )
  {
//    Log.e("shdd", "BA.onError() called with: operationType = [" + operationType + "], error = [" + error + "]");
  }

  @Override
  public void onUserGoDeepLinkForManagingSubscription( @NonNull SubscriptionPurchase purchase, @NonNull String packageName )
  {
    Uri uri = Uri.parse("https://play.google.com/store/account/subscriptions?sku=" + purchase.getSkuId() + "&package=" + packageName);
    Intent intent = new Intent();
    intent.setAction(Intent.ACTION_VIEW);
    intent.setData(uri);
    startActivity(intent);
    loadLicencesOnResume = true;
  }

  @Override
  protected void onSaveInstanceState( Bundle outState )
  {
    super.onSaveInstanceState(outState);
    outState.putBoolean(STATE_LOAD_LICENCES_ON_RESUME, loadLicencesOnResume);
    outState.putBoolean(STATE_SUBS_EXPLANATION, subscriptionExplanationDialogShowed);
  }

  @Override
  public void onCreateSimpleDialog(@Nullable String tag, @NonNull Dialog dialog, @Nullable Bundle extra) {
    requireConsentDialogHandler().onCreateSimpleDialog(tag, dialog, extra);
  }

  @Override
  public void onSimpleDialogButtonPressed(@Nullable String tag, int n, @Nullable Bundle extra) {
    requireConsentDialogHandler().onSimpleDialogButtonPressed(tag, n, extra);
  }

  @Nullable
  private ConsentDialogHandler __consentDialogHandler = null;

  @NonNull
  private ConsentDialogHandler requireConsentDialogHandler() {
    if(__consentDialogHandler == null)
      __consentDialogHandler = new ConsentDialogHandler(getSupportFragmentManager(), CONFIRMATION_DIALOG_TAG);
    return __consentDialogHandler;
  }

  private static class SubscriptionHolder extends RecyclerView.ViewHolder
  {

    Button mButton;

    SubscriptionHolder( Button button )
    {
      super(button);
      mButton = button;
    }

    void bind( final BuyClickListener clickListener, final DictionaryPrice dictionaryPrice )
    {
      CharSequence text = formatSubscriptionButtonText(dictionaryPrice, mButton.getContext());
      mButton.setText(text);
      mButton.setOnClickListener(new View.OnClickListener()
      {
        PreventDoubleClick preventDoubleClick = new PreventDoubleClick();
        @Override
        public void onClick( View view )
        {
          if ( preventDoubleClick.isDoubleClick() )
          {
            return;
          }
          clickListener.onBuyClick(dictionaryPrice.getPeriod());
        }
      });
    }

    @SuppressLint( "ResourceType" )
    @NonNull
    private CharSequence formatSubscriptionButtonText( DictionaryPrice dictionaryPrice, Context context )
    {
      String priceAndCurrency =
          Utils.convertPriceToString(dictionaryPrice.getPrice()) + dictionaryPrice.getCurrency().getSymbol();
      DictionaryPrice.PeriodSubscription period = dictionaryPrice.getPeriod();
      if ( null == period )
      {
        return priceAndCurrency;
      }

      int stringResId;
      int quantity = period.quantity;
      TypePeriodSubscription periodType = period.type;
      if ( periodType.equals(TypePeriodSubscription.YEAR) )
      {
        stringResId = R.plurals.dictionary_manager_ui_subscribe_button_year_with_price;
      }
      else if ( periodType.equals(TypePeriodSubscription.MONTH) )
      {
        stringResId = R.plurals.dictionary_manager_ui_subscribe_button_month_with_price;
      }
      else if ( periodType.equals(TypePeriodSubscription.WEEK) )
      {
        stringResId = R.plurals.dictionary_manager_ui_subscribe_button_week_with_price;
      }
      else
      {
        return priceAndCurrency;
      }
      return context.getResources().getQuantityString(stringResId, quantity, quantity, priceAndCurrency);
    }

  }

  private static class SubscriptionAdapter extends RecyclerView.Adapter< SubscriptionHolder >
  {

    private final List< DictionaryPrice > subscriptions;
    private final BuyClickListener clickListener;

    public SubscriptionAdapter( @NonNull List< DictionaryPrice > subscriptions, @NonNull BuyClickListener listener)
    {
      this.subscriptions = subscriptions;
      this.clickListener = listener;
    }

    @NonNull
    @Override
    public SubscriptionHolder onCreateViewHolder( @NonNull ViewGroup parent, int viewType )
    {
      return new SubscriptionHolder(
          (Button) LayoutInflater.from(parent.getContext()).inflate(R.layout.subscribe_button, parent, false));
    }

    @Override
    public void onBindViewHolder( @NonNull SubscriptionHolder holder, int position )
    {
      holder.bind(clickListener, subscriptions.get(position));
    }

    @Override
    public int getItemCount()
    {
      return subscriptions.size();
    }
  }

  public static class PreventDoubleClick
  {
    private static final long DEFAULT_THRESHOLD = 500;
    private long threshold;
    private long lastClickMillis = 0;

    public PreventDoubleClick()
    {
      this(DEFAULT_THRESHOLD);
    }

    public PreventDoubleClick( long threshold )
    {
      this.threshold = threshold;
    }

    public boolean isDoubleClick()
    {
      if ( System.currentTimeMillis() - lastClickMillis < threshold )
      {
        return true;
      }
      lastClickMillis = System.currentTimeMillis();
      return false;
    }
  }
}
