/*
 * dictionary manager ui
 *
 *  Created on: 14.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager_ui;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Parcelable;
import android.text.Editable;
import android.text.TextUtils;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.RecyclerView;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.ProductDescriptionAndPricesScreenEvent;
import com.paragon_software.analytics_manager.events.TrialScreen;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.IDictionaryDiscount;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.filter.IFilterFactory;
import com.paragon_software.dictionary_manager.filter.external.ILanguageFilter;
import com.paragon_software.dictionary_manager.filter.external.ISearchFilter;
import com.paragon_software.dictionary_manager_ui.LanguagePairSpinners.LanguagePair;
import com.paragon_software.dictionary_manager_ui.my_dictionaries.MyDictionariesAdapter;
import com.paragon_software.dictionary_manager_ui.search.TextChangeListenerDelayed;
import com.paragon_software.utils_slovoed_ui_common.fragments.SimpleDialog;

import java.io.Serializable;
import java.util.List;

public class DictionaryCardsFragment extends Fragment
                                     implements DictionaryAdapter.OnClickListener,
                                                DictionaryManagerAPI.IDictionaryListObserver,
                                                TextView.OnEditorActionListener,
                                                SimpleDialog.Target
{
  protected static final String TABS_ENUM = "TABS_ENUM";

  private static final String TAG = DictionaryCardsFragment.class.toString();
  private static final String CONFIRMATION_DIALOG_TAG = TAG + ".ConfirmationDialog";

  protected RecyclerView             mRecyclerView;
  protected View                     mNoDataResultHint;
  protected RecyclerView.Adapter     mAdapter;
  protected DictionaryControllerAPI  mController;
  protected Tabs                     mTab;
  protected List< DictionaryFilter > mDictionaryFilters;
  protected RecentlyViewedHandler    mRecentlyViewedHandler;
  private   ILanguageFilter          mLanguageFilter;
  private   ISearchFilter            mSearchFilter;
  private LanguageFilterViews mLanguageFilterViews = new LanguageFilterViews();
  private LanguagePairSpinners languagePairSpinners;

  @NonNull
  static DictionaryCardsFragment newInstance( Tabs tabs )
  {
    DictionaryCardsFragment res = new DictionaryCardsFragment();
    Bundle bundle = new Bundle(1);
    bundle.putSerializable(TABS_ENUM, tabs);
    res.setArguments(bundle);
    return res;
  }

  @Override
  public View onCreateView( @NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState )
  {
    mRecentlyViewedHandler = new RecentlyViewedHandler(inflater.getContext().getApplicationContext());
    DictionaryManagerAPI dictionaryManagerAPI = DictionaryManagerHolder.getManager();
    mController = createController(dictionaryManagerAPI);
    dictionaryManagerAPI.registerDictionaryListObserver(this);
    Bundle args = getArguments();
    View rootView = null;
    if ( args != null && mController != null )
    {
      if ( null != ( mTab = Tabs.fromSerializable(args.getSerializable(TABS_ENUM)) ) )
      {
        initDictionaryFilter();
        rootView = inflater.inflate(mTab.getContainerLayoutId(), container, false);
        initLanguageSpinners(rootView);
        initSearchAllField(rootView);
        mController.registerObserver(this);
        mRecyclerView = rootView.findViewById(R.id.main_recycler_view);
        mNoDataResultHint = rootView.findViewById(R.id.no_data_result_hint);
        initAdapter(dictionaryManagerAPI, rootView, mTab.getItemLayoutId());
        mRecyclerView.setAdapter(mAdapter);

      }
    }
    return rootView;
  }

  protected DictionaryControllerAPI createController( DictionaryManagerAPI dictionaryManagerAPI )
  {
    return dictionaryManagerAPI.createController(null);
  }

  protected void initAdapter(DictionaryManagerAPI dictionaryManagerAPI, View rootView, int itemLayoutId) {
    mAdapter = new DictionaryAdapter(createDataProvider(dictionaryManagerAPI), this, itemLayoutId);
  }

  private void initSearchAllField( View rootView )
  {
    final EditText editText = rootView.findViewById(R.id.search_field);
    if ( null != editText )
    {
      final ImageButton clearBtn = rootView.findViewById(R.id.clear_search_field);
      clearBtn.setVisibility(editText.getText().toString().isEmpty() ? View.GONE : View.VISIBLE);
      clearBtn.setOnClickListener(view -> {
        if ( editText != null )
        {
          editText.setText("");
        }
      });

      final DictionaryManagerAPI dictionaryManagerAPI = DictionaryManagerHolder.getManager();
      editText.setOnEditorActionListener(this);
      mSearchFilter = dictionaryManagerAPI.getFilterFactory().createByClass(ISearchFilter.class);
      if ( null != mSearchFilter )
      {
        installFilter(mSearchFilter);
        editText.addTextChangedListener(new TextChangeListenerDelayed(new TextChangeListenerDelayed.Listener()
        {
          @Override
          public void afterTextChanged( @NonNull Editable editable )
          {
            if ( TextUtils.isEmpty(editable.toString()) )
            {
              clearBtn.setVisibility(View.GONE);
              mSearchFilter.setSearchString(editable.toString());
              installFilter(mLanguageFilter);
              mLanguageFilterViews.setVisibility(View.VISIBLE);
            }
            else
            {
              clearBtn.setVisibility(View.VISIBLE);
              mSearchFilter.setSearchString(editable.toString());
              uninstallFilter(mLanguageFilter);
              mLanguageFilterViews.setVisibility(View.GONE);
            }
          }
        }));
        String initialSearchString = mSearchFilter.getInitialSearchString();
        if ( initialSearchString.length() > 0 )
        {
          editText.setText(initialSearchString);
        }
        mLanguageFilterViews.setVisibility(initialSearchString.length() > 0 ? View.GONE : View.VISIBLE);
        clearBtn.setVisibility(initialSearchString.length() > 0 ? View.VISIBLE : View.GONE);
      }
    }
  }

  @Override
  public boolean onEditorAction( TextView view, int actionId, KeyEvent event )
  {
    if (actionId == EditorInfo.IME_ACTION_SEARCH)
    {
      hideKeyboard();
      return true;
    }
    return false;
  }

  private void hideKeyboard()
  {
    InputMethodManager imm = (InputMethodManager) getActivity().getSystemService(Activity.INPUT_METHOD_SERVICE);
    imm.hideSoftInputFromWindow(getView().getRootView().getWindowToken(), 0);
  }

  private DictionaryAdapter.DataProvider createDataProvider( final DictionaryManagerAPI dictionaryManagerAPI )
  {
    return new DictionaryAdapter.DataProvider()
    {
      @Override
      public Dictionary[] getDictionaries()
      {
        return mController.getDictionaries();
      }

      @Override
      public int getDiscountPercentage( Dictionary.DictionaryId dictionaryId )
      {
        IDictionaryDiscount dictionaryDiscount = dictionaryManagerAPI.getDictionaryDiscount(dictionaryId);
        return null != dictionaryDiscount ? dictionaryDiscount.getPercentage() : Integer.MIN_VALUE;
      }

      @Override
      public boolean isDownloaded( @NonNull Dictionary dictionary )
      {
        DictionaryComponent component = MyDictionariesAdapter.getWordBaseComponent(dictionary);
        return null != component && mController.isDownloaded(component);
      }

      @Override
      public boolean isTrialAvailable(Dictionary.DictionaryId dictionaryId) {
        return dictionaryManagerAPI.isTrialAvailable(dictionaryId);
      }

      @Override
      public int getTrialLengthInMinutes( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        return dictionaryManagerAPI.getTrialLengthInMinutes(dictionaryId);
      }
    };
  }

  private void initDictionaryFilter()
  {
    IFilterFactory filterFactory = DictionaryManagerHolder.getManager().getFilterFactory();
    mDictionaryFilters = mTab.getDictionaryFilter(filterFactory);
    for ( DictionaryFilter filter : mDictionaryFilters )
    {
      installFilter(filter);
    }
  }

  private void initLanguageSpinners( @NonNull View rootView )
  {
    Spinner spinnerFrom = rootView.findViewById(R.id.language_from_spinner);
    Spinner spinnerTo = rootView.findViewById(R.id.language_to_spinner);

    if (null != spinnerFrom && null != spinnerTo)
    {
      DictionaryManagerAPI dictionaryManagerAPI = DictionaryManagerHolder.getManager();
      mLanguageFilter = dictionaryManagerAPI.getFilterFactory().createByClass(ILanguageFilter.class);
      if ( null != mLanguageFilter )
      {
        installFilter(mLanguageFilter);
        LanguagePairSpinners.OnLanguagePairChangedListener listener =
            new LanguagePairSpinners.OnLanguagePairChangedListener()
            {
              @Override
              public void onLanguagePairChanged( Integer fromLanguage, Integer toLanguage )
              {
                mLanguageFilter.setLanguages(fromLanguage, toLanguage);
              }
            };
        LanguagePair initialLanguagePair =
            new LanguagePair(mLanguageFilter.getInitialLanguageFrom(), mLanguageFilter.getInitialLanguageTo());
        languagePairSpinners = new LanguagePairSpinners(getContext(), spinnerFrom, spinnerTo, listener);
        languagePairSpinners.fill(initialLanguagePair);
        mLanguageFilterViews = new LanguageFilterViews(rootView.findViewById(R.id.filter_language_from_container),
                                                       rootView.findViewById(R.id.filter_language_to_container),
                                                       rootView.findViewById(R.id.filter_languages_delimiter));
      }
    }
  }

  private void installFilter( @NonNull DictionaryFilter filter )
  {
    if ( null != mController )
    {
      mController.installFilter(filter);
    }
  }

  private void uninstallFilter( @NonNull DictionaryFilter filter )
  {
    if ( null != mController )
    {
      mController.uninstallFilter(filter);
    }
  }

  @Override
  public void onDestroyView()
  {
    DictionaryManagerHolder.getManager().unRegisterDictionaryListObserver(this);
    if (null != mController )
    {
      mController.unregisterObserver(this);
      if (null != mLanguageFilter )
      {
        mController.uninstallFilter(mLanguageFilter);
      }
      if (null != mSearchFilter)
      {
        mController.uninstallFilter(mSearchFilter);
      }
      if (null != mDictionaryFilters )
      {
        for ( DictionaryFilter filter : mDictionaryFilters )
        {
          mController.uninstallFilter(filter);
        }
      }
    }
    if ( mRecyclerView != null )
    {
      mRecyclerView.setAdapter(mAdapter = null);
      mRecyclerView = null;
    }
    super.onDestroyView();
  }

  @Override
  public void onClick( @NonNull Dictionary.DictionaryId dictionaryId, int buttonId )
  {
    Context context = getContext();
    if ( ( buttonId == R.id.buy_dictionary_button ) && ( context != null ) )
    {
      AnalyticsManagerAPI.get().logEvent(new ProductDescriptionAndPricesScreenEvent(mTab.getAnalyticsScreenName()));
      Intent intent = new Intent(context, BuyActivity.class);
      intent.putExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY, (Parcelable) dictionaryId);
      if ( null != mSearchFilter )
      {
        intent.putExtra(DictionaryManagerAPI.UI_EXTRA_ARTICLE_ITEM_KEY,
                        (Serializable) mSearchFilter.getArticleItem(dictionaryId));
      }
      context.startActivity(intent);
    }
    else if ( (buttonId == R.id.preview_dictionary_button) && (context != null) )
    {
      AnalyticsManagerAPI.get().logEvent(new TrialScreen(mTab.getAnalyticsScreenName()));
      requireConsentDialogHandler().showDialog(this, dictionaryId);
    }
    else if ( buttonId == R.id.sample || buttonId == R.id.dictionary_open_button)
    {
      mRecentlyViewedHandler.addRecentlyOpened(dictionaryId, DictionaryManagerHolder.getManager(), mTab);
      if ( getContext() != null )
      {
        DictionaryManagerHolder.getManager().openDictionaryForSearch(getContext(), dictionaryId, null, null);
      }
    } else if ( R.id.go_to_my_dictionaries_button == buttonId && context != null )
    {
      DictionaryManagerHolder.getManager().openMyDictionariesUI(context, dictionaryId);
    }
  }
  @Override
  public void onDictionaryListChanged()
  {
    if ( mAdapter != null )
    {
      try
      {
        DictionaryAdapter.class.cast(mAdapter).update();
        checkNoDataResult();
      }
      catch ( Exception ignore )
      {
      }
    }

    if(languagePairSpinners !=null) {
        LanguagePair initialLanguagePair =
                new LanguagePair(mLanguageFilter.getInitialLanguageFrom(), mLanguageFilter.getInitialLanguageTo());

        languagePairSpinners.fill(initialLanguagePair);
    }
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
      __consentDialogHandler = new ConsentDialogHandler(requireFragmentManager(), CONFIRMATION_DIALOG_TAG);
    return __consentDialogHandler;
  }

  private void checkNoDataResult() {
    if(mAdapter.getItemCount()==0)
    {
      mNoDataResultHint.setVisibility(View.VISIBLE);
      mRecyclerView.setVisibility(View.GONE);
    }
    else
    {
      mNoDataResultHint.setVisibility(View.GONE);
      mRecyclerView.setVisibility(View.VISIBLE);
    }
  }

  private static class LanguageFilterViews {

    private View[] views;

    LanguageFilterViews()
    {
      this(null, null, null);
    }

    LanguageFilterViews( View spinnerFrom, View spinnerTo, View delimiter )
    {
      views = new View[]{ spinnerFrom, spinnerTo, delimiter };
    }

    public void setVisibility( int visible )
    {
      for ( View view : views )
      {
        if (null != view)
        {
          view.setVisibility(visible);
        }
      }
    }
  }
}
