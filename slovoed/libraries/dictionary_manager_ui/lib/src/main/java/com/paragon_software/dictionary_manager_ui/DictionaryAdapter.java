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

import androidx.annotation.IdRes;
import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.utils_slovoed.text.LocalizedString;
import com.paragon_software.utils_slovoed_ui.StringsUtils;

public class DictionaryAdapter extends RecyclerView.Adapter< DictionaryAdapter.DictionaryViewHolder >
{
  @NonNull
  private static final int[] BUTTON_IDS = { R.id.go_to_my_dictionaries_button, R.id.dictionary_open_button, R.id.buy_dictionary_button, R.id.preview_dictionary_button, R.id.sample };

  @NonNull
  private final OnClickListener mOnClickListener;
  @LayoutRes
  private final int             mItemLayoutId;
  @NonNull
  private       Dictionary[]    mDictionaries;
  private       DataProvider    mDataProvider;

  DictionaryAdapter( @NonNull DataProvider controller, @NonNull OnClickListener _onClickListener, @LayoutRes int _itemLayoutId )
  {
    mDataProvider = controller;
    mDictionaries = mDataProvider.getDictionaries();
    mOnClickListener = _onClickListener;
    mItemLayoutId = _itemLayoutId;
  }

  void update()
  {
    mDictionaries = mDataProvider.getDictionaries();
    notifyDataSetChanged();
  }

  @NonNull
  @Override
  public DictionaryViewHolder onCreateViewHolder( @NonNull ViewGroup viewGroup, int i )
  {
    View v = LayoutInflater.from(viewGroup.getContext()).inflate(mItemLayoutId, viewGroup, false);
    return new DictionaryViewHolder(v);
  }

  @Override
  public void onBindViewHolder( @NonNull DictionaryViewHolder dictionaryViewHolder, int i )
  {
    dictionaryViewHolder.bindWithDictionary(mDictionaries[i]);
  }

  @Override
  public int getItemCount()
  {
    return mDictionaries.length;
  }

  public interface OnClickListener
  {
    void onClick( @NonNull Dictionary.DictionaryId dictionaryId, @IdRes int buttonId );
  }

  class DictionaryViewHolder extends RecyclerView.ViewHolder
  {
    private final TextView            dictionaryTitle;
    private final TextView            dictionaryStatus;
    private final TextView            dictionaryDealLabel;
    private final ImageView           dictionaryIcon;
    private final SparseArray< View > buttons;

    DictionaryViewHolder( @NonNull final View itemView )
    {
      super(itemView);
      dictionaryTitle = itemView.findViewById(R.id.dictionary_description_label_text_view);
      dictionaryStatus = itemView.findViewById(R.id.dictionary_status_label_text);
      dictionaryIcon = itemView.findViewById(R.id.dictionary_icon_image_view);
      dictionaryDealLabel = itemView.findViewById(R.id.dictionary_deal_label);
      buttons = new SparseArray<>(BUTTON_IDS.length);
      for ( int id : BUTTON_IDS )
      {
        View button = itemView.findViewById(id);
        if ( ( button == null ) && ( id == R.id.buy_dictionary_button ) )
        {
          button = itemView;
        }
        if ( button != null )
        {
          buttons.put(id, button);
        }
      }
    }

    void bindWithDictionary( @NonNull final Dictionary dictionary )
    {
      final Dictionary.DictionaryId dictionaryId = dictionary.getId();
      IDictionaryIcon icon = null;
      LocalizedString title = dictionary.getTitle();
      icon = dictionary.getIcon();
      setTitle(title);
      setStatus(dictionary.getStatus(), dictionaryId);
      setDealLabel(dictionaryId);
      Utils.setDictionaryIcon(dictionaryIcon, icon);
      for ( int i = 0 ; i < buttons.size() ; i++ )
      {
        final int id = buttons.keyAt(i);
        View buttonView = buttons.get(id);
        buttonView.setOnClickListener(new View.OnClickListener()
        {
          @Override
          public void onClick( View v )
          {
            mOnClickListener.onClick(dictionaryId, id);
          }
        });
        if ( buttonView instanceof Button )
        {
          if ( R.id.buy_dictionary_button == buttonView.getId() )
          {
            Dictionary.DICTIONARY_STATUS status = dictionary.getStatus();
            ( (Button) buttonView ).setText(R.string.utils_slovoed_ui_buy_or_subscribe);
          }
          else if ( R.id.preview_dictionary_button == buttonView.getId() )
          {
            ( (Button) buttonView ).setText(StringsUtils.createTrialLengthString(
                buttonView.getContext(), mDataProvider.getTrialLengthInMinutes(dictionary.getId())));
          }
        }
      }
      setButtonsVisibility(dictionary);
    }

    private void setButtonsVisibility( Dictionary dictionary )
    {
      boolean canFullBaseBeUsed = false;
      boolean isTrialAvailable = mDataProvider.isTrialAvailable(dictionary.getId());
      Utils.setVisibility(buttons, R.id.preview_dictionary_button,
                          (canFullBaseBeUsed || (!isTrialAvailable)) ? View.GONE : View.VISIBLE);
      Utils.setVisibility(buttons, R.id.sample,
                          canFullBaseBeUsed ? View.GONE : View.VISIBLE);
      int goToMyDictionariesVisibility;
      int openDictionaryVisibility;
      if (!canFullBaseBeUsed)
      {
        goToMyDictionariesVisibility = View.GONE;
        openDictionaryVisibility = View.GONE;
      }
      else
      {
        boolean downloaded = mDataProvider.isDownloaded(dictionary);
        goToMyDictionariesVisibility = downloaded ? View.GONE : View.VISIBLE;
        openDictionaryVisibility = !downloaded ? View.GONE : View.VISIBLE;
      }
      Utils.setVisibility(buttons, R.id.go_to_my_dictionaries_button, goToMyDictionariesVisibility);
      Utils.setVisibility(buttons, R.id.dictionary_open_button, openDictionaryVisibility);
    }

    private void setDealLabel( Dictionary.DictionaryId dictionaryId )
    {
      if (dictionaryDealLabel != null)
      {
        int percentage = mDataProvider.getDiscountPercentage(dictionaryId);
        if (percentage > 0)
        {
          String dealText = dictionaryDealLabel.getContext().getString(R.string.dictionary_manager_ui_deal_text, percentage);
          dictionaryDealLabel.setVisibility(View.VISIBLE);
          dictionaryDealLabel.setText(dealText);
        }
        else
        {
          dictionaryDealLabel.setVisibility(View.GONE);
        }
      }
    }

    private void setTitle( LocalizedString title )
    {
      if ( dictionaryTitle != null )
      {
        dictionaryTitle.setText(title.get());
      }
    }

    private void setStatus( @NonNull Dictionary.DICTIONARY_STATUS status, @NonNull Dictionary.DictionaryId dictionaryId )
    {
      if ( dictionaryStatus != null )
      {
          dictionaryStatus.setVisibility(View.GONE);
      }
    }
  }

  public interface DataProvider {
    Dictionary[] getDictionaries();
    int getDiscountPercentage( Dictionary.DictionaryId dictionaryId );
    boolean isDownloaded( @NonNull Dictionary dictionary );
    boolean isTrialAvailable(Dictionary.DictionaryId dictionaryId);
    int getTrialLengthInMinutes( @NonNull Dictionary.DictionaryId dictionaryId );
  }
}
