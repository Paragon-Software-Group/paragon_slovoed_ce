package com.paragon_software.dictionary_manager_ui;

import android.Manifest;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.TrialControllerAPI;
import com.paragon_software.dictionary_manager.TrialItemInfo;
import com.paragon_software.utils_slovoed_ui.StringsUtils;
import com.paragon_software.utils_slovoed_ui_common.activities.ParagonFragmentActivity;

public class DialogFragmentPreview extends DialogFragment
                                   implements ParagonFragmentActivity.ParagonFragment,
                                              DictionaryManagerAPI.IDictionaryListObserver,
                                              TrialControllerAPI.OnUpdateListener,
                                              TrialControllerAPI.ErrorCallbacks {
    private static final String TAG = DialogFragmentPreview.class.toString();

    private static final int TRIAL_BASE_REQUEST_CODE = 1946;

    @Nullable
    private DictionaryManagerAPI mDictionaryManager = null;
    @Nullable
    private TrialControllerAPI[] mTrialControllers  = null;

    private Button[] mTrialButtons;

    private Dictionary.DictionaryId mDictionaryId   = null;

    static void show(@Nullable FragmentManager fragmentManager, @Nullable Dictionary.DictionaryId dictionaryId) {
        if ((fragmentManager != null) && (dictionaryId != null)) {
            if (fragmentManager.findFragmentByTag(TAG) == null) {
                DialogFragment dialog = new DialogFragmentPreview();
                Bundle bundle = new Bundle(1);
                bundle.putParcelable(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY, dictionaryId);
                dialog.setArguments(bundle);
                dialog.show(fragmentManager, TAG);
            }
        }
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        mDictionaryManager = DictionaryManagerHolder.getManager();
        mTrialControllers = new TrialControllerAPI[mDictionaryManager.getTrialCount()];
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setStyle(STYLE_NO_TITLE, 0);
        Bundle bundle = getArguments();
        if (bundle != null)
            mDictionaryId = bundle.getParcelable(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY);
        for (int i = 0; i < mTrialControllers.length; i++)
            mTrialControllers[i] = mDictionaryManager.createTrialController(getContext(),
                                                                            i,
                                                                            TRIAL_BASE_REQUEST_CODE,
                                                                            new PendingIntentFactoryImpl());
        mDictionaryManager.loadOnlineDictionaryStatusInformation(this);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.preview_dialog_fragment, container, false);
        TextView previewTextView = view.findViewById(R.id.preview_text);
        previewTextView.setText(getString(R.string.dictionary_manager_ui_preview_dialog_text, createTrialLengthString(view.getContext())));
        View okButton = view.findViewById(R.id.ok);
        setClickListener(okButton);
        initTrialButtons(view);
        return view;
    }

    @Override
    public void onStart() {
        super.onStart();
        mDictionaryManager.registerDictionaryListObserver(this);
        for(TrialControllerAPI trialController : mTrialControllers)
            if(trialController != null) {
                trialController.registerUpdateListener(this);
                trialController.registerErrorCallbacks(this);
            }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateTrialRelatedState();
    }

    @Override
    public void onStop() {
        super.onStop();
        mDictionaryManager.unRegisterDictionaryListObserver(this);
        for(TrialControllerAPI trialController : mTrialControllers)
            if(trialController != null) {
                trialController.unregisterUpdateListener(this);
                trialController.unregisterErrorCallbacks(this);
            }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        boolean handled = false;
        for (TrialControllerAPI trialController : mTrialControllers)
            if (trialController != null) {
                handled = trialController.handlePermissionResult(getContext(), this, requestCode, permissions, grantResults);
                if (handled)
                    break;
            }
        if (!handled)
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        boolean handled = false;
        for (TrialControllerAPI trialController : mTrialControllers)
            if (trialController != null) {
                handled = trialController.handleResult(getContext(), this, requestCode, resultCode, data);
                if (handled)
                    break;
            }
        if (!handled)
            super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onFragmentResult(int requestCode, int resultCode, Intent data) {
        for (TrialControllerAPI trialController : mTrialControllers)
            if (trialController != null) {
                if (trialController.handleResult(getContext(), this, requestCode, resultCode, data))
                    break;
            }
    }

    private void setClickListener(View okButton) {
        if (null != okButton) {
            okButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    dismiss();
                }
            });
        }
    }

    private void initTrialButtons(View view) {
        mTrialButtons = new Button[1];
        mTrialButtons[0] = view.findViewById(R.id.google_trial);
        mTrialButtons[0].setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mTrialControllers[0].startTrialSequence(getContext(), DialogFragmentPreview.this, mDictionaryId);
            }
        });
    }

    private void updateTrialRelatedState() {
        int n = Math.min(mTrialControllers.length, mTrialButtons.length);
        for (int i = 0; i < n; i++) {
            boolean isBusy = false;
            if (mTrialControllers[i] != null)
                isBusy = mTrialControllers[i].isBusy();
            boolean isAvailable = mDictionaryManager.isTrialAvailable(mDictionaryId, i);
            mTrialButtons[i].setEnabled(isAvailable && (!isBusy));
        }
        if(!mDictionaryManager.isTrialAvailable(mDictionaryId))
        {
            dismiss();
            mDictionaryManager.openMyDictionariesUI(getContext(), mDictionaryId);
        }
    }

    @Override
    public void onDictionaryListChanged() {
        updateTrialRelatedState();
    }

    @Override
    public void onUpdated() {
        updateTrialRelatedState();
    }

    @Override
    public void onOk( @NonNull TrialItemInfo trialItemInfo ) {

    }

    @Override
    public void onException(@NonNull Exception e) {
        displayToast(getString(R.string.utils_slovoed_ui_common_failed_to_start_trial_message));
    }

    @Override
    public void onNoPermission(@NonNull String[] permissions, @NonNull int[] granted) {
        int i, n = Math.min(permissions.length, granted.length);
        for(i = 0; i < n; i++)
            if(granted[i] != PackageManager.PERMISSION_GRANTED)
                break;
        if(i < n) {
            String permission = permissions[i];
            if(Manifest.permission.GET_ACCOUNTS.equals(permission)) {
                Activity activity = getActivity();
                if(activity != null)
                    if (!ActivityCompat.shouldShowRequestPermissionRationale(activity, android.Manifest.permission.GET_ACCOUNTS))
                        DialogFragmentAccessDenied.show(getFragmentManager(), DialogFragmentAccessDenied.Problem.noContactsPermission);
            }
            else if(Manifest.permission.READ_PHONE_STATE.equals(permission))
                DialogFragmentAccessDenied.show(getFragmentManager(), DialogFragmentAccessDenied.Problem.noReadPhoneStatePermission);
        }
    }

    @Override
    public void displayToast(@NonNull String msg) {
        Toast.makeText(getContext(), msg, Toast.LENGTH_LONG).show();
    }

    @Override
    public void displayNoAccountDialog() {
        DialogFragmentAccessDenied.show(getFragmentManager(), DialogFragmentAccessDenied.Problem.noAccount);
    }

    private String createTrialLengthString(@NonNull Context context) {
        return StringsUtils.createTrialLengthString(context, mDictionaryManager.getTrialLengthInMinutes(mDictionaryId));
    }

    private static class PendingIntentFactoryImpl implements DictionaryManagerAPI.PendingIntentFactory {
        @Nullable
        @Override
        public PendingIntent createPendingIntent(@NonNull Fragment fragment, int requestCode) {
            return ParagonFragmentActivity.createFragmentSafePendingIntent(fragment, requestCode);
        }
    }
}
