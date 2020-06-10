package com.paragon_software.dictionary_manager;

import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import java.io.Serializable;

public interface TrialControllerAPI {
    interface OnUpdateListener {
        void onUpdated();
    }

    interface ErrorCallbacks {
        void onOk(@NonNull TrialItemInfo trialItemInfo );
        void onException(@NonNull Exception e);
        void onNoPermission(@NonNull String[] permissions, @NonNull int[] granted);
        void displayToast(@NonNull String msg);
        void displayNoAccountDialog();
    }

    void registerUpdateListener(@NonNull OnUpdateListener listener);
    void unregisterUpdateListener(@NonNull OnUpdateListener listener);
    void registerErrorCallbacks(@NonNull ErrorCallbacks callbacks);
    void unregisterErrorCallbacks(@NonNull ErrorCallbacks callbacks);
    void startTrialSequence(Context context, @NonNull Fragment fragment, @NonNull Dictionary.DictionaryId dictionaryId);
    boolean handlePermissionResult(Context context, @NonNull Fragment fragment, int code, @NonNull String[] permissions, @NonNull int[] granted);
    boolean handleResult(Context context, @NonNull Fragment fragment, int requestCode, int resultCode, Intent data);
    boolean isBusy();
}
