package com.paragon_software.search_manager;

import androidx.recyclerview.widget.RecyclerView;

import com.paragon_software.settings_manager.ApplicationSettings;


public abstract class BaseSearchResultAdapter<T extends RecyclerView.ViewHolder> extends RecyclerView.Adapter<T>
{

  protected float mEntryListFontSize = ApplicationSettings.getDefaultFontSize();

  public void setEntryListFontSize( float entryListFontSize )
  {
    mEntryListFontSize = entryListFontSize;
    notifyDataSetChanged();
  }
}
