package com.paragon_software.dictionary_manager_ui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.filter.external.IRecentlyOpenedFilter;

class RecentlyViewedHandler
{
  public static final String KEY_CURRENT_ACTIVE_TAB = "CURRENT_ACTIVE_TAB";
  private final LocalBroadcastManager broadcastManager;
  private final String                action;
  private List< BroadcastReceiver > receivers = new LinkedList<>();

  public RecentlyViewedHandler( Context applicationContext )
  {
    broadcastManager = LocalBroadcastManager.getInstance(applicationContext);
    action = applicationContext.getPackageName() + ".ON_RECENTLY_VIEWED_DICTIONARY_CHANGED";
  }

  public void registerReceiver( BroadcastReceiver receiver )
  {
    IntentFilter intentFilter = new IntentFilter(action);
    broadcastManager.registerReceiver(receiver, intentFilter);
    receivers.add(receiver);
  }

  public void addRecentlyOpened( @NonNull Dictionary.DictionaryId dictionaryId,
                                 @NonNull DictionaryManagerAPI dictionaryManager )
  {
    addRecentlyOpened(dictionaryId, dictionaryManager, null);
  }

  public void addRecentlyOpened( @NonNull Dictionary.DictionaryId dictionaryId,
                                 @NonNull DictionaryManagerAPI dictionaryManager, @Nullable Tabs tab )
  {
    IRecentlyOpenedFilter recentlyOpened =
        dictionaryManager.getFilterFactory().createByClass(IRecentlyOpenedFilter.class);
    if ( null != recentlyOpened )
    {
      recentlyOpened.addRecentlyOpened(dictionaryId);
      if ( null != broadcastManager )
      {
        Intent intent = new Intent(action);
        if ( null != tab )
        {
          intent.putExtra(KEY_CURRENT_ACTIVE_TAB, tab);
        }
        broadcastManager.sendBroadcast(intent);
      }
    }
  }

  public void unregisterReceiversAll()
  {
    Iterator< BroadcastReceiver > iterator = receivers.iterator();
    while ( iterator.hasNext() )
    {
      broadcastManager.unregisterReceiver(iterator.next());
      iterator.remove();
    }
  }
}
