package com.paragon_software.toolbar_manager;

import android.graphics.Bitmap;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.toolbar_manager.Notifiers.OnBackActionClick;
import com.paragon_software.toolbar_manager.Notifiers.OnDeleteSelectedActionClick;
import com.paragon_software.toolbar_manager.Notifiers.OnDictionarySelect;
import com.paragon_software.toolbar_manager.Notifiers.OnSelectAllActionClick;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.atomic.AtomicReference;

public class ParagonToolbarManager extends ToolbarManager implements DictionaryManagerAPI.IDictionaryListObserver
{
  private final DictionaryManagerAPI                              mDictionaryManager;

  private final HashMap<String,ParagonToolbarController>          mControllers                         = new HashMap<>();
  private final ArrayList< OnDictionarySelect >                   mDictionarySelects                   = new ArrayList<>();
  private final ArrayList< OnDeleteSelectedActionClick >          mDeleteSelectedActionClicks          = new ArrayList<>();
  private final ArrayList< OnSelectAllActionClick >               mSelectAllActionClicks               = new ArrayList<>();
  private final ArrayList< OnBackActionClick >                    mBackActionClicks                    = new ArrayList<>();
  private final AtomicReference<LocalizedString>                           mDefaultDictionaryTitle = new AtomicReference<>();
  private final AtomicReference<Bitmap>                           mDefaultDictionaryIcon = new AtomicReference<>();

  private final AtomicReference <ParagonToolbarController> mActiveController = new AtomicReference<>();

  public ParagonToolbarManager( @NonNull DictionaryManagerAPI manager )
  {
    mDictionaryManager = manager;
    mDictionaryManager.registerDictionaryListObserver(this);
  }

  @Override
  @NonNull
  public ToolbarController getController( @NonNull String s)
  {
    ParagonToolbarController controller = mControllers.get(s);
    if ( null == controller)
    {
      if (ToolbarControllerType.DEFAULT_CONTROLLER.equals(s))
      {
        controller = new ParagonToolbarController(this,
                mDictionaryManager.getDictionaries(),
                null,
                mDefaultDictionaryTitle.get(),
                mDefaultDictionaryIcon.get());
        mControllers.put(ToolbarControllerType.DEFAULT_CONTROLLER, controller);
      }
    }
    mActiveController.set(controller);
    updateActiveController();
    return controller;
  }

  @Override
  public void freeController( @NonNull String uiName )
  {
    if (mActiveController.get() == mControllers.get(uiName))
    {
      mActiveController.set(null);
    }
  }

  @Override
  public void deleteSelectedActionClick()
  {
    for (final OnDeleteSelectedActionClick listener : mDeleteSelectedActionClicks)
    {
      listener.onDeleteSelectedActionClick();
    }
  }

  @Override
  public void selectAllActionClick()
  {
    for (final OnSelectAllActionClick listener : mSelectAllActionClicks)
    {
      listener.onSelectAllActionClick();
    }
  }

  @Override
  public void backActionClick()
  {
    for (final OnBackActionClick listener : mBackActionClicks)
    {
      listener.onBackActionClick();
    }
  }

  @Override
  public void showDictionaryList()
  {
    final ParagonToolbarController controller = mActiveController.get();
    if (null == controller) return;
    controller.showDictionary();
  }

  @Override
  public void showDictionaryList( @Nullable Dictionary.DictionaryId dictionaryId, @Nullable Dictionary.Direction direction )
  {
    final ParagonToolbarController controller = mActiveController.get();
    if (null == controller) return;
    controller.selectDictionary(dictionaryId, direction);
    controller.showDictionary();
  }

  @Override
  public void updateDictionaryList(@Nullable Dictionary.DictionaryId dictionaryId, @Nullable Dictionary.Direction direction) {
    final ParagonToolbarController controller = mActiveController.get();
    if (null == controller) return;
    controller.selectDictionary(dictionaryId, direction);
  }

  @Override
  public void showDictionaryList(boolean fts_mode)
  {
    final ParagonToolbarController controller = mActiveController.get();
    if (null == controller) return;
    controller.showDictionary(fts_mode);
  }

  @Override
  public void showDictionaryListNoDirection()
  {
    final ParagonToolbarController controller = mActiveController.get();
    if (null == controller) return;
    controller.showDictionaryNoDirection();
  }

  @Override
  public void showTitleAndDirectionInToolbar(@NonNull String title)
  {
    final ParagonToolbarController controller = mActiveController.get();
    if (null == controller)
      return;
    controller.setTitle(title);
    controller.showTitleAndDirectionInToolbar();
  }

  @Override
  public void showSelectionMode( int numberOfSelectedItems )
  {
    final ParagonToolbarController controller = mActiveController.get();
    if (controller != null) {
      controller.showSelectionMode(numberOfSelectedItems);
    }
  }

  @Override
  public void showTitle(boolean showBackground) {
    final ParagonToolbarController controller = mActiveController.get();
    if (null == controller) return;
    controller.showTitle(showBackground);
  }

  @Override
  public void showTitle( @NonNull String title )
  {
    showTitle(title, true);
  }

  @Override
  public void showTitle( @NonNull String title, boolean showBackground )
  {
    final ParagonToolbarController controller = mActiveController.get();
    if (null == controller) return;
    controller.setTitle(title);
    controller.showTitle(showBackground);
  }

  @Override
  public void setSelectedDirection(Dictionary.Direction selectedDirection)
  {
    final ParagonToolbarController controller = mActiveController.get();
    if (controller != null) {
      controller.selectDirection(selectedDirection);
    }
  }

  public void changeSelectedDictionary( @Nullable Dictionary.DictionaryId dictionaryId )
  {
    for (final OnDictionarySelect listener : mDictionarySelects)
    {
      listener.onDictionarySelect(dictionaryId);
    }
    saveDictionaryAndDirection();
  }

  void changeSelectedDirection(int directionId)
  {
    for (final OnDictionarySelect listener : mDictionarySelects)
    {
      listener.onDirectionSelect(directionId);
    }
    saveDictionaryAndDirection();
  }

  public void setDefaultDictionaryTitle(@NonNull LocalizedString title){
    mDefaultDictionaryTitle.set(title);
  }

  public void setDefaultDictionaryIcon(@Nullable Bitmap bitmap)
  {
    mDefaultDictionaryIcon.set(bitmap);
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnDictionarySelect && !mDictionarySelects.contains(notifier))
    {
      mDictionarySelects.add((OnDictionarySelect) notifier);
    }
    if (notifier instanceof OnDeleteSelectedActionClick && !mDeleteSelectedActionClicks.contains(notifier))
    {
      mDeleteSelectedActionClicks.add((OnDeleteSelectedActionClick) notifier);
    }
    if (notifier instanceof OnSelectAllActionClick && !mSelectAllActionClicks.contains(notifier))
    {
      mSelectAllActionClicks.add((OnSelectAllActionClick) notifier);
    }
    if (notifier instanceof OnBackActionClick && !mBackActionClicks.contains(notifier))
    {
      mBackActionClicks.add((OnBackActionClick) notifier);
    }
  }

  @Override
  public void unRegisterNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnDictionarySelect)
    {
      mDictionarySelects.remove(notifier);
    }
    if (notifier instanceof OnDeleteSelectedActionClick)
    {
      mDeleteSelectedActionClicks.remove(notifier);
    }
    if (notifier instanceof OnSelectAllActionClick)
    {
      mSelectAllActionClicks.remove(notifier);
    }
    if (notifier instanceof OnBackActionClick)
    {
      mBackActionClicks.remove(notifier);
    }
  }

  @Override
  public Dictionary.DictionaryId getCurrentDictionary()
  {
    if (null != mActiveController.get())
    {
      return mActiveController.get().getSelectedDictionaryItem().getId();
    }
    else
    {
      return null;
    }
  }

  @Override
  public int getCurrentDirection()
  {
    if (null != mActiveController.get())
    {
      final DirectionView directionView = mActiveController.get().getSelectedDirectionView();
      if (null != directionView)
      {
        return mActiveController.get().getSelectedDirectionView().getId();
      }
      else
      {
        return DirectionView.INVALID_DIRECTION;
      }
    }
    else
    {
      return DirectionView.INVALID_DIRECTION;
    }
  }

  private void restoreSelected()
  {
    if (mActiveController.get() != null && !mDictionaryManager.isSelectAllDictionaries()&& mDictionaryManager.getDictionaryAndDirectionSelectedByUser()!=null) {
        final Dictionary.Direction direction = mDictionaryManager.getDictionaryAndDirectionSelectedByUser().getDirection();
        selectDictionary(mDictionaryManager.getDictionaryAndDirectionSelectedByUser().getDictionaryId());
        setSelectedDirection(direction);
        saveDictionaryAndDirection();
      }
  }

  private void saveDictionaryAndDirection()
  {
    if(mActiveController.get()!=null && getCurrentDictionary()!=null && mActiveController.get().getSelectedDirectionView()!=null) {
      mDictionaryManager.setDictionaryAndDirectionSelectedByUser(new DictionaryAndDirection(getCurrentDictionary(),
              new Dictionary.Direction(mActiveController.get().getSelectedDirectionView().getId(), mActiveController.get().getSelectedDirectionView().getDirectonTo(), null)));
      mDictionaryManager.setIsSelectAllDictionaries(false);
    }
    else
	{
		mDictionaryManager.setIsSelectAllDictionaries(true);
	}
  }

  private void selectDictionary(Dictionary.DictionaryId dictionaryId) {
    if (mActiveController.get() != null) {
      mActiveController.get().selectDictionary(dictionaryId, null);
    }
  }

  @Override
  public void showHomeAsUp(boolean visibility) {
    if (mActiveController.get() != null)
    {
      mActiveController.get().showHomeAsUp(visibility);
    }
  }

  @Override
  public void onDictionaryListChanged()
  {
    updateActiveController();
  }

  private void updateActiveController()
  {
    if (mActiveController.get() != null)
    {
      mActiveController.get().setDictionaryList(mDictionaryManager.getDictionaries());
      if ( mDictionaryManager.getDictionaries().size() != 0 )
        restoreSelected();
    }
  }
}
