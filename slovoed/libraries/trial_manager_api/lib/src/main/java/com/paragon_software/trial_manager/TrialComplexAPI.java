package com.paragon_software.trial_manager;

import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import java.io.Serializable;

import com.paragon_software.dictionary_manager.FeatureName;
import com.paragon_software.dictionary_manager.TrialItemInfo;

public interface TrialComplexAPI {
    interface OnUpdateListener {
        void onUpdate();
    }

    interface ErrorCallbacks {
        void onOk(@NonNull TrialItemInfo trialItemData );
        void onException(@NonNull Exception e);
        void onNoPermission(@NonNull String[] permissions, @NonNull int[] granted);
        void displayToast(@NonNull String msg);
        void displayNoAccountDialog();
    }

    void registerUpdateListener(@NonNull OnUpdateListener listener);
    void unregisterUpdateListener(@NonNull OnUpdateListener listener);
    void registerErrorCallbacks(@NonNull ErrorCallbacks callbacks);
    void unregisterErrorCallbacks(@NonNull ErrorCallbacks callbacks);
    void startSequence(Context context, @NonNull Fragment fragment, @NonNull FeatureName featureName);
    boolean handlePermissionResult(Context context, @NonNull Fragment fragment, int code, @NonNull String[] permissions, @NonNull int[] granted);
    boolean handleResult(Context context, @NonNull Fragment fragment, int requestCode, int resultCode, Intent data);
    boolean isBusy();
}
