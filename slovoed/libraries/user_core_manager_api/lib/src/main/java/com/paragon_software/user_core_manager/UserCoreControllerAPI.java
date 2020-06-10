package com.paragon_software.user_core_manager;

import android.content.Context;
import androidx.annotation.NonNull;

import io.reactivex.Completable;
import io.reactivex.Observable;
import io.reactivex.Single;

abstract class UserCoreControllerAPI {
  @NonNull
  abstract Single< String > getWebViewLoginUrl( @NonNull Context context );

  @NonNull
  abstract Completable handleRedirectResult( @NonNull String rawRedirectUrl );

  @NonNull
  abstract Completable discardJwt( @NonNull String jwt );

  public abstract void login(String login, String password);

  @NonNull
  public abstract Observable<Throwable> getErrorObserver();

  public abstract void resetUserPassword(String login);
}
