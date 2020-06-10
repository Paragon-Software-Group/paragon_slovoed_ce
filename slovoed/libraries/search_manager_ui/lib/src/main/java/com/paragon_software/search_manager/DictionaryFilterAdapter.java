package com.paragon_software.search_manager;

import android.graphics.Bitmap;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.search_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.List;

import static androidx.recyclerview.widget.RecyclerView.NO_POSITION;

public class DictionaryFilterAdapter extends RecyclerView.Adapter<DictionaryFilterAdapter.ViewHolder>
{

  private int mSelectedPos = NO_POSITION;
  private OnDictionaryFilterSelect       mOnDictionaryFilterSelectListener;
  private SearchController mSearchController;
  private CollectionView<Dictionary.DictionaryId, Void> mDictionaryIdList;
  private List<Dictionary>               mDictionaries;


  public interface OnDictionaryFilterSelect
  {
    void onDictionaryFilterSelect( Dictionary.DictionaryId dictionaryId );
  }

  public void registerOnDictionaryFilterSelectListener( OnDictionaryFilterSelect listener )
  {
    mOnDictionaryFilterSelectListener = listener;
  }

  public void setSearchController( SearchController searchController)
  {
    mSearchController = searchController;
  }

  @NonNull
  @Override
  public ViewHolder onCreateViewHolder( @NonNull ViewGroup parent, int viewType )
  {
    return new ViewHolder(LayoutInflater.from(parent.getContext())
                                        .inflate(R.layout.horizontal_dictionary_item,parent,false));
  }

  @Override
  public void onBindViewHolder( @NonNull ViewHolder holder, int position )
  {
    holder.setData(mDictionaryIdList.getItem(position));
    holder.itemView.setSelected(mSelectedPos == position);
    if (position != mSelectedPos)
    {
      holder.mDictionaryIcon.setAlpha(127);
    }
    else
    {
      holder.mDictionaryIcon.setAlpha(255);
    }
  }

  @Override
  public int getItemCount()
  {
    if (null == mDictionaryIdList)
    {
      return 0;
    }
    else
    {
      return mDictionaryIdList.getCount();
    }
  }

  public void setDictionaries(List<Dictionary> dictionaries)
  {
    mDictionaries = dictionaries;
  }

  public void setData( CollectionView<Dictionary.DictionaryId, Void> dictionaryIdList )
  {
    mSelectedPos = NO_POSITION;
    mDictionaryIdList = dictionaryIdList;
    notifyDataSetChanged();
  }

  public void updateData(CollectionView<Dictionary.DictionaryId ,Void> dictionaryIdList)
  {
    mDictionaryIdList = dictionaryIdList;
    notifyDataSetChanged();
  }

  public void setSelectedDictionaryId( Dictionary.DictionaryId dictionaryId)
  {
    if (null == mDictionaryIdList) return;

    for (int i = 0; i < mDictionaryIdList.getCount(); ++i )
    {
      if (mDictionaryIdList.getItem(i).equals(dictionaryId))
      {
        mSelectedPos = i;
      }
    }
  }

  public int getSelectedPosition()
  {
    return mSelectedPos;
  }

  public CollectionView<Dictionary.DictionaryId, Void> getData()
  {
    return mDictionaryIdList;
  }

  class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener
  {
    ImageView mDictionaryIcon;
    Dictionary.DictionaryId mDictionaryId;

    public ViewHolder( View itemView )
    {
      super(itemView);
      mDictionaryIcon = itemView.findViewById(R.id.dictionaryIcon);
      itemView.setOnClickListener(this);
    }

    public void setData(Dictionary.DictionaryId dictionaryId )
    {
      mDictionaryId = dictionaryId;
      mDictionaryIcon.setImageBitmap(getBitmap(dictionaryId));
    }

    @Override
    public void onClick( View v )
    {
      final  int position = getAdapterPosition();
      if (position == NO_POSITION) return;

      final Dictionary.DictionaryId selectedDictionary;
      notifyItemChanged(mSelectedPos);
      if (position == mSelectedPos)
      {
        mSelectedPos = NO_POSITION;
        selectedDictionary = null;
      }
      else
      {
        mSelectedPos = position;
        selectedDictionary = mDictionaryId;
        notifyItemChanged(mSelectedPos);
      }
      if (null != mOnDictionaryFilterSelectListener )
      {
        mOnDictionaryFilterSelectListener.onDictionaryFilterSelect(selectedDictionary);
      }
    }

    private Bitmap getBitmap( Dictionary.DictionaryId dictionaryId)
    {
      for (Dictionary dictionary : mDictionaries)
      {
        if (dictionary.getId().equals(dictionaryId))
        {
          return dictionary.getIcon().getBitmap();
        }
      }
      return null;
    }
  }
}
