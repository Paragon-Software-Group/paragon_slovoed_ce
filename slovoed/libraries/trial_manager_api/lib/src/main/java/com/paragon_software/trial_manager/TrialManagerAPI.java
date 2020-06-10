package com.paragon_software.trial_manager;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;

import androidx.annotation.DrawableRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import com.paragon_software.dictionary_manager.FeatureName;
import com.paragon_software.utils_slovoed.text.LocalizedString;

public interface TrialManagerAPI {
    interface Builder {
        void setLoaderBaseId(int baseId);

        void addFeature(@NonNull FeatureName featureName,
                        @NonNull LocalizedString userTitle,
                        int lengthInMinutes,
                        @Nullable Long pdahpcId,
                        @Nullable String bo4Sku,
                        @Nullable Long bo4Matter);

        void setMaxTrialProlongationCountExceededErrorMsg(@StringRes int msgId);

        void setNotificationOptions(@NonNull Class<? extends Activity> activity,
                                    @NonNull String channelId,
                                    @DrawableRes int smallIconResId,
                                    @NonNull Bitmap largeIcon,
                                    @StringRes int titleExpiredId,
                                    @StringRes int titleExpiringId,
                                    @StringRes int messageExpiredId,
                                    @StringRes int messageExpiringId,
                                    int baseId);

        void setPurchaseInfoGetter(@NonNull PurchaseInfoGetter purchaseInfoGetter);

        @NonNull
        TrialManagerAPI build(Context context);
    }

    interface PurchaseInfoGetter {
        boolean isPurchased(Context context, @NonNull FeatureName featureName);
    }

    interface OnUpdateListener {
        void onUpdate();
    }

    interface PendingIntentFactory {
        @Nullable
        PendingIntent createPendingIntent(@NonNull Fragment fragment, int requestCode);
    }

    void startLoaders(@NonNull FragmentActivity activity);
    void startLoaders(@NonNull Fragment fragment);
    void updateOnlineTrials(@NonNull Activity mainActivity);
    void updatePurchaseStatus(Context context, @NonNull FeatureName featureName, boolean bought);
    boolean isTrialAvailable(@NonNull FeatureName featureName);
    boolean isTrialActive(@NonNull FeatureName featureName);
    boolean isTrialExpired(@NonNull FeatureName featureName);
    void registerOnUpdateListener(@NonNull OnUpdateListener listener);
    int getTrialCount();
    int getTrialLengthInMinutes(@NonNull FeatureName featureName);
    long getEndTime(@NonNull FeatureName featureName);
    boolean isTrialAvailable(@NonNull FeatureName featureName, int n);

    @Nullable
    TrialComplexAPI createTrialComplex(Context context, int n, int baseRequestCode, @NonNull PendingIntentFactory pendingIntentFactory);

    @Nullable
    FeatureName getFeatureNameFromNotificationIntent(@Nullable Intent intent);
}
