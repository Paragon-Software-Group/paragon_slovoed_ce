package com.paragon_software.toolbar_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Collection;

public interface ToolbarController
{

  @NonNull
  String getTitle();

  void restoreTitle(String title);

  boolean isShowTitle();

  int getNumberOfSelectedItems();

  @NonNull
  Collection<DictionaryView> getDictionaryList();

  boolean isShowDictionaryList();

  boolean isShowDirectionList();

  boolean isShowSelectionMode();

  boolean isShowDeleteSelectedAction();

  boolean isShowBackground();



  void selectDictionaryItem(@NonNull DictionaryView view);

  @NonNull
  DictionaryView getSelectedDictionaryItem();

  void selectDirectionView(@Nullable DirectionView view);

  DirectionView getSelectedDirectionView();

  void changeDirection();

  void deleteSelectedActionClick();

  void selectAllActionClick();

  void backActionClick();

  void registerNotifier(@NonNull Notifier notifier);
  void unRegisterNotifier(@NonNull Notifier notifier);

  boolean isShowHomeAsUp();

  interface Notifier{

  }
}
