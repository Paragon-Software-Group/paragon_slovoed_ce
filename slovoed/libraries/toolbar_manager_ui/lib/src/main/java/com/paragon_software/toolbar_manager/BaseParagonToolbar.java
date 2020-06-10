package com.paragon_software.toolbar_manager;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Parcelable;
import androidx.annotation.AttrRes;
import androidx.annotation.ColorInt;
import androidx.annotation.ColorRes;
import androidx.annotation.Nullable;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.widget.AppCompatSpinner;
import androidx.appcompat.widget.Toolbar;
import androidx.appcompat.widget.TooltipCompat;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.TextView;

import com.paragon_software.toolbar_manager.Notifiers.OnDeleteSelectedChange;
import com.paragon_software.toolbar_manager.Notifiers.OnDictionaryListChange;
import com.paragon_software.toolbar_manager.Notifiers.OnDirectionListChange;
import com.paragon_software.toolbar_manager.Notifiers.OnSelectionModeChange;
import com.paragon_software.toolbar_manager.Notifiers.OnShowBackgroundChange;
import com.paragon_software.toolbar_manager.Notifiers.OnTitleChange;
import com.paragon_software.toolbar_manager_ui.R;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class BaseParagonToolbar extends Toolbar implements BaseParagonToolbarAPI,
                                                           OnTitleChange, OnSelectionModeChange, OnDeleteSelectedChange,
                                                           View.OnClickListener, OnDictionaryListChange, OnShowBackgroundChange,
                                                           OnDirectionListChange, AdapterView.OnItemSelectedListener
{
  private static final String SUPER_STATE_SAVE_TAG = "super_state";
  private static final String SAVE_TITLE_KEY = "save_title_key";
  private static final String STATE_KEY_DIRECTION_LIST_ADAPTER     = "state_key_direction_list_adapter";

  private TextView  mTitle;
  private TextView  mSelectionModeTitle;

  private AppCompatSpinner      mDictionarySpinner;
  private ImageView             mDirectionIcon;
  private DirectionListAdapter  mDirectionListAdapter;
  private DictionaryListAdapter mDictionaryListAdapter;

  private ToolbarController mController;

  private ActionBar             mActionbar;
  private DrawerLayout          mDrawerLayout;
  private ActionBarDrawerToggle mToggle;

  private String title;

  private View.OnClickListener mDefaultToolbarNavigationClickListener;

  private final View.OnClickListener mSelectionModeToolbarNavigationClickListener = new View.OnClickListener()
  {
    @Override
    public void onClick( View view )
    {
      if (mController != null && mController.isShowSelectionMode()) {
        mController.backActionClick();
      }
    }
  };

  private final OnClickListener mHomeAsUpClickListener = new OnClickListener() {
    @Override
    public void onClick(View view) {
      mController.backActionClick();
    }
  };

  public BaseParagonToolbar( Context context )
  {
    super(context);
    initView(context);
  }

  public BaseParagonToolbar( Context context, @Nullable AttributeSet attrs )
  {
    super(context, attrs);
    initView(context);
  }

  public BaseParagonToolbar( Context context, @Nullable AttributeSet attrs, int defStyleAttr )
  {
    super(context, attrs, defStyleAttr);
    initView(context);
  }

  private void initController()
  {
    ToolbarManager toolbarManager = ToolbarManagerHolder.getManager();
    if ( toolbarManager != null )
    {
      mController = toolbarManager.getController(ToolbarControllerType.DEFAULT_CONTROLLER);
      mController.registerNotifier(this);
    }
  }

  private void freeController()
  {
    mController.unRegisterNotifier(this);
    ToolbarManager toolbarManager = ToolbarManagerHolder.getManager();
    if ( toolbarManager != null )
    {
      toolbarManager.freeController(ToolbarControllerType.DEFAULT_CONTROLLER);
    }
  }

  private void initView(Context context)
  {
    setTitle("");
    mTitle = inflate(context, R.layout.toolbar_title, this).findViewById(R.id.toolbar_title);
    mSelectionModeTitle = inflate(context, R.layout.toolbar_selection_mode_title, this).findViewById(R.id.toolbar_selection_mode_title);
    mSelectionModeTitle.setTextColor(getSelectionModeToolbarTitleColor());
    initDictionaryList(context);
  }

  private void initDictionaryList(Context context)
  {
    ConstraintLayout dictionaryList = inflate(context, R.layout.toolbar_dicitionary_list, this).findViewById(R.id.toolbar_list);
    mDictionarySpinner = dictionaryList.findViewById(R.id.dictionary_spinner);
    mDictionaryListAdapter = new DictionaryListAdapter(context, R.layout.toolbar_default_dictionary_item, R.layout.toolbar_dictionary_item);
    mDictionarySpinner.setAdapter(mDictionaryListAdapter);
    mDictionarySpinner.setOnItemSelectedListener(this);
    mDirectionIcon = dictionaryList.findViewById(R.id.direction_icon);
    TooltipCompat.setTooltipText(mDirectionIcon, getResources().getText(R.string.utils_slovoed_ui_switch_direction));
    mDirectionIcon.setOnClickListener(this);
    mDirectionListAdapter = new DirectionListAdapter();
    mDirectionListAdapter.setImageView(mDirectionIcon);
  }

  @Override
  public void onResume()
  {
    initController();
    restoreTitle();
    updateView();
  }

	private void restoreTitle() {
		if (mController != null) {
			if (title != null) {
				mController.restoreTitle(title);
				title = null;
			}
		}
	}

  @Override
  public void onPause()
  {
    freeController();
  }

  @Override
  public boolean onCreateOptionsMenu( Menu menu, MenuInflater inflater )
  {
    inflater.inflate(R.menu.toolbar_menu, menu);
    return true;
  }

  @Override
  public boolean onPrepareOptionsMenu( Menu menu )
  {
    if (mController != null)
    {
      updateView();
    }
    return true;
  }

  @Override
  public boolean onOptionsItemSelected( MenuItem item )
  {
    if (mController != null)
    {
      if ( item.getItemId() == R.id.toolbar_select_all_action )
      {
        mController.selectAllActionClick();
        return true;
      }
      else if ( item.getItemId() == R.id.toolbar_delete_selected_action )
      {
        onDeleteSelectedActionClick();
        return true;
      }
    }
    return false;
  }

  @Override
  public void setActionbar( @Nullable ActionBar actionbar )
  {
    mActionbar = actionbar;
  }

  @Override
  public void setDrawerLayout( @Nullable DrawerLayout drawerLayout )
  {
    mDrawerLayout = drawerLayout;
  }

  @Override
  public void setToggle( @Nullable ActionBarDrawerToggle toggle )
  {
    mToggle = toggle;
    if (mToggle != null)
    {
      mDefaultToolbarNavigationClickListener = mToggle.getToolbarNavigationClickListener();
    }
  }

  @Override
  public Parcelable onSaveInstanceState()
  {
    Bundle bundle = new Bundle();
    bundle.putParcelable(SUPER_STATE_SAVE_TAG, super.onSaveInstanceState());
    bundle.putString(SAVE_TITLE_KEY, mController.getTitle());
    bundle.putBundle(STATE_KEY_DIRECTION_LIST_ADAPTER, mDirectionListAdapter.onSaveInstanceState());
    return bundle;
  }

  @Override
  public void onRestoreInstanceState( Parcelable state )
  {
    if ( state instanceof Bundle )
    {
      Bundle bundle = (Bundle) state;
      title = bundle.getString(SAVE_TITLE_KEY);
      state = bundle.getParcelable(SUPER_STATE_SAVE_TAG);
      Bundle bundleForDirectionAdapter = bundle.getBundle(STATE_KEY_DIRECTION_LIST_ADAPTER);
      if (null != bundleForDirectionAdapter)
      {
        mDirectionListAdapter.onRestoreInstanceState(bundleForDirectionAdapter);
      }
    }
    super.onRestoreInstanceState(state);
  }

  private void onDeleteSelectedActionClick()
  {
    mController.deleteSelectedActionClick();
  }

  private void changeActionVisibilityStatus( int actionId, int status )
  {
    MenuItem menuItem = getMenu().findItem(actionId);
    if (menuItem != null)
    {
      menuItem.setVisible(status == View.VISIBLE ? true : false);
    }
  }

  private void changeActionEnabledStatus( int actionId, boolean enabled, int disableAlpha )
  {
    MenuItem menuItem = getMenu().findItem(actionId);
    if (menuItem != null)
    {
      menuItem.setEnabled(enabled);
      menuItem.setIcon(menuItem.getIcon().getConstantState().newDrawable().mutate());
      menuItem.getIcon().setAlpha(enabled ? 255 : disableAlpha);
    }
  }

  private void updateView()
  {
    if (mController != null)
    {
      mTitle.setVisibility(mController.isShowTitle() ? View.VISIBLE : View.GONE);
      mTitle.setText(mController.getTitle());
      updateHomeAsUpIndicator(mController.isShowHomeAsUp());
      updateToolbarSelectionMode(mController.isShowSelectionMode() ? View.VISIBLE : View.GONE);
      onDeleteSelectedActionVisibilityStatusChange(mController.isShowDeleteSelectedAction() ? View.VISIBLE : View.GONE);
      updateDictionaryList();
    }
  }

  private void updateDictionaryList()
  {
    mDictionarySpinner.setVisibility(mController.isShowDictionaryList() ? View.VISIBLE : View.GONE);
    mDirectionIcon.setVisibility(mController.isShowDirectionList() ? View.VISIBLE : View.GONE);
    final List<DictionaryView> dictionaryViews = (List<DictionaryView>) mController.getDictionaryList();
    updateDictionarySpinnerView(dictionaryViews);
    final int currentDictionary = mDictionaryListAdapter.getDictionaryViewPosition(mController.getSelectedDictionaryItem());
    final ArrayList<DirectionView> directionViews = (ArrayList<DirectionView>) dictionaryViews.get(currentDictionary).getDirectionViews();
    mDirectionListAdapter.setDirections(directionViews);
  }

  private void updateDictionarySpinnerView( Collection<DictionaryView> dictionaryViews )
  {
    mDictionaryListAdapter.setData(dictionaryViews);
    final int currentDictionary = mDictionaryListAdapter.getDictionaryViewPosition(mController.getSelectedDictionaryItem());
    mDictionarySpinner.setSelection(currentDictionary);
    mDictionarySpinner.setEnabled(mDictionaryListAdapter.getCount() > 1);
  }

  @Override
  public void onTitleTextChange( String text )
  {
    mTitle.setText(text);
  }

  @Override
  public void onTitleStatusChange( int status )
  {
    mTitle.setVisibility(status);
  }

  @Override
  public void onSelectionModeStatusChange( int status )
  {
    updateToolbarSelectionMode(status);
  }

  @Override
  public void onNumberOfSelectedItemsChange( int numberOfSelectedItems )
  {
    mSelectionModeTitle.setText(getResources().getString(R.string.toolbar_manager_ui_selected_mode_title,
                                                 numberOfSelectedItems));
    enableSelectionModeActions(numberOfSelectedItems > 0);
  }

  private void updateToolbarSelectionMode( int status ) {
    mSelectionModeTitle.setVisibility(status);
    setBackgroundColor(status == View.VISIBLE ? getSelectionModeToolbarBackgroundColor() : getToolbarBackgroundColor());
    onNumberOfSelectedItemsChange(mController.getNumberOfSelectedItems());
    changeActionVisibilityStatus(R.id.toolbar_select_all_action, status);
    updateHomeAsUpIndicatorForSelectionMode(status == View.VISIBLE);
  }

  private @ColorInt int getTypedValueAttrColor( @AttrRes int attr, @ColorRes int defaultColorRes )
  {
    TypedArray typedArray = getContext().obtainStyledAttributes(new TypedValue().data, new int[] { attr});
    int color = typedArray.getColor(0, getResources().getColor(defaultColorRes));
    typedArray.recycle();
    return color;
  }

  private @ColorInt int getToolbarBackgroundColor()
  {
    return mController.isShowBackground() ? getTypedValueAttrColor(
        R.attr.colorPrimary, R.color.toolbar_manager_background_color) : Color.TRANSPARENT;
  }

  private @ColorInt int getSelectionModeToolbarBackgroundColor()
  {
    return getTypedValueAttrColor(R.attr.selectionModeToolbarBackgroundColor, R.color.toolbar_manager_selection_mode_background_color);
  }

  private @ColorInt int getSelectionModeToolbarTitleColor()
  {
    return getTypedValueAttrColor(R.attr.selectionModeToolbarTitleColor, R.color.toolbar_manager_selection_mode_title_color);
  }

  private void updateHomeAsUpIndicatorForSelectionMode( boolean active )
  {
    if ( mActionbar != null )
    {
      mActionbar.setDisplayHomeAsUpEnabled(active);
    }
    if (mDrawerLayout != null)
    {
      mDrawerLayout.setDrawerLockMode(active ? DrawerLayout.LOCK_MODE_LOCKED_CLOSED : DrawerLayout.LOCK_MODE_UNLOCKED);
    }
    if (mToggle != null)
    {
      mToggle.setDrawerIndicatorEnabled(!active);
      mToggle.setToolbarNavigationClickListener(active ? mSelectionModeToolbarNavigationClickListener : mDefaultToolbarNavigationClickListener);
      if ( active )
      {
        mToggle.setHomeAsUpIndicator(R.drawable.ic_arrow_back_light);
      }
    }
  }

  private void updateHomeAsUpIndicator(boolean active) {
    if (mActionbar != null && mToggle != null) {
      mToggle.setDrawerIndicatorEnabled(!active);
      mActionbar.setDisplayHomeAsUpEnabled(active);
      mToggle.setToolbarNavigationClickListener(active ? mHomeAsUpClickListener : mDefaultToolbarNavigationClickListener);
    }
    if (mDrawerLayout != null) {
      mDrawerLayout.setDrawerLockMode(active ? DrawerLayout.LOCK_MODE_LOCKED_CLOSED : DrawerLayout.LOCK_MODE_UNLOCKED);
    }
  }

  private void enableSelectionModeActions(boolean enable)
  {
    changeActionEnabledStatus(R.id.toolbar_delete_selected_action, enable, 77);
  }

  @Override
  public void onClick( View view )
  {
    if (view == mDirectionIcon)
    {
      mController.selectDirectionView(mDirectionListAdapter.changeDirection());
    }
  }

  @Override
  public void onDictionaryListContentChange()
  {
    updateDictionarySpinnerView(mController.getDictionaryList());
  }

  @Override
  public void onDictionaryListStatusChange( int status )
  {
    mDictionarySpinner.setVisibility(status);
  }

  @Override
  public void onDictionaryListSelectionChange( int element )
  {
    mDictionarySpinner.setSelection(element);
  }

  @Override
  public void onDirectionListStatusChange( int status )
  {
    mDirectionIcon.setVisibility(status);
  }

  @Override
  public void onDirectionListContentChange( Collection< DirectionView > collection )
  {
    mDirectionListAdapter.setDirection(DirectionListAdapter.UNDEFINED_DIRECTION_INDEX);
    mDirectionListAdapter.setDirections((ArrayList<DirectionView>) collection);
  }

  @Override
  public void onDirectionChange( int element )
  {
    mDirectionListAdapter.setDirection(element);
  }

  @Override
  public void onItemSelected( AdapterView< ? > parent, View view, int position, long id )
  {
    mController.selectDictionaryItem(mDictionaryListAdapter.getDictionaryView(position));
  }

  @Override
  public void onNothingSelected( AdapterView< ? > parent )
  {

  }

  @Override
  public void onDeleteSelectedActionVisibilityStatusChange( int status )
  {
    changeActionVisibilityStatus(R.id.toolbar_delete_selected_action, status);
  }

  @Override
  public void onOnShowBackgroundStatusChange( boolean showBackground )
  {

  }
}
