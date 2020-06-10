package com.paragon_software.favorites_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.favorites_manager_ui.R;
import com.paragon_software.settings_manager.ApplicationSettings;

public class FavoritesRecyclerViewAdapter
    extends RecyclerView.Adapter<FavoritesRecyclerViewAdapter.FavoriteWordViewHolder>
{
  private @NonNull List<ArticleItem> mFavoritesWords       = new ArrayList<>();
  private @NonNull List<ArticleItem> mSelectedWords        = new ArrayList<>();
  private boolean                    mSelectionMode;
  private float                      mEntryListFontSize    = ApplicationSettings.getDefaultFontSize();
  private FavoritesControllerAPI     mFavoritesController;

  private OnItemClickListener mItemClickListener;
  private OnItemLongClickListener mItemLongClickListener;
  private OnItemCheckedChangeListener mItemCheckedChangeListener;

  public void setFavoritesController(@NonNull FavoritesControllerAPI favoritesController)
  {
    mFavoritesController = favoritesController;
  }

  @Override
  public FavoriteWordViewHolder onCreateViewHolder( ViewGroup parent, int viewType ) {
    View convertView = LayoutInflater.from(parent.getContext()).inflate(
        R.layout.favorites_list_item, parent, false);
    return new FavoriteWordViewHolder(convertView);
  }

  public void setOnItemClickListener( @Nullable OnItemClickListener onItemClickListener ) {
    mItemClickListener = onItemClickListener;
  }

  public void setOnItemLongClickListener( @Nullable OnItemLongClickListener onItemLongClickListener ) {
    mItemLongClickListener = onItemLongClickListener;
  }

  public void setOnItemCheckedChangeListener( @Nullable OnItemCheckedChangeListener onItemCheckedChangeListener ) {
    mItemCheckedChangeListener = onItemCheckedChangeListener;
  }

  @Override
  public void onBindViewHolder( @NonNull FavoriteWordViewHolder holder, final int position ) {
    holder.bind(position);
  }

  @Override
  public int getItemCount() {
    return mFavoritesWords.size();
  }

  public void setData( @NonNull List<ArticleItem> favoritesWords, @NonNull List<ArticleItem> selectedWords, boolean selectionMode, float entryListFontSize ) {
    mFavoritesWords = favoritesWords;
    mSelectedWords = selectedWords;
    mSelectionMode = selectionMode;
    mEntryListFontSize = entryListFontSize;
    notifyDataSetChanged();
  }

  public void setSelectedWords(@NonNull List<ArticleItem> selectedWords, boolean notifyDataSetChanged) {
    mSelectedWords = selectedWords;
    if (notifyDataSetChanged)
    {
      notifyDataSetChanged();
    }
  }

  public List<ArticleItem> getWords() {
    return mFavoritesWords;
  }

  public interface OnItemClickListener {
    void onItemClick( int position );
  }

  public interface OnItemLongClickListener {
    boolean onItemLongClick( int position );
  }

  public interface OnItemCheckedChangeListener {
    void onItemCheckedChanged( int position, boolean checked );
  }

  public class FavoriteWordViewHolder extends RecyclerView.ViewHolder {

    TextView wordLabel;

    ImageView directionIcon;

    ImageView dictionaryIcon;

    ImageView soundIcon;

    CheckBox checkBox;

    public FavoriteWordViewHolder( View itemView ) {
      super(itemView);
      wordLabel = itemView.findViewById(R.id.favorite_item_label);
      directionIcon = itemView.findViewById(R.id.favorite_item_direction_icon);
      dictionaryIcon = itemView.findViewById(R.id.dictionary_icon);
      soundIcon = itemView.findViewById(R.id.favorite_item_sound);
      checkBox = itemView.findViewById(R.id.favorite_item_checkbox);
      initItemViewListeners(itemView);
    }

    public void setChecked( boolean checked ) {
      checkBox.setChecked(checked);
    }

    private void initItemViewListeners(View itemView) {
      itemView.setOnClickListener(new View.OnClickListener()
      {
        @Override
        public void onClick( View view )
        {
          if (mSelectionMode) {
            if ( mItemCheckedChangeListener != null )
            {
              mItemCheckedChangeListener.onItemCheckedChanged(getAdapterPosition(), !checkBox.isChecked());
            }
          }
          else
          {
            if ( mItemClickListener != null )
            {
              mItemClickListener.onItemClick(getAdapterPosition());
            }
          }
        }
      });
      itemView.setOnLongClickListener(new View.OnLongClickListener()
      {
        @Override
        public boolean onLongClick( View view )
        {
          if ( mItemLongClickListener != null )
          {
            return mItemLongClickListener.onItemLongClick(getAdapterPosition());
          }
          return false;
        }
      });
      checkBox.setOnClickListener(new View.OnClickListener()
      {
        @Override
        public void onClick( View view )
        {
          if ( mSelectionMode && mItemCheckedChangeListener != null )
          {
            mItemCheckedChangeListener.onItemCheckedChanged(getAdapterPosition(), checkBox.isChecked());
          }
        }
      });
    }

    void bind( int position )
    {
      ArticleItem item = mFavoritesWords.get(position);
      bindWordLabel(item);
      bindDirectionIcon(item);
      bindDictionaryIcon(item);
      bindCheckBox(item);
      bindSoundIcon(position);
    }

    private void bindWordLabel( ArticleItem item )
    {
      wordLabel.setTextSize(TypedValue.COMPLEX_UNIT_SP, mEntryListFontSize);
      wordLabel.setText(item.getShowVariantText());
    }

    private void bindDirectionIcon( ArticleItem item )
    {
      if (item.getDirection() != null && item.getDirection().getIcon() != null
          && item.getDirection().getIcon().getBitmap() != null) {
        directionIcon.setImageBitmap(item.getDirection().getIcon().getBitmap());
      }
      else
      {
        directionIcon.setImageResource(android.R.drawable.presence_invisible);
      }
      directionIcon.setVisibility(mSelectionMode ? View.GONE : View.VISIBLE);
    }

    private void bindDictionaryIcon( ArticleItem item )
    {
      if (null != mFavoritesController)
      {
        IDictionaryIcon icon = mFavoritesController.getDictionaryIcon(item.getDictId());
        if (null != icon)
        {
          dictionaryIcon.setImageBitmap(icon.getBitmap());
        }
      }
    }

    private void bindCheckBox( ArticleItem item )
    {
      checkBox.setVisibility(mSelectionMode ? View.VISIBLE : View.GONE);
      checkBox.setChecked(mSelectedWords.contains(item));
    }

    private void bindSoundIcon( final int position )
    {
      boolean hasSound = false;
      if ( mFavoritesController != null )
      {
        hasSound = mFavoritesController.hasSound(position);
      }
      soundIcon.setVisibility(mSelectionMode || !hasSound ? View.GONE : View.VISIBLE);
      soundIcon.setOnClickListener(new View.OnClickListener()
      {
        @Override
        public void onClick( View view )
        {
          if ( mFavoritesController != null )
          {
            mFavoritesController.playSound(position);
          }
        }
      });
    }
  }
}
