package com.paragon_software.toolbar_manager;

import android.graphics.Bitmap;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.Dictionary.DictionaryId;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.dictionary_manager.exceptions.NotALocalResourceException;
import com.paragon_software.toolbar_manager.Notifiers.OnDeleteSelectedChange;
import com.paragon_software.toolbar_manager.Notifiers.OnDictionaryListChange;
import com.paragon_software.toolbar_manager.Notifiers.OnDirectionListChange;
import com.paragon_software.toolbar_manager.Notifiers.OnSelectionModeChange;
import com.paragon_software.toolbar_manager.Notifiers.OnShowBackgroundChange;
import com.paragon_software.toolbar_manager.Notifiers.OnTitleChange;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class ParagonToolbarController implements ToolbarController
{

  private Collection < DictionaryView >                       mDictionaryViews                  = new ArrayList<>();
  private final ArrayList < OnTitleChange >                   mOnTitleChanges                   = new ArrayList<>();
  private final ArrayList < OnDictionaryListChange >          mOnDictionaryListChanges          = new ArrayList<>();
  private final ArrayList < OnDirectionListChange >           mOnDirectionListChanges           = new ArrayList<>();
  private final ArrayList < OnSelectionModeChange >           mOnSelectionModeChanges           = new ArrayList<>();
  private final ArrayList < OnDeleteSelectedChange >          mOnDeleteSelectedChanges          = new ArrayList<>();
  private final ArrayList < OnShowBackgroundChange >          mOnShowBackgroundChanges          = new ArrayList<>();

  private final ParagonToolbarManager mToolbarManager;
  private String mTitle;
  private int mNumberOfSelectedItems;
  private DictionaryView mSelectedDictionary;
  private int mTitleStatus = View.VISIBLE;
  private int mDictionaryStatus = View.GONE;
  private int mDirectionStatus = View.GONE;

  private int mSelectionModeStatus = View.GONE;

  private int mDeleteSelectedActionStatus = View.GONE;
  private boolean mShowBackground = true;

  private boolean FTS_MODE = false;

  private LocalizedString mDefaultDictionaryTitle;
  private Bitmap mDefaultDictionaryIcon;
  private DirectionView mSelectedDirection;
  private boolean mHomeAsUp = false;

  ParagonToolbarController( @NonNull ParagonToolbarManager manager,
                            @NonNull String title )
  {
    mToolbarManager = manager;
    mTitle = title;
  }

  ParagonToolbarController(@NonNull ParagonToolbarManager manager,
                           @NonNull Collection <Dictionary> list,
                           @Nullable DictionaryId id,
                           @NonNull LocalizedString defaultDictionaryTitle,
                           @Nullable Bitmap defaultDictionaryBitmap)
  {
    mToolbarManager = manager;
    mDefaultDictionaryTitle = defaultDictionaryTitle;
    mDefaultDictionaryIcon = defaultDictionaryBitmap;
    mDictionaryViews = convertToDictionaryViews(list);
    mSelectedDictionary = getDictionaryViewFromId(id);
    mSelectedDirection = getDefaultDirection(mSelectedDictionary);
  }

  private Collection<DictionaryView> convertToDictionaryViews(Collection<Dictionary> dictionaries)
  {
    Collection<DictionaryView> dictionaryViews  = new ArrayList<>();
    IDictionaryIcon icon = new IDictionaryIcon() {
      @Nullable
      @Override
      public Bitmap getBitmap() {
        return mDefaultDictionaryIcon;
      }

      @Override
      public int getResourceId() throws NotALocalResourceException {
        return 0;
      }
    };
    dictionaryViews.add(new DictionaryView(null,mDefaultDictionaryTitle,icon,null));
    if (null != dictionaries)
    {
      for ( final Dictionary dictionary : dictionaries )
      {
        dictionaryViews.add(new DictionaryView(dictionary.getId()
            ,dictionary.getTitle()
            ,dictionary.getIcon()
            ,directionToDirectionView(dictionary.getDirections())));
      }
    }
    return dictionaryViews;
  }

  private Collection<DirectionView > directionToDirectionView( Collection<Dictionary.Direction> directions )
  {
    if (null == directions) return null;
    Collection<DirectionView> directionViews = new ArrayList<>();
    for ( Dictionary.Direction direction : directions)
    {
      directionViews.add(new DirectionView(direction.getLanguageFrom(), direction.getLanguageTo(), direction.getIcon()));
    }
    return directionViews;
  }

  private DictionaryView getDictionaryViewFromId(DictionaryId id)
  {
    for (DictionaryView view : mDictionaryViews)
    {
      if ( ( view.getId() == null && id == null ) || ( id != null && id.equals(view.getId()) ) )
      {
        return view;
      }
    }
    return null;
  }



  @Override
  @NonNull
  public String getTitle()
  {
    return mTitle;
  }

  @Override
  public boolean isShowTitle()
  {
    return View.VISIBLE == mTitleStatus;
  }

  @Override
  public int getNumberOfSelectedItems()
  {
    return mNumberOfSelectedItems;
  }

  @Override
  @NonNull
  public Collection< DictionaryView > getDictionaryList()
  {
      DictionaryId currentDictId = null;
      if (null != mSelectedDictionary)
      {
        currentDictId = mSelectedDictionary.getId();
      }

      Collection<DictionaryView> dictionaries;

      if (!FTS_MODE || null == currentDictId || mDictionaryViews.size() < 2)
      {
        dictionaries = mDictionaryViews;
      }
      else
      {
        dictionaries = ((List)mDictionaryViews).subList(1,mDictionaryViews.size());
      }

      return dictionaries;
  }

  @Override
  public boolean isShowDictionaryList()
  {
    return View.VISIBLE == mDictionaryStatus;
  }

  @Override
  public boolean isShowDirectionList()
  {
    return View.VISIBLE == mDirectionStatus;
  }

  @Override
  public boolean isShowSelectionMode()
  {
    return View.VISIBLE == mSelectionModeStatus;
  }

  @Override
  public boolean isShowDeleteSelectedAction()
  {
    return View.VISIBLE == mDeleteSelectedActionStatus;
  }

  @Override
  public boolean isShowBackground()
  {
    return mShowBackground;
  }

  @Override
  public void selectDictionaryItem( @NonNull DictionaryView dictionaryView )
  {
    if (null != mSelectedDictionary)
    {
        DictionaryId dictionaryId = mSelectedDictionary.getId();
        if (null == dictionaryId && null == dictionaryView.getId())
        {
            return;
        }
        if (null != dictionaryId && dictionaryId.equals(dictionaryView.getId()))
        {
            return;
        }
    }
    boolean updateDictionaryList = (null == mSelectedDictionary.getId() && null != dictionaryView.getId());
    mSelectedDictionary = dictionaryView;
    notifyDirectionListContentChanged(mSelectedDictionary.getDirectionViews());
    mSelectedDirection = getDefaultDirection(dictionaryView);
    notifyDirectionChanged(getDirectionElement(mSelectedDictionary,mSelectedDirection));
    if (updateDictionaryList)
    {
      notifyDictionaryListContentChanged();
    }
    mToolbarManager.changeSelectedDictionary(dictionaryView.getId());
  }

  private int getDirectionElement(DictionaryView dictionaryView, DirectionView directionView)
  {
    ArrayList<DirectionView> directionViews = (ArrayList<DirectionView>)dictionaryView.getDirectionViews();
    if (null == directionViews)
    {
      return -2;
    }
    else
    {
      return directionViews.indexOf(directionView);
    }
  }

  private DirectionView getDefaultDirection(DictionaryView view)
  {
    ArrayList<DirectionView> directionViews = (ArrayList<DirectionView>) view.getDirectionViews();
    if (null == directionViews)
    {
      return null;
    }
    else
    {
      return directionViews.get(0);
    }
  }

  @Override
  @NonNull
  public DictionaryView getSelectedDictionaryItem()
  {
    return mSelectedDictionary;
  }

  @Override
  public void selectDirectionView( @Nullable DirectionView directionView )
  {
    mSelectedDirection = directionView;
    if (null == mSelectedDirection)
    {
      mToolbarManager.changeSelectedDirection(-2);
    }
    else
    {
      mToolbarManager.changeSelectedDirection(mSelectedDirection.getId());
    }
  }

  @Override
  public DirectionView getSelectedDirectionView()
  {
    return mSelectedDirection;
  }

  @Override
  public void changeDirection()
  {
    ArrayList<DirectionView> directionViews = (ArrayList<DirectionView>) mSelectedDictionary.getDirectionViews();
    if (null == directionViews)
    {
      mSelectedDirection = null;
    }
    else if (1 == directionViews.size())
    {
      mSelectedDirection = directionViews.get(0);
    }
    else
    {
      if (null == mSelectedDirection)
      {
        mSelectedDirection = directionViews.get(0);
      }
      else
      {
        int index = directionViews.indexOf(mSelectedDirection);
        if (-1 == index)
        {
          mSelectedDirection = directionViews.get(0);
        }
        else if (index + 1 < directionViews.size())
        {
          mSelectedDirection = directionViews.get(index + 1);
        }
        else
        {
          mSelectedDirection = directionViews.get(0);
        }
      }
    }

    int selectedDirection;
    if (null == mSelectedDirection)
    {
      selectedDirection = -2;
    }
    else
    {
      selectedDirection = mSelectedDirection.getId();
    }

    mToolbarManager.changeSelectedDirection(selectedDirection);
  }

  @Override
  public void deleteSelectedActionClick()
  {
    mToolbarManager.deleteSelectedActionClick();
  }

  @Override
  public void selectAllActionClick()
  {
    mToolbarManager.selectAllActionClick();
  }

  @Override
  public void backActionClick()
  {
    mToolbarManager.backActionClick();
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnTitleChange && !mOnTitleChanges.contains(notifier))
    {
      mOnTitleChanges.add((OnTitleChange) notifier);
    }
    if (notifier instanceof OnDictionaryListChange && !mOnDictionaryListChanges.contains(notifier))
    {
      mOnDictionaryListChanges.add((OnDictionaryListChange)notifier);
    }
    if (notifier instanceof OnDirectionListChange && !mOnDirectionListChanges.contains(notifier))
    {
      mOnDirectionListChanges.add((OnDirectionListChange)notifier);
    }
    if (notifier instanceof OnSelectionModeChange && !mOnSelectionModeChanges.contains(notifier))
    {
      mOnSelectionModeChanges.add((OnSelectionModeChange)notifier);
    }
    if (notifier instanceof OnDeleteSelectedChange && !mOnDeleteSelectedChanges.contains(notifier))
    {
      mOnDeleteSelectedChanges.add((OnDeleteSelectedChange)notifier);
    }
    if (notifier instanceof OnShowBackgroundChange && !mOnShowBackgroundChanges.contains(notifier))
    {
      mOnShowBackgroundChanges.add((OnShowBackgroundChange) notifier);
    }
  }

  @Override
  public void unRegisterNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnTitleChange)
    {
      mOnTitleChanges.remove(notifier);
    }
    if (notifier instanceof OnDictionaryListChange)
    {
      mOnDictionaryListChanges.remove(notifier);
    }
    if (notifier instanceof OnDirectionListChange)
    {
      mOnDirectionListChanges.remove(notifier);
    }
    if (notifier instanceof OnSelectionModeChange)
    {
      mOnSelectionModeChanges.remove(notifier);
    }
    if (notifier instanceof OnDeleteSelectedChange)
    {
      mOnDeleteSelectedChanges.remove(notifier);
    }
    if (notifier instanceof OnShowBackgroundChange)
    {
      mOnShowBackgroundChanges.remove(notifier);
    }
  }

  @Override
  public boolean isShowHomeAsUp() {
    return mHomeAsUp;
  }

  public void setDictionaryList(Collection<Dictionary> list)
  {
    mDictionaryViews = convertToDictionaryViews(list);
    notifyDictionaryListContentChanged();
  }

  public void setDictionaryList(@NonNull Collection<Dictionary> list, @Nullable DictionaryId id)
  {
    setDictionaryList(list);
    selectDictionary(id, null);
  }

  public void selectDictionary(@Nullable DictionaryId dictionaryId, @Nullable Dictionary.Direction direction)
  {
    if(getDictionaryViewFromId(dictionaryId)!=null)
    mSelectedDictionary = getDictionaryViewFromId(dictionaryId);
    mToolbarManager.changeSelectedDictionary(mSelectedDictionary.getId());
    notifyDictionaryListSelectionChanged(getElementNumber(dictionaryId));
    Collection<DirectionView> directionViews = mSelectedDictionary.getDirectionViews();
    notifyDirectionListContentChanged(directionViews);
    mSelectedDirection = null;
    if((direction != null) && (directionViews != null))
      for(DirectionView directionView : directionViews)
        if(directionView != null)
          if((direction.getLanguageFrom() == directionView.getId()) && (direction.getLanguageTo() == directionView.getDirectonTo())) {
            mSelectedDirection = directionView;
            break;
          }
    if(mSelectedDirection == null)
      mSelectedDirection = getDefaultDirection(mSelectedDictionary);
    if (null == mSelectedDirection)
    {
      mToolbarManager.changeSelectedDirection(-2);
    }
    else
    {
      mToolbarManager.changeSelectedDirection(mSelectedDirection.getId());
    }
    notifyDirectionChanged(getDirectionElement(mSelectedDictionary,mSelectedDirection));
  }

  public void selectDirection(@Nullable Dictionary.Direction selectedDirection)
  {
    if (null == selectedDirection || null == mSelectedDirection)
    {
      return;
    }
    if (mSelectedDirection.getId() != selectedDirection.getLanguageFrom() ||
            mSelectedDirection.getDirectonTo() != selectedDirection.getLanguageTo())
    {
      Collection<DirectionView> directionViews = mSelectedDictionary.getDirectionViews();
      for(DirectionView directionView : directionViews)
      {
        if (directionView.getId() == selectedDirection.getLanguageFrom() &&
                directionView.getDirectonTo() == selectedDirection.getLanguageTo())
        {
          mSelectedDirection = directionView;
          notifyDirectionChanged(getDirectionElement(mSelectedDictionary, mSelectedDirection));
          break;
        }
      }
    }
  }

  public void showDictionary()
  {
    showDictionary(FTS_MODE);
  }

  public void showDictionary(boolean fts_mode)
  {
    resetStatus();
    FTS_MODE = fts_mode;
    mDictionaryStatus = View.VISIBLE;
    mDirectionStatus = View.VISIBLE;

    notifyStatusChange();
  }

  public void showDictionaryNoDirection()
  {
    resetStatus();
    mDictionaryStatus = View.VISIBLE;

    notifyStatusChange();
  }

  public void showTitleAndDirectionInToolbar()
  {
    resetStatus();
    mDirectionStatus = View.VISIBLE;
    mTitleStatus = View.GONE;
    mShowBackground = true;

    notifyStatusChange();
  }

  public void showSelectionMode(int numberOfSelectedItems)
  {
    mDictionaryStatus = View.GONE;
    mDirectionStatus = View.GONE;
    mTitleStatus = View.GONE;
    mSelectionModeStatus = View.VISIBLE;
    mDeleteSelectedActionStatus = View.VISIBLE;

    notifyStatusChange();
    if (mNumberOfSelectedItems != numberOfSelectedItems) {
      mNumberOfSelectedItems = numberOfSelectedItems;
      notifyNumberOfSelectedItemsChanged(mNumberOfSelectedItems);
    }
  }

  public void showHomeAsUp(boolean visibility)
  {
    mHomeAsUp = visibility;
  }

  public void showTitle( boolean showBackground )
  {
    resetStatus();
    mTitleStatus = View.VISIBLE;
    mShowBackground = showBackground;

    notifyStatusChange();
  }

  private void resetStatus()
  {
    FTS_MODE = false;
    mTitleStatus = View.GONE;
    mDictionaryStatus = View.GONE;
    mDirectionStatus = View.GONE;
    mDeleteSelectedActionStatus = View.GONE;
    mSelectionModeStatus = View.GONE;
    mShowBackground = true;
  }

  private void notifyStatusChange()
  {
    notifyTitleStatusChanged(mTitleStatus);
    notifyDictionaryListStatusChanged(mDictionaryStatus);
    notifyDirectionListStatusChanged(mDirectionStatus);
    notifySelectionModeStatusChanged(mSelectionModeStatus);
    notifyDeleteSelectedStatusChanged(mDeleteSelectedActionStatus);
    notifyShowBackgroundChanged(mShowBackground);
    notifyDictionaryListContentChanged();
  }

  private void notifyShowBackgroundChanged(boolean showBackground)
  {
    for (final OnShowBackgroundChange listener : mOnShowBackgroundChanges)
    {
      listener.onOnShowBackgroundStatusChange(showBackground);
    }
  }

  private void notifyDictionaryListContentChanged()
  {
    for (final OnDictionaryListChange listener : mOnDictionaryListChanges)
    {
      listener.onDictionaryListContentChange();
    }
  }

  private void notifyDictionaryListStatusChanged(int status)
  {
    for (final OnDictionaryListChange listener : mOnDictionaryListChanges)
    {
      listener.onDictionaryListStatusChange(status);
    }
  }

  private void notifyDictionaryListSelectionChanged(int element)
  {
    for (final OnDictionaryListChange listener : mOnDictionaryListChanges)
    {
      listener.onDictionaryListSelectionChange(element);
    }
  }

  private void notifyDirectionListContentChanged(Collection<DirectionView> content)
  {
    for (final OnDirectionListChange listener : mOnDirectionListChanges)
    {
      listener.onDirectionListContentChange(content);
    }
  }

  private void notifyDirectionListStatusChanged(int status)
  {
    for (final OnDirectionListChange listener : mOnDirectionListChanges)
    {
      listener.onDirectionListStatusChange(status);
    }
  }

  private void notifyDirectionChanged(int element)
  {
    for (final OnDirectionListChange listener : mOnDirectionListChanges)
    {
      listener.onDirectionChange(element);
    }
  }

  private void notifyTitleStatusChanged(int status)
  {
    for (final OnTitleChange listener : mOnTitleChanges)
    {
      listener.onTitleStatusChange(status);
    }
  }

  private void notifyTitleTextChanged(String text)
  {
    for (final OnTitleChange listener : mOnTitleChanges)
    {
      listener.onTitleTextChange(text);
    }
  }

  private void notifySelectionModeStatusChanged(int status)
  {
    for (final OnSelectionModeChange listener : mOnSelectionModeChanges)
    {
      listener.onSelectionModeStatusChange(status);
    }
  }

  private void notifyDeleteSelectedStatusChanged(int status)
  {
    for (final OnDeleteSelectedChange listener : mOnDeleteSelectedChanges)
    {
      listener.onDeleteSelectedActionVisibilityStatusChange(status);
    }
  }

  private void notifyNumberOfSelectedItemsChanged(int mNumberOfSelectedItems)
  {
    for (final OnSelectionModeChange listener : mOnSelectionModeChanges)
    {
      listener.onNumberOfSelectedItemsChange(mNumberOfSelectedItems);
    }
  }

  private int getElementNumber(@Nullable DictionaryId dictionaryId)
  {
    if (null == dictionaryId)
    {
      return 0;
    }
    int pos = 0;
    for (final DictionaryView view : mDictionaryViews)
    {
      if (dictionaryId.equals(view.getId()))
      {
        return pos;
      }
      ++pos;
    }
    return 0;
  }

  @Override
  public void restoreTitle(String title) {
    mTitle = title;
  }

  public void setTitle(@NonNull String title)
  {
    mTitle = title;
    notifyTitleTextChanged(title);
  }

  public void setNumberOfSelectedItems(int numberOfSelectedItems)
  {
    mNumberOfSelectedItems = numberOfSelectedItems;
    notifyNumberOfSelectedItemsChanged(mNumberOfSelectedItems);
  }
}
