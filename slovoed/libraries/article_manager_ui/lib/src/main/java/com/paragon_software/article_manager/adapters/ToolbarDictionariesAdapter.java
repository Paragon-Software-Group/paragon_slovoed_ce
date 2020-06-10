package com.paragon_software.article_manager.adapters;

import androidx.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.paragon_software.article_manager_ui.R;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.collections.CollectionView;

public class ToolbarDictionariesAdapter extends BaseAdapter implements CollectionView.OnItemRangeChanged
{
  private final CollectionView< Dictionary, Void > collectionView;
  private LayoutInflater inflater;

  public ToolbarDictionariesAdapter( CollectionView< Dictionary, Void > dictionariesCollectionView,
                                     LayoutInflater inflater )
  {
    this.collectionView = dictionariesCollectionView;
    this.collectionView.registerListener(this);
    this.inflater = inflater;
  }

  @Override
  public int getCount()
  {
    return collectionView.getCount();
  }

  @Override
  public Dictionary getItem( int position )
  {
    return collectionView.getItem(position);
  }

  @Override
  public long getItemId( int position )  {
    return getItem(position).getId().hashCode();
  }

  @Override
  public View getView( int position, View convertView, ViewGroup viewGroup )
  {
    Holder holder;
    if (null == convertView)
    {
      View view = inflater.inflate(R.layout.share_toolbar_dictionary_item, viewGroup, false);
      holder = new Holder(view);
      view.setTag(holder);
    } else
    {
      holder = (Holder) convertView.getTag();
    }

    holder.populateViews(getItem(position));
    return holder.view;
  }

  @Override
  public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int startPosition, int itemCount )
  {
    notifyDataSetChanged();
  }

  private static class Holder
  {
    private final ImageView icon;
    private final TextView text;
    final View view;

    Holder( @NonNull View view )
    {
      this.view = view;
      icon = view.findViewById(R.id.icon);
      text = view.findViewById(R.id.text);
    }

    void populateViews( Dictionary dictionary )
    {
      icon.setImageBitmap(dictionary.getIcon().getBitmap());
      text.setText(dictionary.getTitle().get());
    }
  }
}
