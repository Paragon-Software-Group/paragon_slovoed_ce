package com.paragon_software.testmode_ui.operations;

import android.content.Context;
import android.preference.ListPreference;
import android.preference.Preference;
import androidx.annotation.NonNull;
import android.widget.Toast;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.errors.ErrorType;
import com.paragon_software.dictionary_manager.errors.OperationType;
import com.paragon_software.testmode.TestModeAPI;

import java.util.List;

public class ConsumeHelper
{
  private final DictionaryManagerAPI      dictionaryManager;
  private       TestModeAPI               testModeAPI;
  private       DictionaryManagerObserver dictionaryManagerObserver;
  private       ListPreference            consumePreference;

  public ConsumeHelper( @NonNull TestModeAPI testModeAPI, @NonNull ListPreference consumePreference )
  {
    this.testModeAPI = testModeAPI;
    this.dictionaryManager = this.testModeAPI.getDictionaryManager();
    if ( null == dictionaryManager )
    {
      throw new IllegalStateException("Dictionary manager is not initialized for Test Mode");
    }
    initListPreference(this.consumePreference = consumePreference);
  }

  private void initListPreference( ListPreference consumePreference )
  {
    List< Dictionary > dictionariesForConsume = testModeAPI.getDictionariesForConsume();
    consumePreference.setEnabled(!dictionariesForConsume.isEmpty());
    if (!dictionariesForConsume.isEmpty())
    {
      consumePreference.setEntryValues(createEntryValues(dictionariesForConsume));
      consumePreference.setEntries(createEntries(dictionariesForConsume));
      consumePreference.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener()
      {
        @Override
        public boolean onPreferenceChange( Preference preference, Object checkedEntryValue )
        {
          Context context = preference.getContext();
          Dictionary.DictionaryId dictionaryId = new Dictionary.DictionaryId(checkedEntryValue.toString());
          consume(context, dictionaryId);
          return false;
        }
      });
    }
  }

  private CharSequence[] createEntryValues( List< Dictionary > dictionaries )
  {
    CharSequence[] charSequences = new CharSequence[dictionaries.size()];
    for ( int i = 0 ; i < dictionaries.size() ; i++ )
    {
      charSequences[i] = dictionaries.get(i).getId().toString();
    }
    return charSequences;
  }

  private CharSequence[] createEntries( List< Dictionary > dictionaries )
  {
    CharSequence[] charSequences = new CharSequence[dictionaries.size()];
    for ( int i = 0 ; i < dictionaries.size() ; i++ )
    {
      charSequences[i] = dictionaries.get(i).getTitle().get();
    }
    return charSequences;
  }

  private void consume( Context context, Dictionary.DictionaryId dictionaryId )
  {
    Dictionary dictionary = findDictionaryById(dictionaryId);
    if ( null == dictionary )
    {
      Toast.makeText(context, "Not found dictionary with id : " + dictionaryId, Toast.LENGTH_LONG).show();
    }
    else
    {
      ErrorType error = testModeAPI.consumeDictionaryPurchase(context, dictionary);
      if ( ErrorType.PURCHASE_ITEM_UNAVAILABLE.equals(error) )
      {
        Toast.makeText(context, "Purchase item not available for dictionary with id : " + dictionaryId,
                       Toast.LENGTH_LONG).show();
      }
      else if ( ErrorType.PURCHASE_ITEM_NOT_OWNED.equals(error) )
      {
        Toast.makeText(context, "Purchase item not owned for dictionary with id : " + dictionaryId, Toast.LENGTH_LONG)
             .show();
      }
      else
      {
        Toast.makeText(context, "Begin consuming " + dictionaryId + ". Wait…", Toast.LENGTH_SHORT).show();
      }
    }
  }

  private Dictionary findDictionaryById( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    for ( Dictionary dictionary : testModeAPI.getDictionariesForConsume() )
    {
      if ( dictionaryId.equals(dictionary.getId()) )
      {
        return dictionary;
      }
    }
    return null;
  }

  public void registerDictionaryManagerObserver( @NonNull Context context )
  {
    dictionaryManagerObserver = new DictionaryManagerObserver(context);
    dictionaryManager.registerErrorObserver(dictionaryManagerObserver);
    dictionaryManager.registerDictionaryListObserver(dictionaryManagerObserver);
  }

  public void unregisterDictionaryManagerObserver()
  {
    dictionaryManager.unRegisterErrorObserver(dictionaryManagerObserver);
    dictionaryManager.unRegisterDictionaryListObserver(dictionaryManagerObserver);
  }

  class DictionaryManagerObserver
      implements DictionaryManagerAPI.IErrorObserver, DictionaryManagerAPI.IDictionaryListObserver
  {

    private Context context;

    DictionaryManagerObserver( Context context )
    {
      this.context = context;
    }

    @Override
    public void onError( @NonNull OperationType operationType, @NonNull List< ErrorType > error )
    {
      if ( operationType.equals(OperationType.UPDATE_PURCHASES) )
      {
        Toast.makeText(context, "Error while update purchases status. " + error.toString(), Toast.LENGTH_LONG).show();
      }
      else if ( operationType.equals(OperationType.CONSUME) )
      {
        Toast.makeText(context, "Error while consume. " + error.toString(), Toast.LENGTH_LONG).show();
      }
    }

    @Override
    public void onDictionaryListChanged()
    {
      Toast.makeText(context, "Dictionaries status updated", Toast.LENGTH_SHORT).show();
      if ( null != consumePreference )
      {
        initListPreference(consumePreference);
      }
    }
  }
}
