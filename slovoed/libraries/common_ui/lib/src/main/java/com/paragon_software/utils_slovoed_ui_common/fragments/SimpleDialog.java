package com.paragon_software.utils_slovoed_ui_common.fragments;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.widget.TextView;

public class SimpleDialog extends DialogFragment
                                  implements Dialog.OnClickListener {
    private static final String TITLE_ID = "com.paragon_software.utils_slovoed_ui.fragments.SimpleDialog.TITLE_ID";
    private static final String MESSAGE_ID = "com.paragon_software.utils_slovoed_ui.fragments.SimpleDialog.MESSAGE_ID";
    private static final String EXTRA_VIEW_ID = "com.paragon_software.utils_slovoed_ui.fragments.SimpleDialog.EXTRA_VIEW_ID";
    private static final String POSITIVE_TEXT_ID = "com.paragon_software.utils_slovoed_ui.fragments.SimpleDialog.POSITIVE_TEXT_ID";
    private static final String NEGATIVE_TEXT_ID = "com.paragon_software.utils_slovoed_ui.fragments.SimpleDialog.NEGATIVE_TEXT_ID";
    private static final String CANCELABLE_ID = "com.paragon_software.utils_slovoed_ui.fragments.SimpleDialog.CANCELABLE_ID";
    private static final String EXTRA_ID = "com.paragon_software.utils_slovoed_ui.fragments.SimpleDialog.EXTRA_ID";

    public interface Target {
        void onCreateSimpleDialog(@Nullable String tag, @NonNull Dialog dialog, @Nullable Bundle extra);
        void onSimpleDialogButtonPressed(@Nullable String tag, int n, @Nullable Bundle extra);
    }

    public static <T extends FragmentActivity & Target> void show(@NonNull T parentActivity,
                                                                  @NonNull String tag,
                                                                  @NonNull Builder builder)
    {
        DialogFragment newFragment = builder.build();
        newFragment.show(parentActivity.getSupportFragmentManager(), tag);
    }

    public static <T extends Fragment & Target> void show(@NonNull T parentFragment,
                                                          @NonNull String tag,
                                                          @NonNull Builder builder)
    {
        DialogFragment newFragment = builder
                .setParentFragment(parentFragment)
                .build();
        Fragment fragment = parentFragment.requireFragmentManager().findFragmentByTag(tag);
        if(fragment == null)
        {
            newFragment.show(parentFragment.requireFragmentManager(), tag);
        }
    }

    @NonNull
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Context context = getActivity();
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        Bundle bundle = getArguments();
        String s = getString(bundle, TITLE_ID);
        boolean isCancelable = true;
        if(s != null)
            builder.setTitle(s);
        s = getString(bundle, MESSAGE_ID);
        if(s != null)
            builder.setMessage(Html.fromHtml(s));
        if(bundle != null) {
            if (bundle.containsKey(EXTRA_VIEW_ID))
                builder.setView(bundle.getInt(EXTRA_VIEW_ID));
            if(bundle.containsKey(CANCELABLE_ID))
                isCancelable = bundle.getBoolean(CANCELABLE_ID);
        }
        s = getString(bundle, POSITIVE_TEXT_ID);
        if(s != null)
            builder.setPositiveButton(s, this);
        s = getString(bundle, NEGATIVE_TEXT_ID);
        if(s != null)
            builder.setNegativeButton(s, this);
        Dialog d = builder.show();
        d.setCanceledOnTouchOutside(isCancelable);
        TextView message = d.findViewById(android.R.id.message);
        if(message != null)
            message.setMovementMethod(LinkMovementMethod.getInstance());
        Target target = getTarget();
        if(target != null)
            target.onCreateSimpleDialog(getTag(), d, getExtra());
        return d;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        Target target = getTarget();
        if(target != null)
            target.onSimpleDialogButtonPressed(getTag(), which, getExtra());
    }

    @Nullable
    private Bundle getExtra() {
        Bundle extra = null;
        Bundle args = getArguments();
        if(args != null)
            extra = args.getBundle(EXTRA_ID);
        return extra;
    }

    @Nullable
    private Target getTarget() {
        Target res = null;
        Fragment targetFragment = getTargetFragment();
        if(targetFragment instanceof Target)
            res = (Target) targetFragment;
        else {
            FragmentActivity parentActivity = getActivity();
            if(parentActivity instanceof Target)
                res = (Target) parentActivity;
        }
        return res;
    }

    @Nullable
    private static String getString(Bundle bundle, String id) {
        String res = null;
        if((bundle != null) && (id != null))
            res = bundle.getString(id);
        return res;
    }

    public static class Builder
    {
        @Nullable
        private Fragment mParentFragment;
        @Nullable
        private String mTitle;
        @Nullable
        private String mMessage;
        @Nullable
        @LayoutRes
        private Integer mExtraViewId;
        @Nullable
        private String mPositiveText;
        @Nullable
        private String mNegativeText;
        @Nullable
        private Bundle mExtra;
        @Nullable
        private Boolean mCancelable;

        public Builder setParentFragment(@Nullable Fragment parentFragment)
        {
            this.mParentFragment = parentFragment;
            return this;
        }

        public Builder setTitle(@Nullable String mTitle)
        {
            this.mTitle = mTitle;
            return this;
        }

        public Builder setMessage(@Nullable String mMessage)
        {
            this.mMessage = mMessage;
            return this;
        }

        public Builder setExtraViewId(@Nullable Integer mExtraViewId)
        {
            this.mExtraViewId = mExtraViewId;
            return this;
        }

        public Builder setPositiveText(@Nullable String mPositiveText)
        {
            this.mPositiveText = mPositiveText;
            return this;
        }

        public Builder setNegativeText(@Nullable String mNegativeText)
        {
            this.mNegativeText = mNegativeText;
            return this;
        }

        public Builder setExtra(@Nullable Bundle mExtra)
        {
            this.mExtra = mExtra;
            return this;
        }

        public Builder setCancelable(@NonNull Boolean mCancelable)
        {
            this.mCancelable = mCancelable;
            return this;
        }

        SimpleDialog build()
        {
            SimpleDialog newFragment = new SimpleDialog();
            if(mParentFragment != null)
                newFragment.setTargetFragment(mParentFragment, 0);

            Bundle bundle = new Bundle(2);
            if(mTitle != null)
                bundle.putString(TITLE_ID, mTitle);
            if(mMessage != null)
                bundle.putString(MESSAGE_ID, mMessage);
            if(mExtraViewId != null)
                bundle.putInt(EXTRA_VIEW_ID, mExtraViewId);
            if(mPositiveText != null)
                bundle.putString(POSITIVE_TEXT_ID, mPositiveText);
            if(mNegativeText != null)
                bundle.putString(NEGATIVE_TEXT_ID, mNegativeText);
            if(mExtra != null)
                bundle.putBundle(EXTRA_ID, mExtra);
            if(mCancelable != null)
                bundle.putBoolean(CANCELABLE_ID, mCancelable);
            newFragment.setArguments(bundle);

            return newFragment;
        }
    }
}
