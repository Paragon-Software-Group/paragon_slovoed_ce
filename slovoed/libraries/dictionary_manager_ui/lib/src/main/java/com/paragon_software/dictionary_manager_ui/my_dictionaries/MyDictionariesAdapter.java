package com.paragon_software.dictionary_manager_ui.my_dictionaries;

import android.content.Context;
import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.android.flexbox.FlexWrap;
import com.google.android.flexbox.FlexboxLayoutManager;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager_ui.DictionaryAdapter;
import com.paragon_software.dictionary_manager_ui.R;
import com.paragon_software.dictionary_manager_ui.Utils;
import com.paragon_software.utils_slovoed_ui.StringsUtils;

import java.util.ArrayList;
import java.util.List;

import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.IComponentDownloadProgressObserver;

public class MyDictionariesAdapter extends RecyclerView.Adapter< MyDictionariesAdapter.DictionaryViewHolder >
{
  @NonNull
  private final DictionaryAdapter.OnClickListener mOnClickListener;
  @LayoutRes
  private final int                               mItemLayoutId;
  @NonNull
  private final IMyDictionariesAdapterProvider    provider;
  @NonNull
  private       List< Dictionary >                mDictionaries;

  public MyDictionariesAdapter( @NonNull DictionaryAdapter.OnClickListener mOnClickListener,
                                @NonNull IMyDictionariesAdapterProvider provider, int itemLayoutId )
  {
    mItemLayoutId = itemLayoutId;
    this.mOnClickListener = mOnClickListener;
    this.mDictionaries = provider.getEnabledDictionaries();
    this.provider = provider;
  }

  @NonNull
  @Override
  public DictionaryViewHolder onCreateViewHolder( @NonNull ViewGroup viewGroup, int viewType )
  {
    View view = LayoutInflater.from(viewGroup.getContext()).inflate(mItemLayoutId, viewGroup, false);
    return new DictionaryViewHolder(view);
  }

  @Override
  public void onBindViewHolder( @NonNull DictionaryViewHolder holder, int position )
  {
    Dictionary dictionary = mDictionaries.get(position);
    holder.bind(dictionary);
  }

  @Override
  public int getItemCount()
  {
    return mDictionaries.size();
  }

  public void update()
  {
    mDictionaries = provider.getEnabledDictionaries();
    notifyDataSetChanged();
  }

  public int getPosition( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    for ( int i = 0 ; i < mDictionaries.size() ; i++ )
    {
      if ( dictionaryId.equals(mDictionaries.get(i).getId()) )
      {
        return i;
      }
    }
    return Integer.MIN_VALUE;
  }

  public class DictionaryViewHolder extends RecyclerView.ViewHolder
  {
    private final ImageView                          dictionaryIcon;
    private final TextView                           dictionaryTitle;
    private final TextView                           dictionaryDownloadedBytes;
    private final TextView                           dictionaryStatus;
    private final View                               openButton;
    private       IComponentDownloadProgressObserver progressObserver;
    private final RecyclerView                       dictionaryComponentsView;
    private       MyDictionaryComponentsAdapter      componentsAdapter;
    private       Dictionary                         dictionary;

    DictionaryViewHolder( @NonNull final View itemView )
    {
      super(itemView);
      dictionaryIcon = itemView.findViewById(R.id.dictionary_icon_image_view);
      dictionaryTitle = itemView.findViewById(R.id.dictionary_description_label_text_view);
      dictionaryDownloadedBytes = itemView.findViewById(R.id.dictionary_downloaded_bytes_text_view);
      dictionaryStatus = itemView.findViewById(R.id.dictionary_status_text_view);
      openButton = itemView.findViewById(R.id.dictionary_open_button);
      progressObserver = new IComponentDownloadProgressObserver()
      {
        @Override
        public void onProgressChanged() { }

        @Override
        public void onStatusChanged()
        {
          bindTotalSize();
          bindButtons();
        }
      };
      dictionaryComponentsView = itemView.findViewById(R.id.dictionary_components);
      FlexboxLayoutManager layoutManager = new FlexboxLayoutManager(dictionaryComponentsView.getContext());
      layoutManager.setFlexWrap(FlexWrap.WRAP);
      dictionaryComponentsView.setLayoutManager(layoutManager);
      dictionaryComponentsView.setNestedScrollingEnabled(false);
      componentsAdapter = new MyDictionaryComponentsAdapter(provider);
      dictionaryComponentsView.setAdapter(componentsAdapter);
    }

    private void bindButtons()
    {
      if ( null != dictionary )
      {
        DictionaryComponent wordBaseComponent = getWordBaseComponent(dictionary);
        if ( null != wordBaseComponent )
        {
          boolean downloaded = provider.isDownloaded(wordBaseComponent);
          setEnabled(openButton, downloaded);
        }
      }
    }

    public void bind( @NonNull final Dictionary dictionary )
    {
      this.dictionary = dictionary;
      bindIconAndTitle(dictionary);
      bindStatus(dictionary);

      openButton.setOnClickListener(new View.OnClickListener()
      {
        @Override
        public void onClick( View view )
        {
          mOnClickListener.onClick(dictionary.getId(), openButton.getId());
        }
      });
      final DictionaryComponent wordBaseComponent = getWordBaseComponent(dictionary);
      final List<DictionaryComponent> soundComponents = getDictionaryComponents(dictionary, false, DictionaryComponent.Type.SOUND);
      final List<DictionaryComponent> pictureComponents = getDictionaryComponents(dictionary, false, DictionaryComponent.Type.PICT);
      List<DictionaryComponent> componentsList = new ArrayList< DictionaryComponent >()
      {{
        if (wordBaseComponent != null) add(wordBaseComponent);
        if (pictureComponents != null) addAll(pictureComponents);
        if (soundComponents != null) addAll(soundComponents);
      }};
      provider.unregisterComponentProgressObserver(progressObserver);
      for ( DictionaryComponent component : componentsList )
      {
        provider.registerComponentProgressObserver(component, progressObserver);
      }
      componentsAdapter.setData(componentsList, dictionary);
      bindTotalSize();
      bindButtons();
    }

    private void bindStatus( Dictionary dictionary )
    {
      dictionaryStatus.setVisibility(View.GONE);
    }

    private void bindIconAndTitle( @NonNull Dictionary dictionary )
    {
      Utils.setDictionaryIcon(dictionaryIcon, dictionary.getIcon());
      dictionaryTitle.setText(dictionary.getTitle().get());
    }

    private void bindTotalSize()
    {
      long totalSize = null == dictionary ? 0 : provider.getTotalSize(dictionary);
      Context context = dictionaryTitle.getContext();
      String totalSizeString = context.getString(R.string.dictionary_manager_ui_my_dictionaries_total_size, Utils.formatSize(totalSize));
      dictionaryDownloadedBytes.setText(totalSizeString);
    }

    private void setEnabled( View button, boolean enabled )
    {
      if ( enabled != button.isEnabled() )
      {
        button.setEnabled(enabled);
      }
    }
  }

  public static DictionaryComponent getWordBaseComponent( Dictionary dictionary )
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

  public static List<DictionaryComponent> getDictionaryComponents( @NonNull Dictionary dictionary, boolean demo, @NonNull DictionaryComponent.Type type )
  {
    List<DictionaryComponent> components = new ArrayList<>();
    for ( DictionaryComponent component : dictionary.getDictionaryComponents() )
    {
      if ( component.isDemo() == demo && type.equals(component.getType()))
      {
        components.add(component);
      }
    }
    return components;
  }
}
