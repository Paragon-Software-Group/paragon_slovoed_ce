package com.paragon_software.dictionary_manager_ui;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.fragment.app.FragmentManager;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.utils_slovoed_ui.StringsUtils;
import com.paragon_software.utils_slovoed_ui_common.fragments.SimpleDialog;

import java.io.Serializable;

class ConsentDialogHandler implements SimpleDialog.Target {
    @NonNull
    private final FragmentManager mFragmentManager;

    @NonNull
    private final String mTag;

    @NonNull
    private final String mDictionaryIdExtra;

    ConsentDialogHandler(@NonNull FragmentManager fragmentManager, @NonNull String tag) {
        mFragmentManager = fragmentManager;
        mTag = tag;
        mDictionaryIdExtra = tag + ".DICTIONARY_ID_EXTRA";

    }

    <T extends FragmentActivity & SimpleDialog.Target> void showDialog(@NonNull T activity, @NonNull Dictionary.DictionaryId dictionaryId) {
        SimpleDialog.show(activity,
                          mTag,
                          new SimpleDialog.Builder()
                            .setExtraViewId(R.layout.consent_dialog_contents)
                            .setPositiveText(activity.getString(R.string.utils_slovoed_ui_agree))
                            .setNegativeText(activity.getString(R.string.utils_slovoed_ui_common_cancel))
                            .setExtra(createExtrasBundle(dictionaryId)));
    }

    <T extends Fragment & SimpleDialog.Target> void showDialog(@NonNull T fragment, @NonNull Dictionary.DictionaryId dictionaryId) {
        SimpleDialog.show(fragment,
                mTag,
                new SimpleDialog.Builder()
                        .setParentFragment(fragment)
                        .setExtraViewId(R.layout.consent_dialog_contents)
                        .setPositiveText(fragment.getString(R.string.utils_slovoed_ui_agree))
                        .setNegativeText(fragment.getString(R.string.utils_slovoed_ui_common_cancel))
                        .setExtra(createExtrasBundle(dictionaryId)));
    }

    @Override
    public void onCreateSimpleDialog(@Nullable String tag, @NonNull Dialog dialog, @Nullable Bundle extra) {
        TextView textView = dialog.findViewById(R.id.consent_body2);
        Dictionary.DictionaryId dictionaryId = getDictionaryId(extra);
        if((textView != null) && (dictionaryId != null)) {
            textView.setText(Html.fromHtml(textView.getContext().getString(R.string.dictionary_manager_ui_consent_dlg_body2, createTrialLengthString(textView.getContext(), dictionaryId))));
            textView.setMovementMethod(LinkMovementMethod.getInstance());
        }
        CheckBox checkBox1 = dialog.findViewById(R.id.agree_checkbox1);
        CheckBox checkBox2 = dialog.findViewById(R.id.agree_checkbox2);
        if((checkBox1 != null) && (checkBox2 != null)) {
            CompoundButton.OnCheckedChangeListener listener = new ConsentCheckBoxListener(dialog);
            checkBox1.setOnCheckedChangeListener(listener);
            checkBox2.setOnCheckedChangeListener(listener);
            listener.onCheckedChanged(checkBox1, checkBox1.isChecked());
        }
    }

    @Override
    public void onSimpleDialogButtonPressed(@Nullable String tag, int n, @Nullable Bundle extra) {
        if(mTag.equals(tag) && (n == Dialog.BUTTON_POSITIVE)) {
            Dictionary.DictionaryId dictionaryId = getDictionaryId(extra);
            if(dictionaryId != null)
                DialogFragmentPreview.show(mFragmentManager, dictionaryId);
        }
    }

    @Nullable
    private Dictionary.DictionaryId getDictionaryId(@Nullable Bundle extra) {
        Dictionary.DictionaryId dictionaryId = null;
        if(extra != null) {
            Serializable s = extra.getSerializable(mDictionaryIdExtra);
            if(s instanceof Dictionary.DictionaryId)
                dictionaryId = (Dictionary.DictionaryId) s;
        }
        return dictionaryId;
    }

    @NonNull
    private Bundle createExtrasBundle(@NonNull Dictionary.DictionaryId dictionaryId) {
        Bundle bundle = new Bundle(1);
        bundle.putSerializable(mDictionaryIdExtra, dictionaryId);
        return bundle;
    }

    private static String createTrialLengthString(@NonNull Context context, @NonNull Dictionary.DictionaryId dictionaryId ) {
        return StringsUtils.createTrialLengthString(context, DictionaryManagerHolder.getManager().getTrialLengthInMinutes(dictionaryId));
    }

    private static class ConsentCheckBoxListener implements CompoundButton.OnCheckedChangeListener {
        @NonNull
        private final Dialog dialog;

        ConsentCheckBoxListener(@NonNull Dialog _dialog) {
            dialog = _dialog;
        }

        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if(dialog instanceof AlertDialog) {
                CheckBox checkBox1 = dialog.findViewById(R.id.agree_checkbox1);
                CheckBox checkBox2 = dialog.findViewById(R.id.agree_checkbox2);
                Button positiveButton = ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE);
                if((checkBox1 != null) && (checkBox2 != null) && (positiveButton != null))
                    positiveButton.setEnabled(checkBox1.isChecked() && checkBox2.isChecked());
            }
        }
    }
}
