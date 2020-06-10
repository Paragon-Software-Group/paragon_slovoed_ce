package com.paragon_software.dictionary_manager;

import android.content.Context;
import android.os.Handler;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.errors.ErrorType;
import com.paragon_software.dictionary_manager.errors.OperationType;
import com.paragon_software.dictionary_manager.filter.DictionaryComparator;
import com.paragon_software.dictionary_manager.purchase.RestorePurchasesResult;
import com.paragon_software.dictionary_manager.purchase.RestorePurchasesResultType;
import com.paragon_software.utils_slovoed.network.NetworkUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.CompositeDisposable;
import io.reactivex.functions.Consumer;
import io.reactivex.subjects.BehaviorSubject;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.IComponentDownloadProgressObserver;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.IDictionaryListObserver;
import static com.paragon_software.dictionary_manager.DictionaryManagerAPI.IOnDownloadFailureObserver;

class DictionaryController implements DictionaryControllerAPI
{
  private final IDownloadController         downloadController;
  private final DictionaryManagerAPI       dictionaryManager;
  private       Dictionary.DictionaryId    dictionaryIdSelectedInMyDictionaries;
  private LinkedList< DictionaryFilter >        filters                 = new LinkedList<>();
  private LinkedList< IDictionaryListObserver > dictionaryListObservers = new LinkedList<>();
  private DictionaryFilter.FilterStateChangedListener allFilterStateChangeListener =
          this::notifyDictionaryListObservers;

  private static final int                        RESTORE_PURCHASES_TIMEOUT_MILLIS  = 10000;
  private final Handler                           restorePurchasesHandler           = new Handler();
  private final BehaviorSubject< Boolean >        restorePurchasesInProgressSubject = BehaviorSubject.createDefault(false);
  private final Subject< RestorePurchasesResult > restorePurchasesResultSubject     = PublishSubject.create();

  private final BehaviorSubject< Dictionary.DictionaryId > dictionaryIdForDescriptionSubject
          = BehaviorSubject.createDefault( new Dictionary.DictionaryId("TemporaryFirstElement") );

  @NonNull
  private Map< Dictionary.DictionaryId, SubscriptionPurchase > mPrevSubscriptionPurchases = new HashMap<>();
  private CompositeDisposable mRestorePurchaseCompositeDisposable;
  private final DictionaryManagerAPI.IErrorObserver restorePurchasesErrorObserver = new RestorePurchasesDictManagerErrorObserver();

  @NonNull
  private final DictionaryLocationResolver mDictionaryLocationResolver;


  @SuppressWarnings( "WeakerAccess" )
  public DictionaryController( @NonNull DictionaryManagerAPI dictionaryManager, @NonNull DictionaryLocationResolver locationResolver )
  {
    this.dictionaryManager = dictionaryManager;
    this.downloadController = new HttpDownloaderLibController(locationResolver, dictionaryManager);
    mDictionaryLocationResolver = locationResolver;
  }

  @NonNull
  @Override
  public Dictionary[] getDictionaries()
  {
    Dictionary[] filtered = applyFilters(dictionaryManager.getDictionaries(), filters);
    return applySort(filtered, filters);
  }

  @Override
  public Dictionary getDictionaryById(@NonNull Dictionary.DictionaryId dictionaryId) {
    return dictionaryManager.getDictionaryById(dictionaryId);
  }

  private Dictionary[] applyFilters(@NonNull Collection< Dictionary > allDictionaries,
                                    @NonNull LinkedList< DictionaryFilter > filters )
  {
    ArrayList< Dictionary > dictionaries = new ArrayList<>(allDictionaries.size());
    for ( Dictionary d : allDictionaries )
    {
      boolean applyForFilters = true;
      for ( DictionaryFilter filter : filters )
      {
        if ( !filter.apply(d.getId()) )
        {
          applyForFilters = false;
          break;
        }
      }
      if ( applyForFilters )
      {
        dictionaries.add(d);
      }
    }
    return dictionaries.toArray(new Dictionary[dictionaries.size()]);
  }

  private Dictionary[] applySort( @NonNull Dictionary[] filtered, @NonNull LinkedList< DictionaryFilter > filters )
  {
    Dictionary[] sortInProgress = Arrays.copyOf(filtered, filtered.length);
    for ( DictionaryFilter filter : filters )
    {
      Map< Dictionary.DictionaryId, Integer > priority = DictionaryComparator.getPriority(sortInProgress, filter);
      if ( !priority.isEmpty() )
      {
        Arrays.sort(sortInProgress, new DictionaryComparator(priority));
      }
    }

    return sortInProgress;
  }

  @Override
  public void registerObserver( @NonNull IDictionaryListObserver iDictionaryListObserver )
  {
    dictionaryListObservers.add(iDictionaryListObserver);
  }

  @Override
  public void unregisterObserver( @NonNull IDictionaryListObserver iDictionaryListObserver )
  {
    dictionaryListObservers.remove(iDictionaryListObserver);
  }

  @Override
  public void installFilter( @NonNull DictionaryFilter dictionaryFilter )
  {
    filters.add(dictionaryFilter);
    dictionaryFilter.registerListener(allFilterStateChangeListener);
    notifyDictionaryListObservers();
  }

  private void notifyDictionaryListObservers()
  {
    for ( IDictionaryListObserver observer : new ArrayList<>(dictionaryListObservers) )
    {
      observer.onDictionaryListChanged();
    }
  }

  @Override
  public void uninstallFilter( @NonNull DictionaryFilter dictionaryFilter )
  {
    if ( filters.remove(dictionaryFilter) )
    {
      dictionaryFilter.unregisterListener(allFilterStateChangeListener);
      notifyDictionaryListObservers();
    }
  }

  @Override
  public void download( final DictionaryComponent component, Dictionary dictionary )
  {
    downloadController.download(component, dictionary);
  }

  @Override
  public boolean isDownloaded( DictionaryComponent component )
  {
    return mDictionaryLocationResolver.isInAssets(component) || downloadController.isDownloaded(component);
  }

  @Override
  public boolean isInProgress( DictionaryComponent component )
  {
    return downloadController.isInProgress(component);
  }

  @Override
  public int getTrialLengthInMinutes( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    return dictionaryManager.getTrialLengthInMinutes(dictionaryId);
  }

  @Override
  public long getTransferredBytes( DictionaryComponent component )
  {
    return downloadController.getTransferredBytes(component);
  }

  @Override
  public long getSizeBytes( DictionaryComponent component )
  {
    return downloadController.getSizeBytes(component);
  }

  @Override
  public void unregisterComponentProgressObserver( @NonNull IComponentDownloadProgressObserver observer )
  {
    downloadController.unregisterComponentProgressObserver(observer);
  }

  @Override
  public void registerComponentProgressObserver( @NonNull DictionaryComponent component,
                                                 @NonNull IComponentDownloadProgressObserver observer )
  {
    downloadController.registerComponentProgressObserver(component, observer);
  }

  @Override
  public void registerDownloadFailureObserver( IOnDownloadFailureObserver observer )
  {
    downloadController.registerDownloadFailureObserver(observer);
  }

  @Override
  public void unregisterDownloadFailureObserver( IOnDownloadFailureObserver observer )
  {
    downloadController.unregisterDownloadFailureObserver(observer);
  }

  @Override
  public Dictionary.DictionaryId getDictionaryIdSelectedInMyDictionaries()
  {
    return dictionaryIdSelectedInMyDictionaries;
  }

  @Override
  public void setDictionaryIdSelectedInMyDictionaries( Dictionary.DictionaryId dictionaryId )
  {
    this.dictionaryIdSelectedInMyDictionaries = dictionaryId;
  }

  @Override
  public void remove( @NonNull DictionaryComponent component )
  {
    downloadController.remove(component);
  }

  @Override
  public void pauseDownload( DictionaryComponent component, Dictionary dictionary )
  {
    downloadController.pauseDownload(component, dictionary);
  }

  @Override
  public boolean isAnyDictionaryDownloaded() {
    return downloadController.isAnyDictionaryDownloaded();
  }

  @Override
  public void deleteAllDictionaries()
  {
    downloadController.removeAll();
  }

  @Override
  public void restorePurchases( @NonNull FragmentActivity activity )
  {
    if ( canRestorePurchases(activity) )
    {
      startRestorePurchases();
      dictionaryManager.loadOnlineDictionaryStatusInformation(activity);
    }
  }

  @Override
  public void restorePurchases( @NonNull Fragment fragment )
  {
    if ( canRestorePurchases(fragment.getContext()) )
    {
      startRestorePurchases();
      dictionaryManager.loadOnlineDictionaryStatusInformation(fragment);
    }
  }

  private void startRestorePurchases()
  {
    restorePurchasesHandler.removeCallbacksAndMessages(null);
    restorePurchasesInProgressSubject.onNext(true);

    mPrevSubscriptionPurchases = dictionaryManager.getSubscriptionPurchases();
    this.dictionaryManager.registerErrorObserver(restorePurchasesErrorObserver);
    mRestorePurchaseCompositeDisposable = new CompositeDisposable();
    mRestorePurchaseCompositeDisposable.add(this.dictionaryManager.getUpdateFinishedAfterLicenseChangeObservable()
                                                   .observeOn(AndroidSchedulers.mainThread())
                                                   .subscribe(new RestorePurchasesDictManagerUpdateFinishedObserver()));

    restorePurchasesHandler.postDelayed(() -> {
      stopRestorePurchases();
      restorePurchasesResultSubject.onNext(new RestorePurchasesResult.Builder(
          RestorePurchasesResultType.ERROR).setErrorType(ErrorType.TIMEOUT).build());
    }, RESTORE_PURCHASES_TIMEOUT_MILLIS);
  }

  private void stopRestorePurchases()
  {
    this.dictionaryManager.registerErrorObserver(restorePurchasesErrorObserver);
    mRestorePurchaseCompositeDisposable.clear();

    restorePurchasesHandler.removeCallbacksAndMessages(null);
    restorePurchasesInProgressSubject.onNext(false);
  }

  private boolean canRestorePurchases( Context context )
  {
    if ( restorePurchasesInProgressSubject.getValue() )
      return false;
    if ( !NetworkUtils.isNetworkAvailable(context) )
    {
      restorePurchasesResultSubject.onNext(new RestorePurchasesResult.Builder(
          RestorePurchasesResultType.ERROR).setErrorType(ErrorType.NO_INTERNET).build());
      return false;
    }
    return true;
  }

  @NonNull
  @Override
  public Observable< Boolean > getRestorePurchasesInProgressObservable()
  {
    return restorePurchasesInProgressSubject;
  }

  @NonNull
  @Override
  public Observable< RestorePurchasesResult > getRestorePurchasesResultObservable()
  {
    return restorePurchasesResultSubject;
  }

  @NonNull
  @Override
  public Observable<Boolean> getTrialUpdateObservable()
  {
    return dictionaryManager.getTrialUpdateObservable();
  }

  @Override
  public void setDescriptionPageDictionaryId( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    this.dictionaryIdForDescriptionSubject.onNext( dictionaryId );
  }

  @NonNull
  @Override
  public Observable<Dictionary.DictionaryId> getDescriptionDictionaryIdObservable()
  {
    return dictionaryIdForDescriptionSubject;
  }
  
  @Override
  public Date getUserCoreEndTime(Dictionary.DictionaryId dictionaryId) {
    return dictionaryManager.getUserCoreEndTime(dictionaryId);
  }

  private class RestorePurchasesDictManagerErrorObserver implements DictionaryManagerAPI.IErrorObserver
  {
    @Override
    public void onError( @NonNull OperationType operationType, @NonNull List< ErrorType > error )
    {
      if ( restorePurchasesInProgressSubject.getValue() && !error.isEmpty() )
      {
        stopRestorePurchases();
        restorePurchasesResultSubject.onNext(new RestorePurchasesResult.Builder(
            RestorePurchasesResultType.ERROR).setErrorType(error.get(0)).build());
      }
    }
  }

  private class RestorePurchasesDictManagerUpdateFinishedObserver implements Consumer< Boolean >
  {

    @Override
    public void accept( Boolean state ) throws Exception
    {
      if ( state && restorePurchasesInProgressSubject.getValue() )
      {
        stopRestorePurchases();

        Map< Dictionary.DictionaryId, SubscriptionPurchase > subscriptionPurchases = dictionaryManager.getSubscriptionPurchases();
        if ( subscriptionPurchases.isEmpty() )
        {
          restorePurchasesResultSubject.onNext(new RestorePurchasesResult.Builder(
              RestorePurchasesResultType.NO_PURCHASES_FOUND).build());
        }
        else if ( mPrevSubscriptionPurchases.equals(subscriptionPurchases) )
        {
          restorePurchasesResultSubject.onNext(new RestorePurchasesResult.Builder(
              RestorePurchasesResultType.NO_NEW_PURCHASES).build());
        }
        else
        {
          restorePurchasesResultSubject.onNext(new RestorePurchasesResult.Builder(
              RestorePurchasesResultType.RESTORE_PURCHASES_SUCCESSFULLY).build());
        }
      }
    }
  }
}
