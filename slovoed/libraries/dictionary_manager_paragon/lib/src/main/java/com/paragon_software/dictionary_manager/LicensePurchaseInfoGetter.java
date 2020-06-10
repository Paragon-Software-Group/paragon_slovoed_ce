package com.paragon_software.dictionary_manager;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.license_manager_api.LicenseFeature;
import com.paragon_software.license_manager_api.LicenseManager;
import com.paragon_software.trial_manager.TrialManagerAPI;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

class LicensePurchaseInfoGetter implements TrialManagerAPI.PurchaseInfoGetter {
    private static final long TIMEOUT = 5L * 60L * 1000L;

    @NonNull
    private final LicenseManager mLicenseManager;

    @NonNull
    private final LicenseManagerListener mLicenseManagerListener = new LicenseManagerListener();

    private boolean needToWait = true;

    LicensePurchaseInfoGetter(@NonNull LicenseManager licenseManager) {
        mLicenseManager = licenseManager;
    }

    @Override
    public boolean isPurchased(Context context, @NonNull FeatureName featureName) {
        if (needToWait) {
            mLicenseManager.registerNotifier(mLicenseManagerListener);
            mLicenseManager.update(context);
            mLicenseManagerListener.await(TIMEOUT);
            mLicenseManager.unregisterNotifier(mLicenseManagerListener);
            needToWait = false;
        }
        return LicenseFeature.FEATURE_STATE.ENABLED.equals(mLicenseManager.checkFeature(featureName));
    }

    private static class LicenseManagerListener implements LicenseManager.Notifier {
        @NonNull
        private final CountDownLatch latch = new CountDownLatch(1);

        @Override
        public void onChange() {
            latch.countDown();
        }

        void await(long millis) {
            try {
                latch.await(millis, TimeUnit.MILLISECONDS);
            } catch (InterruptedException ignore) {
            }
        }
    }
}
