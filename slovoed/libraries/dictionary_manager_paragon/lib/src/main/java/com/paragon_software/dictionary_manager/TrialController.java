package com.paragon_software.dictionary_manager;

import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.paragon_software.trial_manager.TrialComplexAPI;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

class TrialController implements TrialControllerAPI {
    interface FeatureNameFinder {
        @Nullable
        FeatureName findFeatureName(@Nullable Dictionary.DictionaryId dictionaryId);
    }

    @NonNull
    private final TrialComplexAPI mCore;

    @NonNull
    private final FeatureNameFinder mFeatureFinder;

    @NonNull
    private final Map<OnUpdateListener, TrialComplexAPI.OnUpdateListener> mListeners = new HashMap<>();

    @NonNull
    private final Map<ErrorCallbacks, TrialComplexAPI.ErrorCallbacks> mErrorCallbacks = new HashMap<>();

    TrialController(@NonNull TrialComplexAPI core, @NonNull FeatureNameFinder featureFinder) {
        mCore = core;
        mFeatureFinder = featureFinder;
    }

    @Override
    public void registerUpdateListener(@NonNull final OnUpdateListener listener) {
        if(!mListeners.containsKey(listener)) {
            TrialComplexAPI.OnUpdateListener coreListener = listener::onUpdated;
            mListeners.put(listener, coreListener);
            mCore.registerUpdateListener(coreListener);
        }
    }

    @Override
    public void unregisterUpdateListener(@NonNull OnUpdateListener listener) {
        TrialComplexAPI.OnUpdateListener coreListener = mListeners.get(listener);
        if(coreListener != null) {
            mCore.unregisterUpdateListener(coreListener);
            mListeners.remove(listener);
        }
    }

    @Override
    public void registerErrorCallbacks(@NonNull final ErrorCallbacks callbacks) {
        if(!mErrorCallbacks.containsKey(callbacks)) {
            TrialComplexAPI.ErrorCallbacks coreCallbacks = new TrialComplexAPI.ErrorCallbacks() {
                @Override
                public void onOk( @NonNull TrialItemInfo trialItemInfo )
                {
                    callbacks.onOk(trialItemInfo);
                }

                @Override
                public void onException(@NonNull Exception e) {
                    callbacks.onException(e);
                }

                @Override
                public void onNoPermission(@NonNull String[] permissions, @NonNull int[] granted) {
                    callbacks.onNoPermission(permissions, granted);
                }

                @Override
                public void displayToast(@NonNull String msg) {
                    callbacks.displayToast(msg);
                }

                @Override
                public void displayNoAccountDialog() {
                    callbacks.displayNoAccountDialog();
                }
            };
            mErrorCallbacks.put(callbacks, coreCallbacks);
            mCore.registerErrorCallbacks(coreCallbacks);
        }
    }

    @Override
    public void unregisterErrorCallbacks(@NonNull ErrorCallbacks callbacks) {
        TrialComplexAPI.ErrorCallbacks coreCallbacks = mErrorCallbacks.get(callbacks);
        if(coreCallbacks != null) {
            mCore.unregisterErrorCallbacks(coreCallbacks);
            mErrorCallbacks.remove(callbacks);
        }
    }

    @Override
    public void startTrialSequence(Context context, @NonNull Fragment fragment, @NonNull Dictionary.DictionaryId dictionaryId) {
        FeatureName featureName = mFeatureFinder.findFeatureName(dictionaryId);
        if(featureName != null)
            mCore.startSequence(context, fragment, featureName);
    }

    @Override
    public boolean handlePermissionResult(Context context, @NonNull Fragment fragment, int code, @NonNull String[] permissions, @NonNull int[] granted) {
        return mCore.handlePermissionResult(context, fragment, code, permissions, granted);
    }

    @Override
    public boolean handleResult(Context context, @NonNull Fragment fragment, int requestCode, int resultCode, Intent data) {
        return mCore.handleResult(context, fragment, requestCode, resultCode, data);
    }

    @Override
    public boolean isBusy() {
        return mCore.isBusy();
    }
}
