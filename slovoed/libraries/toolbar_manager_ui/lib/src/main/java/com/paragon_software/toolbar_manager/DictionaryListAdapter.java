package com.paragon_software.toolbar_manager;

import android.content.Context;
import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.paragon_software.toolbar_manager_ui.R;

import java.util.Collection;
import java.util.List;

public class DictionaryListAdapter extends BaseAdapter
{

  private       List< DictionaryView > mDictionaryList;
  private final int                    mDefaultRes;
  private final int                    mDictionaryRes;
  private final Context                mContext;

  public DictionaryListAdapter( @NonNull Context context, @LayoutRes int defaultRes,
                                @LayoutRes int dictionaryRes){
    mDefaultRes = defaultRes;
    mDictionaryRes = dictionaryRes;
    mContext = context;
  }

  @Override
  public int getCount()
  {
    return mDictionaryList == null ? 0 : mDictionaryList.size();
  }

  @Override
  public Object getItem( int position )
  {
    return mDictionaryList.get(position);
  }

  @Override
  public long getItemId( int position )
  {
    return position;
  }

  @Override
  public View getView( int position, View convertView, ViewGroup parent )
  {
    final View view;
    DictionaryView dictionary = mDictionaryList.get(position);

    if (0 == position && null == dictionary.getId())
    {
      view = LayoutInflater.from(mContext).inflate(mDefaultRes, parent, false);
    }
    else
    {
      view = LayoutInflater.from(mContext).inflate(mDictionaryRes, parent, false);
    }

    TextView textView = view.findViewById(R.id.text);
    textView.setText(dictionary.getTitle().get());
    ImageView image = view.findViewById(R.id.icon);
    image.setImageBitmap(dictionary.getIcon().getBitmap());

    return view;
  }

  public void setData( Collection<DictionaryView> dictionaryViews){
    mDictionaryList = (List<DictionaryView>)dictionaryViews;
    notifyDataSetChanged();
  }

  public DictionaryView getDictionaryView (int position){
    return mDictionaryList.get(position);
  }

  public int getDictionaryViewPosition(DictionaryView view){
    for (int i = 0; i < mDictionaryList.size(); ++i)
    {
      if (mDictionaryList.get(i).getId() == view.getId())
        return i;
    }
    return 0;
  }
}
