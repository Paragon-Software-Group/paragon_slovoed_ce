package com.paragon_software.testmode_ui;

import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import androidx.annotation.Nullable;
import android.widget.Toast;

import com.paragon_software.testmode.TestModeAPI;
import com.paragon_software.testmode.TestModeHolderUI;
import com.paragon_software.testmode_ui.operations.ConsumeHelper;


public class TestModeFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener
{
  private static final String FAVORITES_ENTRIES_PREF_KEY = "test_mode_favorites_entries";
  private static final String HISTORY_ENTRIES_PREF_KEY = "test_mode_history_entries";
  private static final String TEST_NEWS_PREF_KEY = "test_mode_news";

  private TestModeAPI testModeAPI;
  private ConsumeHelper consumeHelper;

  private EditTextPreference favoritesEntriesPreference;
  private EditTextPreference historyEntriesPreference;
  private CheckBoxPreference testNewsPreference;

  @Override
  public void onCreate( @Nullable Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    onCreateTestModeApi();
    addPreferencesFromResource(R.xml.test_mode_preferences);

    initGeneralPreferences();
    try
    {
      ListPreference consumePreference = (ListPreference) findPreference("test_mode_purchases_consume");
      consumeHelper = new ConsumeHelper(testModeAPI, consumePreference);
      consumeHelper.registerDictionaryManagerObserver(consumePreference.getContext());
    }
    catch ( Exception e )
    {
      e.printStackTrace();
      Toast.makeText(getActivity(), e.toString(), Toast.LENGTH_LONG).show();
    }
  }

  private void initGeneralPreferences()
  {
    favoritesEntriesPreference = (EditTextPreference) findPreference(FAVORITES_ENTRIES_PREF_KEY);
    historyEntriesPreference = (EditTextPreference) findPreference(HISTORY_ENTRIES_PREF_KEY);
    testNewsPreference = (CheckBoxPreference) findPreference(TEST_NEWS_PREF_KEY);
    testNewsPreference.setChecked(testModeAPI.getNewsTestModeState());
    favoritesEntriesPreference.setOnPreferenceChangeListener(this);
    historyEntriesPreference.setOnPreferenceChangeListener(this);
    testNewsPreference.setOnPreferenceChangeListener(this);
  }

  @Override
  public void onDestroyView()
  {
    super.onDestroyView();
    if (null != consumeHelper)
    {
      consumeHelper.unregisterDictionaryManagerObserver();
    }

  }

  private void onCreateTestModeApi()
  {
    try
    {
      testModeAPI = TestModeHolderUI.get();
    }
    catch ( Exception e )
    {
      e.printStackTrace();
      finish();
    }
  }

  private void finish()
  {
    if ( null != getActivity() )
    {
      getActivity().finish();
    }
  }

  @Override
  public boolean onPreferenceChange( Preference preference, Object newValue )
  {
    if ( preference.getKey().equals(FAVORITES_ENTRIES_PREF_KEY)
        || preference.getKey().equals(HISTORY_ENTRIES_PREF_KEY) )
    {
      handleFavoritesAndHistoryPrefs(preference, (String) newValue);
    }
    if ( preference.getKey().equals(TEST_NEWS_PREF_KEY) )
    {
      handleTestNewsPrefs(preference, (boolean) newValue);
    }
    return true;
  }

  private void handleTestNewsPrefs(Preference preference, boolean newValue) {
    if ( preference.getKey().equals(TEST_NEWS_PREF_KEY) )
    {
      testModeAPI.changeNewsTestModeState(newValue);
    }
  }

  private void handleFavoritesAndHistoryPrefs( Preference preference, String newValue )
  {
    final StringBuilder managerName = new StringBuilder();
    TestModeAPI.OnAddItemsInManager callback = new TestModeAPI.OnAddItemsInManager()
    {
      @Override
      public void onItemsAddedBySearch( int found, int added, int size )
      {
        Toast.makeText(getActivity(), managerName + " manager:\n" + found + " - FTS result\n" + added
            + " - added\n" + size + " - final size", Toast.LENGTH_SHORT).show();
      }
    };
    if ( preference.getKey().equals(FAVORITES_ENTRIES_PREF_KEY) )
    {
      managerName.append("Favorites");
      testModeAPI.addItemsInFavoritesBySearch(newValue, callback);
    }
    else if ( preference.getKey().equals(HISTORY_ENTRIES_PREF_KEY) )
    {
      managerName.append("History");
      testModeAPI.addItemsInHistoryBySearch(newValue, callback);
    }

  }
}
