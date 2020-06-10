/*
 * slovoed
 *
 *  Created on: 26.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.slovoed;

import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.navigation_manager.NavigationControllerType;
import com.paragon_software.navigation_manager.NavigationUiApi;
import com.paragon_software.news_manager.NewsManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerHolder;
import com.paragon_software.toolbar_manager.BaseParagonToolbarAPI;
import com.paragon_software.utils_slovoed.device.DeviceUtils;
import com.paragon_software.utils_slovoed.static_consts.BundleKeys;
import com.paragon_software.utils_slovoed_ui_common.activities.ParagonFragmentActivity;
import com.paragon_software.word_of_day.WotDItem;
import com.paragon_software.word_of_day.WotDManagerAPI;

import java.io.Serializable;


public class MainActivity extends ParagonFragmentActivity
{
  @NonNull
  private final NavigationUiApi mNavigationUi = DeviceUtils.isTablet() ?
    BuildConfig.navigationUiFactoryTablet.create(NavigationControllerType.DEFAULT_CONTROLLER) :
    BuildConfig.navigationUiFactory.create(NavigationControllerType.DEFAULT_CONTROLLER);

  private @Nullable BaseParagonToolbarAPI mToolbar;

  @Override
  protected void onCreate( Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    View view = mNavigationUi.initNavigationView(this);
    mToolbar = mNavigationUi.initToolbar(this, ApplicationState.getToolbarFactory() == null ? null :
                                               ApplicationState.getToolbarFactory().inflateToolbar(view));
    if (null == savedInstanceState)
      mNavigationUi.selectDefaultFragment(this);

    try
    {
      SettingsManagerHolder.getManager().isRestored();
    }
    catch ( NullPointerException e )
    {
      e.printStackTrace();
    }

    try
    {
      DictionaryManagerHolder.getManager().updateOnlineTrials(this);
    } catch (NullPointerException e)
    {
      e.printStackTrace();
    }
    parseIntent(getIntent());
  }

  @Override
  protected void onResume()
  {
    if (mToolbar != null)
    {
      mToolbar.onResume();
    }
    super.onResume();
  }

  @Override
  protected void onPause()
  {
    if (mToolbar != null)
    {
      mToolbar.onPause();
    }
    super.onPause();
  }

  @Override
  protected void onNewIntent( Intent intent )
  {
    super.onNewIntent(intent);
    setIntent(intent);
    parseIntent(intent);
  }

  private void parseIntent( Intent intent )
  {
    if (null != intent)
    {

      if (intent.hasExtra(NewsManagerAPI.INTENT_KEY_ADS_FROM_NOTIFICATION)) {
        int keyAdsId = intent.getIntExtra(NewsManagerAPI.INTENT_KEY_ADS_ID, -1);
        if(keyAdsId != -1)
        {
          mNavigationUi.openNewsItemScreen(keyAdsId);
          //Clear extra for intent after action
          intent.putExtra(NewsManagerAPI.INTENT_KEY_ADS_ID, -1);
        }
      }

      WotDItem wotDItem = (WotDItem) intent.getSerializableExtra(WotDManagerAPI.WOTD_ITEM_EXTRA);
      if(wotDItem != null)
      {
        mNavigationUi.openWotDItemScreen(wotDItem);
        //Clear extra for intent after action
        intent.putExtra(WotDManagerAPI.WOTD_ITEM_EXTRA, (Serializable) null);
      }
      else
      {
        Dictionary.DictionaryId dictionaryId = intent.getParcelableExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY);
        String fragment = intent.getStringExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY);
        boolean fromNotification =
                intent.getBooleanExtra(DictionaryManagerAPI.UI_PENDING_INTENT_KEY_BASE_FROM_NOTIFICATION, false);

        if (DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_SEARCH.equals(fragment))
        {
          Dictionary.Direction direction = null;
          Serializable s = intent.getSerializableExtra(DictionaryManagerAPI.UI_EXTRA_DIRECTION_KEY);
          if (s instanceof Dictionary.Direction)
            direction = (Dictionary.Direction) s;
          boolean isOpenPreview = intent.getBooleanExtra(BundleKeys.BUNDLE_KEY_IS_OPEN_FREE_PREVIEW, false);
          if (isOpenPreview) {
            mNavigationUi.showDictionaryPreview(this, dictionaryId, direction);
          }
          else if (dictionaryId != null) {
            String searchText = intent.getStringExtra(DictionaryManagerAPI.UI_EXTRA_SEARCH_STRING_KEY);

            mNavigationUi.showDictionarySample(this, dictionaryId, direction, searchText);
          }
        }
        else if (DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_CATALOG.equals(fragment) || fromNotification)
        {
          boolean isRestorePurchaseAction = intent.getBooleanExtra
                                      (BundleKeys.BUNDLE_KEY_IS_RESTORE_PURCHASE, false);
          if (isRestorePurchaseAction)
          {
            mNavigationUi.openCatalogueAndRestorePurchase(this);
          }
          else if (dictionaryId != null)
          {
            mNavigationUi.openMyDictionaries(this, dictionaryId);
          }
        }
        else if ( DictionaryManagerAPI.UI_EXTRA_FRAGMENT_VALUE_DOWNLOAD.equals(fragment))
        {
            mNavigationUi.openDownloadManager(this, dictionaryId);
        }
        //Clear extra fragment after action
        intent.putExtra(DictionaryManagerAPI.UI_EXTRA_FRAGMENT_KEY, "");
        intent.putExtra(DictionaryManagerAPI.UI_PENDING_INTENT_KEY_BASE_FROM_NOTIFICATION, false);
      }
    }
  }

  @Override
  public boolean onCreateOptionsMenu( Menu menu )
  {
    super.onCreateOptionsMenu(menu);
    if (mToolbar != null)
    {
      mToolbar.onCreateOptionsMenu(menu, getMenuInflater());
    }
    mNavigationUi.onCreateOptionsMenu(menu, getMenuInflater());
    return true;
  }

  @Override
  public boolean onPrepareOptionsMenu( Menu menu )
  {
    super.onPrepareOptionsMenu(menu);
    if (mToolbar != null)
    {
      mToolbar.onPrepareOptionsMenu(menu);
    }
    mNavigationUi.onPrepareOptionsMenu(menu);
    return true;
  }

  @Override
  public boolean onOptionsItemSelected( MenuItem item )
  {
    if ( mToolbar != null && mToolbar.onOptionsItemSelected(item) )
    {
      return true;
    }
    if ( mNavigationUi.onOptionsItemSelected(item) )
    {
      return true;
    }
    return super.onOptionsItemSelected(item);
  }

  @Override
  public void onBackPressed() {
    if(!mNavigationUi.onBackPressed(this))
      super.onBackPressed();
  }

  @Override
  public void onActivityResult(int requestCode, int resultCode, Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
    for (Fragment fragment : getSupportFragmentManager().getFragments()) {
      fragment.onActivityResult(requestCode, resultCode, data);
    }
  }
}
