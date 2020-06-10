package com.paragon_software.search_manager;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.search_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.ArrayList;
import java.util.List;

public class SearchResultAdapter extends BaseSearchResultAdapter<SearchResultAdapter.BaseViewHolder>

{
  private OnArticleClickListener    mOnArticleClickListener  = null;
  private Dictionary.DictionaryId   mSearchDictionaryId      = null;
  private Dictionary.DictionaryId   mFilterDictionaryId      = null;
  private SearchController mSearchController;
  private CollectionView<ArticleItem, ?> mResultList;
  private ArrayList<ArticleItem>    mFilteredList            = new ArrayList<>();
  private List<Dictionary>          mDictionaries;

  private static final int SEARCH_RESULT_TYPE_BASE = 0;
  private static final int SEARCH_RESULT_TYPE_ALL_DICTIONARIES = 1;

  public void setSearchController(SearchController searchController)
  {
    mSearchController = searchController;
  }

  public void selectDictionary( Dictionary.DictionaryId dictionaryId)
  {
    mSearchDictionaryId = dictionaryId;
  }

  public void registerOnArticleClickListener(OnArticleClickListener listener){
    mOnArticleClickListener = listener;
  }

  public CollectionView<ArticleItem, ?> getData()
  {
    return mResultList;
  }

  public void setData(CollectionView <ArticleItem, ?> result, float entryListFontSize)
  {
    mEntryListFontSize = entryListFontSize;
    if (null != mResultList && mResultList.equals(result))
    {
      return;
    }

    mResultList = result;
    notifyDataSetChanged();
  }

  public void setDictionaries(List<Dictionary> dictionaries)
  {
    mDictionaries = dictionaries;
  }

  public void setFilterDictionaryId( Dictionary.DictionaryId dictionaryId)
  {
    mFilterDictionaryId = dictionaryId;
    createFilteredList();
    notifyDataSetChanged();
  }

  public void createFilteredList()
  {
    mFilteredList.clear();

    if (null != mFilterDictionaryId)
    {
      for (int i = 0; i < mResultList.getCount(); ++i)
      {
        ArticleItem item = mResultList.getItem(i);
        if (item.getDictId().equals(mFilterDictionaryId))
        {
          mFilteredList.add(item);
        }
      }
    }
  }

  public void notifyRangeInserted( int position, int count )
  {
    if (null == mFilterDictionaryId)
    {
      notifyItemRangeInserted(position,count);
    }
    else
    {
      final int end = position + count;
      ArticleItem item;
      for ( int i = position ; i < end ; ++i )
      {
        item = mResultList.getItem(i);
        if ( item.getDictId() == mFilterDictionaryId )
        {
          mFilteredList.add(item);
        }
      }
    }
  }

  @NonNull
  @Override
  public BaseViewHolder onCreateViewHolder( @NonNull ViewGroup parent, int viewType )
  {
    final BaseViewHolder viewHolder;

    if (SEARCH_RESULT_TYPE_ALL_DICTIONARIES == viewType)
    {
      View view = LayoutInflater.from(parent.getContext())
                                .inflate(R.layout.all_dictionaries_article, parent, false);
      viewHolder =  new AllDictionariesViewHolder(view);
    }
    else
    {
      View view = LayoutInflater.from(parent.getContext())
                                .inflate(R.layout.base_article, parent, false);
      viewHolder =  new BaseViewHolder(view);
    }
    return viewHolder;
  }

  @Override
  public void onBindViewHolder( @NonNull BaseViewHolder holder, int position )
  {
    ArticleItem item;
    if (null == mFilterDictionaryId)
    {
      item = mResultList.getItem(position);
    }
    else
    {
      item = mFilteredList.get(position);
    }

    holder.setData(item, mEntryListFontSize);
  }

  @Override
  public int getItemCount()
  {
    if (null == mFilterDictionaryId)
    {
      if ( null == mResultList )
      {
        return 0;
      }
      return mResultList.getCount();
    }
    else
    {
      return mFilteredList.size();
    }
  }

  @Override
  public int getItemViewType( int position )
  {
    return null == mSearchDictionaryId ? SEARCH_RESULT_TYPE_ALL_DICTIONARIES : SEARCH_RESULT_TYPE_BASE;
  }

  class BaseViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener
  {

    private final TextView  mTextView;
    private ArticleItem     mItem;
    private final ImageButton mSound;

    BaseViewHolder( View v )
    {
      super(v);
      mTextView = v.findViewById(R.id.text);
      v.setOnClickListener(this);
      mSound = v.findViewById(R.id.sound);
      mSound.setOnClickListener(this);
    }

    @Override
    public void onClick( View v )
    {
      if (v == mSound)
      {
        mSearchController.playSound(mItem);
      }
      else if (null != mOnArticleClickListener )
      {
        mOnArticleClickListener.onArticleClick(mItem);
      }
    }

    public void setData(ArticleItem item, float entryListFontSize)
    {
      mTextView.setTextSize(TypedValue.COMPLEX_UNIT_SP, entryListFontSize);
      mTextView.setText(item.getLabel());
      mItem = item;

      if (null != mSearchController && mSearchController.itemHasSound(item))
      {
        mSound.setVisibility(View.VISIBLE);
      }
      else
      {
        mSound.setVisibility(View.INVISIBLE);
      }
    }
  }

  class AllDictionariesViewHolder extends BaseViewHolder
  {
    private final ImageView mDictionaryIcon;
    private final ImageView mDirectionIcon;

    public AllDictionariesViewHolder( View v )
    {
      super(v);
      mDictionaryIcon = v.findViewById(R.id.dictionaryIcon);
      mDirectionIcon = v.findViewById(R.id.directionIcon);
    }

    @Override
    public void setData(ArticleItem item, float entryListFontSize)
    {
      super.setData(item, entryListFontSize);
      setIcons(item);
      if (null == mSearchController) return;
    }

    private void setIcons( ArticleItem item )
    {
      if (null != mSearchDictionaryId) return;
      if (null == mDictionaryIcon) return;
      for (Dictionary dictionary : mDictionaries)
      {
        if (dictionary.getId().equals(item.getDictId()))
        {
          mDictionaryIcon.setImageBitmap(dictionary.getIcon().getBitmap());
          mDirectionIcon.setImageBitmap(item.getDirection().getIcon().getBitmap());
          break;
        }
      }
    }
  }
}
