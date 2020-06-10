package com.paragon_software.history_manager;

import android.app.Dialog;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.Toast;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

import com.paragon_software.history_manager_ui.R;
import com.paragon_software.utils_slovoed_ui_common.fragments.SimpleDialog;

public class HistoryFragment extends Fragment implements OnControllerHistoryListChangedListener,
                                                         OnControllerSelectionModeChangedListener,
                                                         OnControllerErrorListener,
                                                         OnControllerEntryListFontSizeChangeListener,
                                                         OnControllerTransitionStateChangedListener,
                                                         OnControllerDeleteAllActionChange, OnControllerShowDeleteSelectedDialogListener,
                                                         SimpleDialog.Target
{
  private static final String DELETE_ALL_DIALOG_TAG = "delete_all_dialog_tag";
  private static final String DELETE_SELECTED_DIALOG_TAG = "delete_selected_dialog_tag";

  private RecyclerView                 mHistoryRecyclerView;
  private HistoryRecyclerViewAdapter   mHistoryRecyclerViewAdapter;

  private ProgressBar mHistoryProgressBar;

  /**
   * Controller to provide communication between history manager and the view.
   */
  private HistoryControllerAPI mController;

  @Nullable
  private MenuItem mDeleteAllItem;

  @Override
  public void onCreate( @Nullable Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
  }

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
    HistoryManagerAPI historyManager = HistoryManagerHolder.getManager();
    if ( historyManager != null )
    {
      mController = historyManager.getController(HistoryControllerType.DEFAULT_CONTROLLER);
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
    HistoryManagerAPI historyManager = HistoryManagerHolder.getManager();
    if ( historyManager != null )
    {
      historyManager.freeController(HistoryControllerType.DEFAULT_CONTROLLER);
    }
    super.onPause();
  }

  @Nullable
  @Override
  public View onCreateView( @NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                            @Nullable Bundle savedInstanceState )
  {
    setHasOptionsMenu(true);
    View view = inflater.inflate(R.layout.fragment_history, container,false);
    initViews(view);
    return view;
  }

  @Override
  public void onCreateOptionsMenu( Menu menu, MenuInflater inflater )
  {
    inflater.inflate(R.menu.history_toolbar_menu, menu);
    super.onCreateOptionsMenu(menu, inflater);
  }

  @Override
  public void onPrepareOptionsMenu( Menu menu )
  {
    mDeleteAllItem = menu.findItem(R.id.history_toolbar_delete_all_action);
    onDeleteAllActionVisibilityStatusChange();
    onDeleteAllActionEnableStatusChange();
    super.onPrepareOptionsMenu(menu);
  }

  @Override
  public boolean onOptionsItemSelected( MenuItem item )
  {
    if ( item.getItemId() == R.id.history_toolbar_delete_all_action )
    {
      onDeleteAllHistoryActionClick();
      return true;
    }
    return super.onOptionsItemSelected(item);
  }

  private void onDeleteAllHistoryActionClick()
  {
    SimpleDialog.show(this, DELETE_ALL_DIALOG_TAG,
            new SimpleDialog.Builder()
                    .setTitle(getString(R.string.history_manage_ui_clear_history))
                    .setMessage(getString(R.string.history_manage_ui_delete_all_articles))
                    .setPositiveText(getString(R.string.utils_slovoed_ui_confirm))
                    .setNegativeText(getString(R.string.utils_slovoed_ui_common_cancel)));
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
    mHistoryProgressBar = fragmentView.findViewById(R.id.history_progress_bar);
    mHistoryRecyclerView = fragmentView.findViewById(R.id.history_list);
    mHistoryRecyclerView.setLayoutManager(new LinearLayoutManager(getContext()));
    mHistoryRecyclerViewAdapter = new HistoryRecyclerViewAdapter();
    initHistoryRecyclerViewAdapterListeners(mHistoryRecyclerViewAdapter);
    mHistoryRecyclerView.setAdapter(mHistoryRecyclerViewAdapter);
  }

  private void initHistoryRecyclerViewAdapterListeners(HistoryRecyclerViewAdapter historyRecyclerViewAdapter) {
    historyRecyclerViewAdapter.setOnItemClickListener(new HistoryRecyclerViewAdapter.OnItemClickListener()
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
    historyRecyclerViewAdapter.setOnItemLongClickListener(new HistoryRecyclerViewAdapter.OnItemLongClickListener()
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
    historyRecyclerViewAdapter.setOnItemCheckedChangeListener(new HistoryRecyclerViewAdapter.OnItemCheckedChangeListener()
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
    if (getView() != null && mHistoryRecyclerViewAdapter != null && mController != null)
    {
      mHistoryRecyclerViewAdapter.setHistoryController(mController);
      updateHistoryRecyclerView();
      updateHistoryProgressBar(mController.isInTransition());
      getView().findViewById(R.id.empty_label).setVisibility(
          mHistoryRecyclerViewAdapter.getItemCount() > 0 ? View.GONE : View.VISIBLE);
      getView().findViewById(R.id.history_content_card).setVisibility(
          mHistoryRecyclerViewAdapter.getItemCount() > 0 ? View.VISIBLE : View.GONE);
      onDeleteAllActionVisibilityStatusChange();
      onDeleteAllActionEnableStatusChange();
    }
  }

  private void updateHistoryRecyclerView() {
    if (mHistoryRecyclerViewAdapter != null && mController != null)
    {
      mHistoryRecyclerViewAdapter.setData(mController.getWords(), mController.getSelectedWords(), mController.isInSelectionMode(),
                                            mController.getEntryListFontSize());
    }
  }

  private void updateHistoryProgressBar( boolean inTransition )
  {
    mHistoryProgressBar.setVisibility(inTransition ? View.VISIBLE : View.INVISIBLE);
  }

  @Override
  public void onControllerHistoryListChanged( List< ArticleItem > historyWords )
  {
    updateView();
  }

  @Override
  public void onSelectedListChanged( final List< ArticleItem > selectedWords )
  {
    // update existing list item views to keep standard android animation of CheckBox's
    mHistoryRecyclerViewAdapter.setSelectedWords(selectedWords, false);
    mHistoryRecyclerView.post(new Runnable()
    {
      @Override
      public void run()
      {
        for (int position = 0 ; position < mHistoryRecyclerViewAdapter.getWords().size(); position++) {
          ArticleItem item = mHistoryRecyclerViewAdapter.getWords().get(position);
          HistoryRecyclerViewAdapter.HistoryWordViewHolder viewHolder =
              (HistoryRecyclerViewAdapter.HistoryWordViewHolder)
                  mHistoryRecyclerView.findViewHolderForAdapterPosition(position);
          if (viewHolder != null)
          {
            viewHolder.setChecked(selectedWords.contains(item));
          }
        }
      }
    });
    if (mController.getWords().size() == selectedWords.size())
    {
      mHistoryRecyclerView.postDelayed(new Runnable()
      {
        @Override
        public void run()
        {
          mHistoryRecyclerViewAdapter.setSelectedWords(selectedWords, true);
        }
      }, 400);
    }
  }

  @Override
  public void onSelectionModeChanged( boolean selectionMode )
  {
    updateHistoryRecyclerView();
  }

  @Override
  public void onControllerError( Exception exception )
  {
    Toast.makeText(getContext(), exception.getMessage(), Toast.LENGTH_SHORT).show();
  }

  @Override
  public void onControllerEntryListFontSizeChanged()
  {
    updateHistoryRecyclerView();
  }

  @Override
  public void onControllerTransitionStateChanged( boolean inTransition )
  {
    updateHistoryProgressBar(inTransition);
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
    SimpleDialog.show(this, DELETE_SELECTED_DIALOG_TAG,
            new SimpleDialog.Builder()
                    .setTitle(getString(R.string.history_manage_ui_clear_history))
                    .setMessage(getString(R.string.history_manage_ui_delete_selected_articles))
                    .setPositiveText(getString(R.string.utils_slovoed_ui_confirm))
                    .setNegativeText(getString(R.string.utils_slovoed_ui_common_cancel)));
  }
}
