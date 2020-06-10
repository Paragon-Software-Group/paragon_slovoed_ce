package com.paragon_software.favorites_manager;

import android.app.Dialog;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.appcompat.widget.PopupMenu;
import androidx.recyclerview.widget.RecyclerView;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.favorites_manager_ui.R;
import com.paragon_software.utils_slovoed_ui_common.fragments.SimpleDialog;

public class FavoritesFragment extends Fragment
    implements OnControllerFavoritesListChangedListener, OnControllerSelectionModeChangedListener, OnControllerErrorListener,
               OnControllerEntryListFontSizeChangeListener, OnControllerSortingChangeListener,
               OnControllerTransitionStateChangedListener, OnControllerSortingActionChange,
               OnControllerDeleteAllActionChange, OnControllerShowDeleteSelectedDialogListener,
               SimpleDialog.Target
{
  private static final String SHOW_SORTING_POPUP_STATE_SAVE_TAG = "show_favorites_sorting_popup_menu";

  private static final String DELETE_ALL_DIALOG_TAG = "delete_all_dialog_tag";
  private static final String DELETE_SELECTED_DIALOG_TAG = "delete_selected_dialog_tag";

  private RecyclerView                   mFavoritesRecyclerView;
  private FavoritesRecyclerViewAdapter   mFavoritesRecyclerViewAdapter;

  @Nullable
  private MenuItem mSortingItem, mDeleteAllItem;

  /**
   * Needed to restore favorites sorting popup menu state after configuration changes.
   */
  private boolean mShowFavoritesSortingPopupMenu;

  private FavoritesSortingPopupMenu mFavoritesSortingPopupMenu;

  /**
   * Text view label to show current favorites sorting type.
   */
  private TextView mFavoritesSortingLabel;

  private ProgressBar mFavoritesProgressBar;

  /**
   * Controller to provide communication between favorites manager and the view.
   */
  private FavoritesControllerAPI mController;

  private final PopupMenu.OnMenuItemClickListener mFavoritesSortingPopupMenuItemClickListener = new PopupMenu.OnMenuItemClickListener()
  {
    @Override
    public boolean onMenuItemClick( MenuItem menuItem )
    {
      int sortingOrdinal = FavoritesSortingPopupMenu.getOrdinalByMenuItemId(menuItem.getItemId());
      mController.selectSorting(sortingOrdinal);
      return true;
    }
  };

  private final PopupMenu.OnDismissListener mFavoritesSortingPopupMenuDismissListener = new PopupMenu.OnDismissListener()
  {
    @Override
    public void onDismiss( PopupMenu popupMenu )
    {
      mShowFavoritesSortingPopupMenu = false;
    }
  };

  @Override
  public void onStart()
  {
    setMenuVisibility(true);
    super.onStart();
  }

  @Override
  public void onStop()
  {
    setMenuVisibility(false);
    super.onStop();
  }

  private void initController()
  {
    FavoritesManagerAPI favoritesManager = FavoritesManagerHolder.getManager();
    if ( favoritesManager != null )
    {
      mController = favoritesManager.getController(FavoritesControllerType.DEFAULT_CONTROLLER);
    }
  }

  @Override
  public void onResume()
  {
    super.onResume();
    initController();
    initKeyListener();
    updateView();
    mController.registerNotifier(this);
  }

  @Override
  public void onPause()
  {
    mController.unregisterNotifier(this);
    FavoritesManagerAPI favoritesManager = FavoritesManagerHolder.getManager();
    if ( favoritesManager != null )
    {
      favoritesManager.freeController(FavoritesControllerType.DEFAULT_CONTROLLER);
    }
    super.onPause();
  }

  @Nullable
  @Override
  public View onCreateView( @NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                            @Nullable Bundle savedInstanceState )
  {
    setHasOptionsMenu(true);
    View view = inflater.inflate(R.layout.fragment_favorites, container,false);
    initViews(view);
    return view;
  }

  private void initKeyListener() {
    getView().setFocusableInTouchMode(true);
    getView().requestFocus();
    getView().setOnKeyListener(new View.OnKeyListener()
    {
      @Override
      public boolean onKey( View view, int keyCode, KeyEvent event )
      {
        if( keyCode == KeyEvent.KEYCODE_BACK && mController != null && mController.isInSelectionMode())
        {
          mController.selectionModeOff();
          return true;
        }
        return false;
      }
    });
  }

  private void initViews( View fragmentView ){
    mFavoritesProgressBar = fragmentView.findViewById(R.id.favorites_progress_bar);
    mFavoritesSortingLabel = fragmentView.findViewById(R.id.favorites_sorting_type_label);
    mFavoritesRecyclerView = fragmentView.findViewById(R.id.favorites_list);
    mFavoritesRecyclerView.setLayoutManager(new LinearLayoutManager(getContext()));
    mFavoritesRecyclerViewAdapter = new FavoritesRecyclerViewAdapter();
    initFavoritesRecyclerViewAdapterListeners(mFavoritesRecyclerViewAdapter);
    mFavoritesRecyclerView.setAdapter(mFavoritesRecyclerViewAdapter);
  }

  private void initFavoritesRecyclerViewAdapterListeners(FavoritesRecyclerViewAdapter favoritesRecyclerViewAdapter) {
    favoritesRecyclerViewAdapter.setOnItemClickListener(new FavoritesRecyclerViewAdapter.OnItemClickListener()
    {
      @Override
      public void onItemClick( int position )
      {
        if ( mController != null )
        {
          mController.openArticle(position, getContext());
        }
      }
    });
    favoritesRecyclerViewAdapter.setOnItemLongClickListener(new FavoritesRecyclerViewAdapter.OnItemLongClickListener()
    {
      @Override
      public boolean onItemLongClick( int position )
      {
        if ( mController != null && !mController.isInSelectionMode() )
        {
          mController.selectItem(position);
        }
        return true;
      }
    });
    favoritesRecyclerViewAdapter.setOnItemCheckedChangeListener(new FavoritesRecyclerViewAdapter.OnItemCheckedChangeListener()
    {
      @Override
      public void onItemCheckedChanged( int position, boolean checked )
      {
        if ( mController != null )
        {
          if ( checked )
          {
            mController.selectItem(position);
          }
          else
          {
            mController.unselectItem(position);
          }
        }
      }
    });
  }

  private void updateView() {
    if (getView() != null && mFavoritesRecyclerViewAdapter != null && mController != null)
    {
      mFavoritesRecyclerViewAdapter.setFavoritesController(mController);
      updateFavoritesRecyclerView();
      updateFavoritesProgressBar(mController.isInTransition());
      updateFavoritesSortingLabel(mFavoritesRecyclerViewAdapter.getItemCount() > 0 ? mController.getFavoritesSorting() : null);
      getView().findViewById(R.id.empty_label).setVisibility(
          mFavoritesRecyclerViewAdapter.getItemCount() > 0 ? View.GONE : View.VISIBLE);
      getView().findViewById(R.id.favorites_content_card).setVisibility(
          mFavoritesRecyclerViewAdapter.getItemCount() > 0 ? View.VISIBLE : View.GONE);
      onSortingActionVisibilityStatusChange();
      onSortingActionEnableStatusChange();
      onDeleteAllActionVisibilityStatusChange();
      onDeleteAllActionEnableStatusChange();
    }
  }

  private void updateFavoritesRecyclerView() {
    if (mFavoritesRecyclerViewAdapter != null && mController != null)
    {
      mFavoritesRecyclerViewAdapter.setData(mController.getWords(), mController.getSelectedWords(), mController.isInSelectionMode(),
                                            mController.getEntryListFontSize());
    }
  }

  private void updateFavoritesProgressBar( boolean inTransition )
  {
    mFavoritesProgressBar.setVisibility(inTransition ? View.VISIBLE : View.INVISIBLE);
  }

  @Override
  public void onCreateOptionsMenu( Menu menu, MenuInflater inflater )
  {
    inflater.inflate(R.menu.favorites_toolbar_menu, menu);
    super.onCreateOptionsMenu(menu, inflater);
  }

  @Override
  public void onPrepareOptionsMenu( Menu menu )
  {
    mSortingItem = menu.findItem(R.id.favorites_toolbar_sorting_action);
    mDeleteAllItem = menu.findItem(R.id.favorites_toolbar_delete_all_action);
    onSortingActionVisibilityStatusChange();
    onSortingActionEnableStatusChange();
    onDeleteAllActionVisibilityStatusChange();
    onDeleteAllActionEnableStatusChange();
    // Restore favorites sorting popup menu state (show menu if needed)
    getView().postDelayed(new Runnable()
    {
      @Override
      public void run()
      {
        if (mShowFavoritesSortingPopupMenu)
        {
          createFavoritesSortingPopupMenu().show();
        }
      }
    }, 300);
    super.onPrepareOptionsMenu(menu);
  }

  @Override
  public boolean onOptionsItemSelected( MenuItem item )
  {
    if ( item.getItemId() == R.id.favorites_toolbar_sorting_action )
    {
      onFavoritesSortingActionClick();
      return true;
    }
    if ( item.getItemId() == R.id.favorites_toolbar_delete_all_action )
    {
      onDeleteAllFavoritesActionClick();
      return true;
    }
    return super.onOptionsItemSelected(item);
  }

  private void onFavoritesSortingActionClick( )
  {
    createFavoritesSortingPopupMenu().show();
    mShowFavoritesSortingPopupMenu = true;
  }

  private PopupMenu createFavoritesSortingPopupMenu()
  {
    if ( getActivity() != null )
    {
      View actionView = getActivity().findViewById(R.id.favorites_toolbar_sorting_action);
      mFavoritesSortingPopupMenu =
          new FavoritesSortingPopupMenu(getActivity(), actionView == null ? mFavoritesSortingLabel : actionView);
      mFavoritesSortingPopupMenu.setOnMenuItemClickListener(mFavoritesSortingPopupMenuItemClickListener);
      mFavoritesSortingPopupMenu.setOnDismissListener(mFavoritesSortingPopupMenuDismissListener);
      if ( mController != null &&  mController.getFavoritesSorting() != null )
      {
        mFavoritesSortingPopupMenu.setCheckedByOrdinal(true, mController.getFavoritesSorting().ordinal());
      }
    }
    return mFavoritesSortingPopupMenu;
  }

  private void onDeleteAllFavoritesActionClick()
  {
    SimpleDialog.show(this,
            DELETE_ALL_DIALOG_TAG,
            new SimpleDialog.Builder()
                    .setTitle(getString(R.string.favorites_manage_ui_clear_favorites))
                    .setMessage(getString(R.string.favorites_manage_ui_delete_all_articles))
                    .setPositiveText(getString(R.string.utils_slovoed_ui_confirm))
                    .setNegativeText(getString(R.string.utils_slovoed_ui_common_cancel)));
  }

  @Override
  public void onSaveInstanceState( @NonNull Bundle outState )
  {
    outState.putBoolean(SHOW_SORTING_POPUP_STATE_SAVE_TAG, mShowFavoritesSortingPopupMenu);
    super.onSaveInstanceState(outState);
  }

  @Override
  public void onActivityCreated( @Nullable Bundle savedInstanceState )
  {
    if ( savedInstanceState != null )
    {
      mShowFavoritesSortingPopupMenu = savedInstanceState.getBoolean(SHOW_SORTING_POPUP_STATE_SAVE_TAG, false);
    }
    super.onActivityCreated(savedInstanceState);
  }

  /**
   * Update text of label showing current favorites sorting type.
   * @param sorting current favorites sorting type
   */
  private void updateFavoritesSortingLabel( FavoritesSorting sorting )
  {
    if (sorting == null)
    {
      mFavoritesSortingLabel.setText("");
      return;
    }


    switch ( sorting )
    {
      case ALPHABETICALLY_ASCENDING:
        mFavoritesSortingLabel.setText(R.string.favorites_manage_ui_sort_alphabetically_ascending);
        break;
      case ALPHABETICALLY_DESCENDING:
        mFavoritesSortingLabel.setText(R.string.favorites_manage_ui_sort_alphabetically_descending);
        break;
      case BY_DATE_ASCENDING:
        mFavoritesSortingLabel.setText(R.string.favorites_manage_ui_sort_by_date_ascending);
        break;
      case BY_DATE_DESCENDING:
        mFavoritesSortingLabel.setText(R.string.favorites_manage_ui_sort_by_date_descending);
        break;
      default:
        mFavoritesSortingLabel.setText("");
    }
  }

  @Override
  public void onControllerFavoritesListChanged( List< ArticleItem > favoritesWords )
  {
    updateView();
  }

  @Override
  public void onSelectedListChanged( final List< ArticleItem > selectedWords )
  {
    // update existing list item views to keep standard android animation of CheckBox's
    mFavoritesRecyclerViewAdapter.setSelectedWords(selectedWords, false);
    mFavoritesRecyclerView.post(new Runnable()
    {
      @Override
      public void run()
      {
        for (int position = 0 ; position < mFavoritesRecyclerViewAdapter.getWords().size(); position++) {
          ArticleItem item = mFavoritesRecyclerViewAdapter.getWords().get(position);
          FavoritesRecyclerViewAdapter.FavoriteWordViewHolder viewHolder =
              (FavoritesRecyclerViewAdapter.FavoriteWordViewHolder)
                  mFavoritesRecyclerView.findViewHolderForAdapterPosition(position);
          if (viewHolder != null)
          {
            viewHolder.setChecked(selectedWords.contains(item));
          }
        }
      }
    });
    if (mController.getWords().size() == selectedWords.size())
    {
      mFavoritesRecyclerView.postDelayed(new Runnable()
      {
        @Override
        public void run()
        {
          mFavoritesRecyclerViewAdapter.setSelectedWords(selectedWords, true);
        }
      }, 400);
    }
  }

  @Override
  public void onSelectionModeChanged( boolean selectionMode )
  {
    updateFavoritesRecyclerView();
  }

  @Override
  public void onControllerError( Exception exception )
  {
    Toast.makeText(getContext(), exception.getMessage(), Toast.LENGTH_SHORT).show();
  }

  @Override
  public void onControllerEntryListFontSizeChanged()
  {
    updateFavoritesRecyclerView();
  }

  @Override
  public void onFavoritesSortingChanged()
  {
    updateFavoritesSortingLabel(mFavoritesRecyclerViewAdapter.getItemCount() > 0 ? mController.getFavoritesSorting() : null);
    mFavoritesRecyclerView.scrollToPosition(0);
  }

  @Override
  public void onControllerTransitionStateChanged( boolean inTransition )
  {
    updateFavoritesProgressBar(inTransition);
  }

  @Override
  public void onSortingActionVisibilityStatusChange()
  {
    changeActionVisibilityStatus(mSortingItem, mController.getSortingActionVisibilityStatus());
  }

  @Override
  public void onSortingActionEnableStatusChange()
  {
    changeActionEnabledStatus(mSortingItem, mController.isSortingActionEnable(), 66);
  }

  @Override
  public void onDeleteAllActionVisibilityStatusChange()
  {
    changeActionVisibilityStatus(mDeleteAllItem, mController.getDeleteAllActionVisibilityStatus());
  }

  @Override
  public void onDeleteAllActionEnableStatusChange()
  {
    changeActionEnabledStatus(mDeleteAllItem, mController.isDeleteAllActionEnable(), 66);
  }

  private void changeActionVisibilityStatus( @Nullable MenuItem menuItem, int status )
  {
    if ( menuItem != null )
    {
      menuItem.setVisible(status == View.VISIBLE);
    }
  }

  private void changeActionEnabledStatus( @Nullable MenuItem menuItem, boolean enabled, int disableAlpha )
  {
    if ( menuItem != null )
    {
      menuItem.setEnabled(enabled);
      menuItem.setIcon(menuItem.getIcon().getConstantState().newDrawable().mutate());
      menuItem.getIcon().setAlpha(enabled ? 255 : disableAlpha);
    }
  }

  @Override
  public void onCreateSimpleDialog( @Nullable String tag, @NonNull Dialog dialog, @Nullable Bundle extra )
  {

  }

  @Override
  public void onSimpleDialogButtonPressed( @Nullable String tag, int n, @Nullable Bundle extra )
  {
    if ( tag != null )
    {
      if ( tag.equals(DELETE_ALL_DIALOG_TAG) && ( n == Dialog.BUTTON_POSITIVE ) )
      {
        mController.deleteAllWords();
      }
      else if ( tag.equals(DELETE_SELECTED_DIALOG_TAG) && ( n == Dialog.BUTTON_POSITIVE ) )
      {
        mController.deleteSelectedWords();
      }
    }
  }

  @Override
  public void onControllerShowDeleteSelectedDialog()
  {
    SimpleDialog.show(this,
            DELETE_SELECTED_DIALOG_TAG,
            new SimpleDialog.Builder()
                    .setTitle(getString(R.string.favorites_manage_ui_clear_favorites))
                    .setMessage(getString(R.string.favorites_manage_ui_delete_selected_articles))
                    .setPositiveText(getString(R.string.utils_slovoed_ui_confirm))
                    .setNegativeText(getString(R.string.utils_slovoed_ui_common_cancel)));
  }
}
