package com.paragon_software.toolbar_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.Dictionary.DictionaryId;

public abstract class ToolbarManager
{

  public abstract void showDictionaryList();

  public abstract void showDictionaryList(@Nullable DictionaryId  selected, @Nullable Dictionary.Direction direction);

  public abstract void updateDictionaryList(@Nullable DictionaryId  selected, @Nullable Dictionary.Direction direction);

  public abstract void showDictionaryList( boolean showAllDictionaries );

  public abstract void showDictionaryListNoDirection();

  public abstract void showTitleAndDirectionInToolbar(@NonNull String title);

  public abstract void showSelectionMode( int numberOfSelectedItems );

  public abstract void showTitle(boolean showBackground);

  public abstract void showTitle(@NonNull String title);

  public abstract void showTitle( @NonNull String title, boolean showBackground );

  public abstract void setSelectedDirection(Dictionary.Direction selectedDirection);

  public abstract void registerNotifier(@NonNull Notifier notifier);

  public abstract void unRegisterNotifier(@NonNull Notifier notifier);

  public abstract DictionaryId getCurrentDictionary();

  public abstract int getCurrentDirection();

  public abstract void showHomeAsUp(boolean visibility);

  @NonNull
  abstract ToolbarController getController( @NonNull String uiName );

  abstract void freeController( @NonNull String uiName );

  abstract void deleteSelectedActionClick();

  abstract void selectAllActionClick();

  abstract void backActionClick();

  public interface Notifier {

  }
}
