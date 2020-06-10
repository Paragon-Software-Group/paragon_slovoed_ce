package com.paragon_software.dictionary_manager_ui.my_dictionaries;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.appcompat.widget.AppCompatButton;
import androidx.recyclerview.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.components.PictureComponent;
import com.paragon_software.dictionary_manager.components.SoundComponent;
import com.paragon_software.dictionary_manager.components.WordBaseComponent;
import com.paragon_software.dictionary_manager_ui.R;
import com.paragon_software.dictionary_manager_ui.Utils;
import com.paragon_software.dictionary_manager_ui.view.ProgressBarAppCompatButton;

import java.util.ArrayList;
import java.util.List;

public class MyDictionaryComponentsAdapter
    extends RecyclerView.Adapter< MyDictionaryComponentsAdapter.ComponentViewHolder >
{
  @NonNull
  private final IMyDictionariesAdapterProvider mProvider;
  @NonNull
  private       List< DictionaryComponent >    mComponents = new ArrayList<>();

  private Dictionary mDictionary;

  MyDictionaryComponentsAdapter( @NonNull IMyDictionariesAdapterProvider provider )
  {
    mProvider = provider;
  }

  @NonNull
  @Override
  public ComponentViewHolder onCreateViewHolder( @NonNull ViewGroup viewGroup, int viewType )
  {
    View view = LayoutInflater.from(viewGroup.getContext())
                              .inflate(R.layout.my_dictionary_component_item_view, viewGroup, false);
    return new ComponentViewHolder(view);
  }

  @Override
  public void onBindViewHolder( @NonNull ComponentViewHolder holder, int position )
  {
    DictionaryComponent component = mComponents.get(position);
    if ( component != null )
    {
      holder.bind(mComponents.get(position));
    }
  }

  @Override
  public int getItemCount()
  {
    return mComponents.size();
  }

  public void setData( List< DictionaryComponent > components, Dictionary dictionary )
  {
    mComponents = components;
    mDictionary = dictionary;
    notifyDataSetChanged();
  }

  class ComponentViewHolder extends RecyclerView.ViewHolder
  {
    private DictionaryComponent mComponent;

    /**
     * Stub button view to which all other component buttons are anchored. This stub button is used to allocate
     * maximum fixed space for other component buttons (to prevent buttons resizing at application runtime).
     */
    private final AppCompatButton componentsSpaceAnchorButton;

    private final ProgressBar                                             baseDownloadingProgressBar;
    private final ProgressBarAppCompatButton                              pauseBaseDownloadingButton;
    private final AppCompatButton                                         downloadBaseButton;
    private final AppCompatButton                                         removeBaseButton;
    private final DictionaryManagerAPI.IComponentDownloadProgressObserver progressObserver =
        new DictionaryManagerAPI.IComponentDownloadProgressObserver()
        {
          @Override
          public void onProgressChanged()
          {
            bindProgress();
          }

          @Override
          public void onStatusChanged()
          {
            bindButtons();
          }
        };

    ComponentViewHolder( @NonNull final View itemView )
    {
      super(itemView);
      componentsSpaceAnchorButton = itemView.findViewById(R.id.component_space_anchor_button);
      downloadBaseButton = itemView.findViewById(R.id.download_base_button);
      removeBaseButton = itemView.findViewById(R.id.remove_base_button);
      pauseBaseDownloadingButton = itemView.findViewById(R.id.pause_base_downloading_button);
      baseDownloadingProgressBar = itemView.findViewById(R.id.base_downloading_progress_bar);
    }

    private void bindProgress()
    {
      if ( mComponent != null )
      {
        boolean inProgress = mProvider.isInProgress(mComponent);
        int downloadProgress = mProvider.getDownloadProgress(mComponent);
        bindPauseButton(downloadProgress, inProgress);
        if ( inProgress )
        {
          pauseBaseDownloadingButton.setProgress(downloadProgress);
        }
      }
    }

    private void bindButtons()
    {
      if ( mComponent != null )
      {
        String baseSizeMbStr = Utils.formatSize(mComponent.getSize());
        boolean downloaded = mProvider.isDownloaded(mComponent);
        boolean inProgress = mProvider.isInProgress(mComponent);
        bindDownloadButton(baseSizeMbStr, downloaded, inProgress);
        bindPauseButton(0, inProgress);
        bindRemoveButton(baseSizeMbStr, downloaded);
      }
    }

    private void bindDownloadButton( String baseSizeMbStr, boolean downloaded, boolean inProgress )
    {
      Context context = downloadBaseButton.getContext();
      String label = context.getString(R.string.utils_slovoed_ui_common_download);
      if ( mComponent instanceof WordBaseComponent )
      {
        label = context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_word_base_button, baseSizeMbStr);
      }
      else if ( mComponent instanceof SoundComponent )
      {
        label = context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_sound_base_button,
                                  ( (SoundComponent) mComponent ).getName(), baseSizeMbStr);
      }
      else if ( mComponent instanceof PictureComponent )
      {
        label = context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_picture_base_button, baseSizeMbStr);
      }
      downloadBaseButton.setText(label);
      setEnabled(downloadBaseButton, !( downloaded || inProgress ));
      downloadBaseButton.setVisibility(!( downloaded || inProgress ) ? View.VISIBLE : View.GONE);
    }

    private void bindPauseButton( int downloadProgress, boolean inProgress )
    {
      String label = pauseBaseDownloadingButton.getContext().getString(R.string.dictionary_manager_ui_my_dictionaries_download_progress_text,
                                                                       downloadProgress);
      pauseBaseDownloadingButton.setText(label);
      setEnabled(pauseBaseDownloadingButton, inProgress);
      pauseBaseDownloadingButton.setVisibility(inProgress ? View.VISIBLE : View.GONE);
      baseDownloadingProgressBar.setVisibility(inProgress ? View.VISIBLE : View.GONE);
    }

    private void bindRemoveButton( String baseSizeMbStr, boolean downloaded )
    {
      Context context = removeBaseButton.getContext();
      String label = context.getString(R.string.dictionary_manager_ui_my_dictionaries_remove);
      if ( mComponent instanceof WordBaseComponent )
      {
        label = context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_word_base_button, baseSizeMbStr);
      }
      else if ( mComponent instanceof SoundComponent )
      {
        label = context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_sound_base_button,
                                  ( (SoundComponent) mComponent ).getName(), baseSizeMbStr);
      }
      else if ( mComponent instanceof PictureComponent )
      {
        label = context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_picture_base_button, baseSizeMbStr);
      }
      removeBaseButton.setText(label);
      removeBaseButton.setVisibility(downloaded ? View.VISIBLE : View.GONE);
    }

    private void setEnabled( View button, boolean enabled )
    {
      if ( enabled != button.isEnabled() )
      {
        button.setEnabled(enabled);
      }
    }

    void bind( @NonNull final DictionaryComponent component )
    {
      mComponent = component;
      initSpaceAnchorButtonText();
      pauseBaseDownloadingButton.setOnClickListener(new View.OnClickListener()
      {
        @Override
        public void onClick( View view )
        {
          mProvider.pause(mComponent, mDictionary);
        }
      });

      downloadBaseButton.setOnClickListener(new View.OnClickListener()
      {
        @Override
        public void onClick( View view )
        {
          mProvider.download(mComponent, mDictionary);
        }
      });
      removeBaseButton.setOnClickListener(new View.OnClickListener()
      {
        @Override
        public void onClick( View view )
        {
          mProvider.remove(mComponent);
        }
      });

      mProvider.unregisterComponentProgressObserver(progressObserver);
      mProvider.registerComponentProgressObserver(mComponent, progressObserver);
      bindButtons();
      bindProgress();
    }

    /**
     * Initialize space anchor button text by a label with maximum possible length so what
     * the button would occupy its maximum possible space.
     */
    private void initSpaceAnchorButtonText()
    {
      final Context context = componentsSpaceAnchorButton.getContext();
      List< String > labels = new ArrayList< String >()
      {{
        add(context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_progress_text, 100));
      }};
      String baseSizeMbStr = Utils.formatSize(mComponent.getSize());
      if ( mComponent instanceof WordBaseComponent )
      {
        labels.add(context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_word_base_button, baseSizeMbStr));
      }
      else if ( mComponent instanceof SoundComponent )
      {
        String langIso = ( (SoundComponent) mComponent ).getName();
        labels.add(context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_sound_base_button, langIso, baseSizeMbStr));
      }
      else if ( mComponent instanceof PictureComponent )
      {
        labels.add(context.getString(R.string.dictionary_manager_ui_my_dictionaries_download_and_remove_picture_base_button, baseSizeMbStr));
      }
      for ( String label : labels )
      {
        if ( label.length() > componentsSpaceAnchorButton.getText().length() )
        {
          componentsSpaceAnchorButton.setText(label + "    ");
        }
      }
    }
  }
}
