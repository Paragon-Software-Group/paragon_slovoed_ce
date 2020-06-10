package com.paragon_software.history_manager;

import android.app.Activity;
import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.List;

public abstract class HistoryControllerAPI
{
  public abstract void registerNotifier( @NonNull Notifier notifier );

  public abstract void unregisterNotifier( @NonNull Notifier notifier );

  public abstract boolean isInSelectionMode();

  public abstract void openArticle( int articleIndex, Context context );

  @NonNull
  public abstract List< ArticleItem > getWords();

  @NonNull
  public abstract List< ArticleItem > getSelectedWords();

  public abstract float getEntryListFontSize();

  public abstract boolean isInTransition();

  public abstract void deleteAllWords();

  public abstract void deleteSelectedWords();

  public abstract void selectItem( int articleIndex );

  public abstract void unselectItem( int articleIndex );

  public abstract boolean hasSound( int articleIndex );

  public abstract void playSound( int articleIndex );

  public abstract void selectionModeOff();

  public abstract int getDeleteAllActionVisibilityStatus();

  public abstract int getExportActionVisibilityStatus();

  public abstract boolean isDeleteAllActionEnable();

  public abstract boolean isExportActionEnable();

  public abstract void exportHistory( @NonNull Context context, @NonNull ExportParams params);

  public abstract void exportHistoryAsString( @NonNull Context context, ExportParams params );

  public abstract void exportHistoryAsHtmlString(@NonNull Activity context, ExportParams params);

  abstract void activate();

  abstract void deactivate();

  @Nullable
  abstract IDictionaryIcon getDictionaryIcon(@NonNull Dictionary.DictionaryId dictionaryId);

  abstract boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );

  @Nullable
  abstract LocalizedString getCurrentDictionaryTitle();

  interface Notifier
  {
  }
}
