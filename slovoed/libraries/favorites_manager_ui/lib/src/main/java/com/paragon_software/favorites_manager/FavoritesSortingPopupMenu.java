package com.paragon_software.favorites_manager;

import android.content.Context;
import androidx.annotation.IdRes;
import androidx.annotation.NonNull;
import androidx.annotation.StringRes;
import androidx.appcompat.widget.PopupMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import com.paragon_software.favorites_manager_ui.R;

class FavoritesSortingPopupMenu extends PopupMenu
{
  public FavoritesSortingPopupMenu( @NonNull Context context, @NonNull View anchor )
  {
    super(context, anchor);
    initView();
  }

  public FavoritesSortingPopupMenu( @NonNull Context context, @NonNull View anchor, int gravity )
  {
    super(context, anchor, gravity);
    initView();
  }

  public FavoritesSortingPopupMenu( @NonNull Context context, @NonNull View anchor, int gravity, int popupStyleAttr,
                                    int popupStyleRes )
  {
    super(context, anchor, gravity, popupStyleAttr, popupStyleRes);
    initView();
  }

  private void initView()
  {
    for ( FavoritesSorting sorting : FavoritesSorting.values())
    {
      @StringRes int stringRes;
      @IdRes int idRes;
      switch ( sorting )
      {
        case ALPHABETICALLY_ASCENDING:
          stringRes = R.string.favorites_manage_ui_sort_alphabetically_ascending;
          idRes = R.id.favorites_popup_sort_alphabetically_ascending;
          break;
        case ALPHABETICALLY_DESCENDING:
          stringRes = R.string.favorites_manage_ui_sort_alphabetically_descending;
          idRes = R.id.favorites_popup_sort_alphabetically_descending;
          break;
        case BY_DATE_ASCENDING:
          stringRes = R.string.favorites_manage_ui_sort_by_date_ascending;
          idRes = R.id.favorites_popup_sort_by_date_ascending;
          break;
        case BY_DATE_DESCENDING:
          stringRes = R.string.favorites_manage_ui_sort_by_date_descending;
          idRes = R.id.favorites_popup_sort_by_date_descending;
          break;
        default:
          continue;
      }
      MenuItem item = getMenu().add(1, idRes, Menu.NONE, stringRes);
      item.setCheckable(true);
    }
    getMenu().setGroupCheckable(1, true, true);
  }

  public void setCheckedByOrdinal( boolean checked, int ordinal )
  {
    if (ordinal == FavoritesSorting.ALPHABETICALLY_ASCENDING.ordinal())
    {
      getMenu().findItem(R.id.favorites_popup_sort_alphabetically_ascending).setChecked(checked);
    }
    else if (ordinal == FavoritesSorting.ALPHABETICALLY_DESCENDING.ordinal())
    {
      getMenu().findItem(R.id.favorites_popup_sort_alphabetically_descending).setChecked(checked);
    }
    else if (ordinal == FavoritesSorting.BY_DATE_ASCENDING.ordinal())
    {
      getMenu().findItem(R.id.favorites_popup_sort_by_date_ascending).setChecked(checked);
    }
    else if (ordinal == FavoritesSorting.BY_DATE_DESCENDING.ordinal())
    {
      getMenu().findItem(R.id.favorites_popup_sort_by_date_descending).setChecked(checked);
    }
  }

  public static int getOrdinalByMenuItemId( int id )
  {
    if (id == R.id.favorites_popup_sort_alphabetically_ascending)
    {
      return FavoritesSorting.ALPHABETICALLY_ASCENDING.ordinal();
    }
    else if (id == R.id.favorites_popup_sort_alphabetically_descending)
    {
      return FavoritesSorting.ALPHABETICALLY_DESCENDING.ordinal();
    }
    else if (id == R.id.favorites_popup_sort_by_date_ascending)
    {
      return FavoritesSorting.BY_DATE_ASCENDING.ordinal();
    }
    else if (id == R.id.favorites_popup_sort_by_date_descending)
    {
      return FavoritesSorting.BY_DATE_DESCENDING.ordinal();
    }
    return FavoritesSorting.BY_DATE_DESCENDING.ordinal();
  }
}
