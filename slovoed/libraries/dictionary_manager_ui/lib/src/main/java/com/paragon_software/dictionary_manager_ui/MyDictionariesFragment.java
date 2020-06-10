package com.paragon_software.dictionary_manager_ui;

import android.content.Context;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager_ui.my_dictionaries.IMyDictionariesAdapterProvider;
import com.paragon_software.dictionary_manager_ui.my_dictionaries.MyDictionariesAdapter;

import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.List;

public class MyDictionariesFragment extends DictionaryCardsFragment
    implements DictionaryManagerAPI.IOnDownloadFailureObserver
{
  static final String MY_DICTIONARIES_CONTROLLER_TAG = "MY_DICTIONARIES";

  @NonNull
  public static DictionaryCardsFragment newInstanceMyDictionaries()
  {
    MyDictionariesFragment res = new MyDictionariesFragment();
    Bundle bundle = new Bundle(1);
    bundle.putSerializable(TABS_ENUM, Tabs.MY_DICTIONARIES);
    res.setArguments(bundle);
    return res;
  }

  @Override
  public void onDictionaryListChanged()
  {
    if ( null != mAdapter )
    {
      try
      {
        MyDictionariesAdapter.class.cast(mAdapter).update();
      }
      catch ( Exception ignore )
      {
      }
    }
  }

  @Override
  public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    View res = super.onCreateView(inflater, container, savedInstanceState);
    if ( getArguments() != null && mController != null && mTab != null)
      mController.registerDownloadFailureObserver(this);
    return res;
  }

  @Override
  protected void initAdapter(DictionaryManagerAPI dictionaryManagerAPI, View rootView, int itemLayoutId) {
    IMyDictionariesAdapterProvider statusProvider = new MyDictionariesAdapterProvider(mController, getActivity());
    mAdapter = new MyDictionariesAdapter(this, statusProvider, itemLayoutId);
  }

  @Override
  protected DictionaryControllerAPI createController( DictionaryManagerAPI dictionaryManagerAPI )
  {
    return dictionaryManagerAPI.createController(MY_DICTIONARIES_CONTROLLER_TAG);
  }

  @Override
  public void onResume()
  {
    super.onResume();
    if (null != mController)
    {
      Dictionary.DictionaryId dictionaryId = mController.getDictionaryIdSelectedInMyDictionaries();
      if (null != dictionaryId)
      {
        mController.setDictionaryIdSelectedInMyDictionaries(null);
        scrollDictionariesList(dictionaryId);
      }
    }
  }

  private void scrollDictionariesList( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    if ( null != mAdapter && null != mRecyclerView)
    {
      try
      {
        int position = MyDictionariesAdapter.class.cast(mAdapter).getPosition(dictionaryId);
        if (position >= 0)
        {
          mRecyclerView.scrollToPosition(position);
        }
      }
      catch ( Exception ignore )
      {
      }
    }
  }

  @Override
  public void onClick( @NonNull Dictionary.DictionaryId dictionaryId, int buttonId )
  {
    if ( R.id.dictionary_open_button == buttonId )
    {
      DictionaryManagerAPI dictionaryManagerAPI = DictionaryManagerHolder.getManager();
      mRecentlyViewedHandler.addRecentlyOpened(dictionaryId, dictionaryManagerAPI, mTab);
      Context context = getContext();
      if ( context != null )
      {
        dictionaryManagerAPI.openDictionaryForSearch(context, dictionaryId, null, null);
      }
    }
  }

  @Override
  public void onFailed( @NonNull DictionaryManagerAPI.DownloadFailureReason reason )
  {
    switch(reason)
    {
      case CONNECTION_UNAVAILABLE:
        Toast.makeText(getContext(), getString(R.string.utils_slovoed_ui_connection_unavailable), Toast.LENGTH_LONG).show();
        break;
      case CONNECTION_LOST:
        Toast.makeText(getContext(), getString(R.string.utils_slovoed_ui_connection_lost), Toast.LENGTH_LONG).show();
        break;
      case STORAGE_INSUFFICIENT_SPACE:
        Toast.makeText(getContext(), getString(R.string.dictionary_manager_ui_my_dictionaries_download_insufficient_space), Toast.LENGTH_LONG).show();
        break;
      case FILESYSTEM_ERROR:
      case COMMON_ERROR:
        Toast.makeText(getContext(), getString(R.string.dictionary_manager_ui_my_dictionaries_download_common_error), Toast.LENGTH_LONG).show();
        break;
      case FILE_CORRUPTED:
        Toast.makeText(getContext(), getString(R.string.dictionary_manager_ui_my_dictionaries_download_file_corrupted), Toast.LENGTH_LONG).show();
        break;
    }
    if (null != mAdapter)
    {
      mAdapter.notifyDataSetChanged();
    }
  }

  @Override
  public void onDestroyView()
  {
    super.onDestroyView();
    if (null != mController)
    {
      mController.unregisterDownloadFailureObserver(this);
    }
  }

  private static class MyDictionariesAdapterProvider implements IMyDictionariesAdapterProvider
  {
    @NonNull
    private       DictionaryControllerAPI           mController;
    private final WeakReference< FragmentActivity > activity;

    MyDictionariesAdapterProvider( @NonNull DictionaryControllerAPI mController, @Nullable FragmentActivity activity )
    {
      this.mController = mController;
      this.activity = new WeakReference<>(activity);
    }

    @Override
    public boolean isDownloaded( DictionaryComponent component )
    {
      return mController.isDownloaded(component);
    }

    @Override
    public boolean isInProgress( DictionaryComponent component )
    {
      return mController.isInProgress(component);
    }

    @Override
    public int getDownloadProgress( DictionaryComponent component )
    {
      long transferred = mController.getTransferredBytes(component);
      long size = mController.getSizeBytes(component);
      return 0 == size ? 0 : (int) ( Math.floor(( ( (float) transferred ) / size ) * 100) );
    }

    @Override
    public int getTrialLengthInMinutes( @NonNull Dictionary.DictionaryId dictionaryId )
    {
      return mController.getTrialLengthInMinutes(dictionaryId);
    }

    @Override
    public List< Dictionary > getEnabledDictionaries()
    {
      return Arrays.asList(mController.getDictionaries());
    }

    @Override
    public long getTotalSize( @NonNull Dictionary dictionary )
    {
      long totalSize = 0;
      for ( DictionaryComponent component : dictionary.getDictionaryComponents() )
      {
        if ( !isInAssets(component) && isDownloaded(component) )
        {
          totalSize += component.getSize();
        }
      }
      return totalSize;
    }

    @Override
    public void unregisterComponentProgressObserver( @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer )
    {
      mController.unregisterComponentProgressObserver(observer);
    }

    @Override
    public void registerComponentProgressObserver( @NonNull DictionaryComponent component,
                                                   @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer )
    {
      mController.registerComponentProgressObserver(component, observer);
    }

    @Override
    public void download( @NonNull DictionaryComponent component,
                          @NonNull Dictionary dictionary )
    {
      mController.download(component, dictionary);
    }

    @Override
    public void remove( @NonNull DictionaryComponent component )
    {
      FragmentActivity fragmentActivity = activity.get();
      if ( null != fragmentActivity )
      {
        DialogFragmentDeleteDataBase.show(fragmentActivity.getSupportFragmentManager(), component);
      }
    }

    @Override
    public void pause( DictionaryComponent component, Dictionary dictionary )
    {
      mController.pauseDownload(component, dictionary);
    }

    private boolean isInAssets( DictionaryComponent component )
    {
      // TODO: add impl
      return false;
    }
  }
}
