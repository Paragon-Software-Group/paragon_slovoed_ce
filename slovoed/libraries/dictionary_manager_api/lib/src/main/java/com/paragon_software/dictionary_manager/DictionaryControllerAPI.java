package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.purchase.RestorePurchasesResult;

import java.util.Date;

import io.reactivex.Observable;

public interface DictionaryControllerAPI
{
  @NonNull
  Dictionary[] getDictionaries();

  @Nullable
  Dictionary getDictionaryById( @NonNull Dictionary.DictionaryId dictionaryId );

  void registerObserver( @NonNull DictionaryManagerAPI.IDictionaryListObserver observer );

  void unregisterObserver( @NonNull DictionaryManagerAPI.IDictionaryListObserver observer );

  void installFilter( @NonNull DictionaryFilter filter );

  void uninstallFilter( @NonNull DictionaryFilter filter );

  void download( DictionaryComponent component, Dictionary dictionary );

  boolean isDownloaded( DictionaryComponent component );

  boolean isInProgress( DictionaryComponent component );

  int getTrialLengthInMinutes( @NonNull Dictionary.DictionaryId dictionaryId );

  long getTransferredBytes( DictionaryComponent component );

  long getSizeBytes( DictionaryComponent component );

  void unregisterComponentProgressObserver( @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer );

  void registerComponentProgressObserver( @NonNull DictionaryComponent component,
                                          @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer );

  void registerDownloadFailureObserver( DictionaryManagerAPI.IOnDownloadFailureObserver observer );

  void unregisterDownloadFailureObserver( DictionaryManagerAPI.IOnDownloadFailureObserver observer );

  Dictionary.DictionaryId getDictionaryIdSelectedInMyDictionaries();

  void setDictionaryIdSelectedInMyDictionaries( Dictionary.DictionaryId dictionaryIdSelectedInMyDictionaries );

  void remove( @NonNull DictionaryComponent component );

  void pauseDownload( DictionaryComponent component, Dictionary dictionary );

  boolean isAnyDictionaryDownloaded();

  void deleteAllDictionaries();

  void restorePurchases( @NonNull FragmentActivity activity );
  void restorePurchases( @NonNull Fragment fragment );
  @NonNull
  Observable< Boolean > getRestorePurchasesInProgressObservable();
  @NonNull
  Observable< RestorePurchasesResult > getRestorePurchasesResultObservable();
  @NonNull
  Observable< Boolean > getTrialUpdateObservable();

  void setDescriptionPageDictionaryId( @NonNull Dictionary.DictionaryId dictionaryId );
  @NonNull
  Observable< Dictionary.DictionaryId > getDescriptionDictionaryIdObservable();
  
  Date getUserCoreEndTime(Dictionary.DictionaryId dictionaryId);
}
