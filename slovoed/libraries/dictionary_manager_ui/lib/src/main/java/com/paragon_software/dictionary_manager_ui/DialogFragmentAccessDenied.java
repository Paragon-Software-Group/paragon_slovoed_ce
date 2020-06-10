package com.paragon_software.dictionary_manager_ui;

import android.app.Dialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.io.Serializable;

public class DialogFragmentAccessDenied extends DialogFragment implements View.OnClickListener {
    public enum Problem {
        noContactsPermission,
        noAccount,
        noWriteExternalStoragePermission,
        safNotTurnedOn,
        noReadPhoneStatePermission
    }

    private static final String TAG = DialogFragmentAccessDenied.class.getName();
    private static final String EXTRA_PROBLEM = "EXTRA_PROBLEM";

    public static void show(FragmentManager fm, Problem problem) {
        Bundle b = new Bundle(1);
        b.putSerializable(EXTRA_PROBLEM, problem);
        DialogFragmentAccessDenied dlg = new DialogFragmentAccessDenied();
        dlg.setArguments(b);
        dlg.show(fm, TAG);
    }

    public DialogFragmentAccessDenied() {}

    @NonNull
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        setStyle(STYLE_NO_TITLE, 0);
        return super.onCreateDialog(savedInstanceState);
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View res = inflater.inflate(R.layout.dialog_access_account_denied, container, false);

        Problem problem = getProblem();
        @StringRes int strId = 0;
        if(problem == Problem.noContactsPermission)
            strId = R.string.dictionary_manager_ui_trial_explain_dialog_message_permission;
        else if(problem == Problem.noAccount)
            strId = R.string.dictionary_manager_ui_trial_explain_dialog_message_account;
        else if(problem == Problem.noWriteExternalStoragePermission)
            strId = R.string.dictionary_manager_ui_need_write_external_storage_permission;
        else if(problem == Problem.noReadPhoneStatePermission)
            strId = R.string.dictionary_manager_ui_trial_explain_dialog_message_permission_read_phone_state;
        else if(problem == Problem.safNotTurnedOn)
            strId = R.string.dictionary_manager_ui_enable_saf_help;

        TextView txt = res.findViewById(R.id.explain);
        if(strId != 0)
            txt.setText(strId);

        res.findViewById(R.id.open_settings).setOnClickListener(this);
        res.findViewById(R.id.cancel).setOnClickListener(this);

        return res;
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.open_settings) {
            Problem problem = getProblem();
            Intent intent = null;
            if((problem == Problem.noContactsPermission) || (problem == Problem.noWriteExternalStoragePermission) || (problem == Problem.noReadPhoneStatePermission))
                intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS, Uri.parse("package:" + view.getContext().getPackageName()));
            else if(problem == Problem.noAccount) {
                intent = new Intent(Settings.ACTION_ADD_ACCOUNT);
                if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2)
                    intent.putExtra(Settings.EXTRA_ACCOUNT_TYPES, new String[] {"com.google"});
            }
            else if(problem == Problem.safNotTurnedOn)
                intent = new Intent(android.provider.Settings.ACTION_SETTINGS);
            try {
                if(intent != null)
                    startActivity(intent);
            }
            catch(Exception ignore) { }
            dismiss();
        }
        else if (view.getId() == R.id.cancel) {
            dismiss();
        }
    }

    private Problem getProblem() {
        Problem problem = Problem.noContactsPermission;
        Bundle args = getArguments();
        if(args != null) {
            Serializable s = args.getSerializable(EXTRA_PROBLEM);
            if(s instanceof Problem)
                problem = (Problem) s;
        }
        return problem;
    }
}
