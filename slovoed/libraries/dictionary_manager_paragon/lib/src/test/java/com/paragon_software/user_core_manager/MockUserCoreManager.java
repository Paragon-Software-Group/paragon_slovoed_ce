package com.paragon_software.user_core_manager;

import androidx.annotation.NonNull;

import java.util.List;

import io.reactivex.Observable;

public abstract class MockUserCoreManager extends UserCoreManagerAPI
{
  @NonNull
  @Override
  public Observable< List< ProductLicense > > getProductLicenses()
  {
    return Observable.empty();
  }
}
