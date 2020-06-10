/*
 * dictionary manager api
 *
 *  Created on: 19.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import com.paragon_software.dictionary_manager.downloader.DownloadLibraryBuilder;
import com.paragon_software.dictionary_manager.errors.ErrorType;
import com.paragon_software.dictionary_manager.errors.OperationType;
import com.paragon_software.dictionary_manager.exceptions.DictionaryManagerException;
import com.paragon_software.dictionary_manager.filter.IFilterFactory;

import java.util.Date;
import java.util.List;
import java.util.Map;

import androidx.fragment.app.FragmentManager;
import io.reactivex.Observable;

@SuppressWarnings( "unused" )
public abstract class DictionaryManagerAPI
{

  public static final String UI_EXTRA_FRAGMENT_VALUE_CATALOG = DictionaryManagerAPI.class.getName() + ".FRAGMENT_CATALOG";
  public static final String UI_EXTRA_FRAGMENT_VALUE_SEARCH = DictionaryManagerAPI.class.getName() + ".FRAGMENT_SEARCH";
  public static final String UI_EXTRA_FRAGMENT_VALUE_DOWNLOAD = DictionaryManagerAPI.class.getName() + ".FRAGMENT_DOWNLOAD";
  public static final String UI_EXTRA_FRAGMENT_KEY = DictionaryManagerAPI.class.getName() + ".FRAGMENT_KEY";

  public static final String UI_PENDING_INTENT_KEY_BASE_FROM_NOTIFICATION = DictionaryManagerAPI.class.getName() + ".KEY_BASE_FROM_NOTIFICATION";
  public static final String UI_PENDING_INTENT_KEY_BASE_INFO_STATUS = DictionaryManagerAPI.class.getName() + ".KEY_BASE_INFO_STATUS";
  public static final String UI_EXTRA_SERIALIZABLE_BUNDLE = DictionaryManagerAPI.class.getName() + ".SERIALIZABLE_BUNDLE";
  /**
   * Key string using to pass dictionary id value to UI as {@link Intent} extra parameter.
   */
  public static final String UI_EXTRA_DICTIONARY_ID_KEY = DictionaryManagerAPI.class.getName() + ".DICTIONARY_ID_KEY";
  public static final String UI_EXTRA_DIRECTION_KEY = DictionaryManagerAPI.class.getName() + ".DIRECTION_KEY";
  public static final String UI_EXTRA_SEARCH_STRING_KEY = DictionaryManagerAPI.class.getName() + ".SEARCH_STRING_KEY";
  public static final String DOWNLOAD_CONTROLLER_TAG = "DOWNLOAD_CONTROLLER";
  /**
   * Key string using to pass article item object to UI as {@link Intent} extra parameter.
   */
  public static final String UI_EXTRA_ARTICLE_ITEM_KEY = DictionaryManagerAPI.class.getName() + ".ARTICLE_ITEM_KEY";

  /**
   * Register UI elements to launch from the dictionary manager.
   * @param buyActivity class of activity for buying dictionaries
   * @param mainActivity class of activity with search screen
   */
  public abstract void registerDictionaryManagerUI( @Nullable Class buyActivity, @Nullable Class mainActivity );

  /**
   * @return dictionary collection. Can be empty
   */
  @NonNull
  public abstract List< Dictionary > getDictionaries();

  @NonNull
  public abstract List<DictionaryPack> getDictionaryPacks();

  /**
   * @return dictionary found by ID number. Can be null
   */
  @Nullable
  public abstract Dictionary getDictionaryById( @Nullable Dictionary.DictionaryId dictionaryId );

  /**
   * Register new observer to update list information. If observer already registered: do nothing
   *
   * @param observer new observer
   */
  public abstract void registerDictionaryListObserver( @NonNull IDictionaryListObserver observer );

  /**
   * Unregister observer. If observer was not registered: do nothing
   *
   * @param observer object to unregister
   */
  public abstract void unRegisterDictionaryListObserver( @NonNull IDictionaryListObserver observer );

  /**
   * get price for dictionary
   *
   * @param dictionaryId id of dictionary
   *
   * @return dictionary price, or null, if price unavailable (e.g dictionary unavailable, or dictionary is trial)
   */
  @Nullable
  public abstract DictionaryPrice getDictionaryPrice( @NonNull Dictionary.DictionaryId dictionaryId );

  public abstract Observable< Boolean > getUpdateStateObservable();

  public abstract Observable< Boolean > getTrialUpdateObservable();

  public abstract Observable< Boolean > getUpdateFinishedAfterLicenseChangeObservable();

  @Nullable
  public abstract List<DictionaryPrice> getSubscriptionPrices( @NonNull Dictionary.DictionaryId dictionaryId );

  public abstract ErrorType activationEnd( final int requestCode, int resultCode, @NonNull final Intent intent );

  /**
   * @return {@link ErrorType#OK} - on success. </br> Possible errors: {@link ErrorType#PURCHASE_ITEM_UNAVAILABLE}, {@link ErrorType#PURCHASE_ITEM_ALREADY_OWNED}, {@link ErrorType#ACCOUNT_MANAGEMENT_EXCEPTION}, {@link ErrorType#UNDEFINED}
   */
  public abstract ErrorType buyCatalogItem(@NonNull Activity activity, @NonNull final Dictionary.DictionaryId dictionaryId,
                                           @Nullable DictionaryPrice.PeriodSubscription subscriptionPeriod );

  public abstract void buy( @NonNull Activity activity, @NonNull final Dictionary.DictionaryId dictionaryId );

  public abstract void openDictionaryForSearch( @NonNull Context context,
                                                @NonNull final Dictionary.DictionaryId dictionaryId,
                                                @Nullable Dictionary.Direction direction,
                                                @Nullable String text);

  public abstract void openDictionaryForPreview( @NonNull Context context,
                                                @NonNull final Dictionary.DictionaryId dictionaryId);

  public abstract void openDictionaryDescription();

  public abstract void openMyDictionariesUI( @NonNull Context context, @NonNull final Dictionary.DictionaryId dictionaryId );

  public abstract void openDownloadManagerUI(@NonNull Context context, @Nullable Dictionary.DictionaryId dictionaryId);

  public abstract void openCatalogueAndRestorePurchase(@NonNull Context context);

  public abstract ErrorType consume(@NonNull Context appContext, @NonNull Dictionary dictionary );

  @Nullable
  public abstract DictionaryControllerAPI createController( String name );

  @NonNull
  public abstract IFilterFactory getFilterFactory();

  abstract void freeController( @NonNull DictionaryControllerAPI controller );

  @Nullable
  public abstract IDictionaryDiscount getDictionaryDiscount( @NonNull Dictionary.DictionaryId dictionaryId );

  public abstract void updateLicensesStatus();

  public abstract void updateDictionariesInBackground();

  public abstract void registerErrorObserver( @NonNull IErrorObserver errorObserver );

  public abstract void unRegisterErrorObserver( @NonNull IErrorObserver errorObserver );

  public abstract void loadOnlineDictionaryStatusInformation(@NonNull FragmentActivity activity);

  public abstract void loadOnlineDictionaryStatusInformation(@NonNull Fragment fragment);

  public abstract void updateOnlineTrials(@NonNull Activity mainActivity);

  public abstract SubscriptionPurchase getSubscriptionPurchase( @NonNull Dictionary dictionary );

  @NonNull
  abstract Map< Dictionary.DictionaryId, SubscriptionPurchase > getSubscriptionPurchases();

  public abstract boolean isTrialAvailable(@NonNull Dictionary.DictionaryId dictionaryId);

  public abstract boolean isTrialExpired(@NonNull Dictionary.DictionaryId dictionaryId);

  public abstract int getTrialCount();

  public abstract boolean isDictionariesInit();

  public abstract boolean isProductDemoFtsPromise();

  public abstract boolean isTrialAvailable(@NonNull Dictionary.DictionaryId dictionaryId, int n);

  public abstract int getTrialLengthInMinutes( @NonNull Dictionary.DictionaryId dictionaryId );

  public abstract long getTrialEndTime( @NonNull Dictionary.DictionaryId dictionaryId );

  @Nullable
  public abstract Date getUserCoreEndTime(@NonNull Dictionary.DictionaryId dictionaryId );

  @Nullable
  public abstract TrialControllerAPI createTrialController(Context context, int n, int baseRequestCode, @NonNull PendingIntentFactory pendingIntentFactory);

  abstract DownloadLibraryBuilder getDownloadLibraryBuilder();

  @Nullable
  public abstract Dictionary.DictionaryId getDictionaryIdFromTrialNotificationIntent(@Nullable Intent intent);

  @Nullable
  public abstract DictionaryAndDirection getDictionaryAndDirectionSelectedByUser();

  public abstract void setDictionaryAndDirectionSelectedByUser(@NonNull DictionaryAndDirection dictionaryAndDirection);

  public abstract Observable<Boolean> getDictionaryAndDirectionChangeObservable();

  public abstract void setIsSelectAllDictionaries(boolean isSelectAll);

  public abstract boolean isSelectAllDictionaries();

  public abstract void setDefaultDictionaryAndDirection( @Nullable Dictionary.DictionaryId dictionaryId, @Nullable Dictionary.Direction direction );

  public abstract boolean isExternalBaseDownloaded(@Nullable Dictionary.DictionaryId currentDictionaryId, @Nullable String baseId);

  public abstract void openSignIn(@NonNull Activity activity, int requestCode);

  public abstract void openSignIn(@NonNull Fragment fragment, int requestCode);

  public abstract void openSignIn(FragmentManager fragmentManager);

  public abstract void signOut(@NonNull Context context);

  public abstract LanguageStrings getLanguageStrings(@NonNull Context context, int language);

  public abstract void resetUserPassword(FragmentManager fragmentManager);

  @NonNull
  public abstract Object getScreenOpener();

    public interface IErrorObserver
  {
    void onError( @NonNull OperationType operationType, @NonNull List<ErrorType> error );
  }
  /**
   * Dictionary List Observer interface
   */
  public interface IDictionaryListObserver
  {
    /**
     * Will be called if dictionary list was changed
     */
    void onDictionaryListChanged();
  }

  public enum DownloadFailureReason
  {
    CONNECTION_UNAVAILABLE,
    CONNECTION_LOST,
    STORAGE_INSUFFICIENT_SPACE,
    FILESYSTEM_ERROR,
    FILE_CORRUPTED,
    COMMON_ERROR
  }

  public interface IOnDownloadFailureObserver
  {
    void onFailed( @NonNull DownloadFailureReason reason);
  }

  public interface IComponentDownloadProgressObserver
  {
    void onProgressChanged();

    void onStatusChanged();
  }

  /**
   * Download process notifier
   */
  public interface IDownloadCallback
  {
    /**
     * Called when another part of file was downloaded
     *
     * @param currentSize how many bytes downloaded for now
     * @param totalSize   total downloading file size
     */
    void onProgress( final long currentSize, final long totalSize );

    /**
     * called when download finished successfully
     */
    void onFinished();

    /**
     * called when download process failed
     *
     * @param reason contains exception that has been the fail reason
     */
    void onFailed( @NonNull final DictionaryManagerException reason );
  }

  public interface PendingIntentFactory {
    @Nullable
    PendingIntent createPendingIntent(@NonNull Fragment fragment, int requestCode);
  }
}
