package com.paragon_software.dictionary_manager;

import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.license_manager_api.LicenseFeature;
import com.paragon_software.license_manager_api.LicenseManager;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

abstract class MockLicenseManager implements LicenseManager
{
  @NonNull
  private final List<LicenseFeature> mFeatures = new ArrayList<>();

  MockLicenseManager( @NonNull FeatureSKUProvider featureSKUProvider, List< FeatureName > featureNames )
  {
    for ( FeatureName name : featureNames )
    {
      mFeatures.add(featureSKUProvider.getLicenceFeatures(name));
    }
  }

  @NonNull
  @Override
  public LicenseFeature.FEATURE_STATE checkFeature( @NonNull FeatureName s )
  {
    LicenseFeature licenseFeature = getFeature(s);
    return ( licenseFeature != null ) ? LicenseFeature.FEATURE_STATE.AVAILABLE : LicenseFeature.FEATURE_STATE.DISABLED;
  }

  @Nullable
  @Override
  public LicenseFeature getFeature( @NonNull FeatureName s )
  {
    LicenseFeature res = null;
    for ( LicenseFeature licenseFeature : mFeatures )
    {
      if ( licenseFeature.getName().equals(s) )
      {
        res = licenseFeature;
        break;
      }
    }
    return res;
  }

  @NonNull
  @Override
  public Collection< LicenseFeature > getFeatures()
  {
    return Collections.unmodifiableCollection(mFeatures);
  }

  @Override
  public boolean activationEnd( int i, int resultCode, @NonNull Intent intent )
  {
    return false;
  }

  interface FeatureSKUProvider
  {
    @NonNull
    LicenseFeature getLicenceFeatures( @NonNull FeatureName featureName );
  }
}
