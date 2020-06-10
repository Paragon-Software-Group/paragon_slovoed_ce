package com.paragon_software.utils_slovoed_ui_common.activities;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.google.android.material.snackbar.Snackbar;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import java.util.Map;
import java.util.TreeMap;

public class SnackbarActivity extends Activity implements View.OnClickListener, Runnable {
    public interface Listener {
        void onAction(Context context, @Nullable Bundle bundle);
    }

    public static void addListener(@NonNull String key, @NonNull Listener listener) {
        LISTENERS.put(key, listener);
    }

    public static void show(Context context, @NonNull String message) {
        show(context, message, "", "", null);
    }

    public static void show(Context context, @NonNull String message, @NonNull String buttonText, @NonNull String listenerKey, @Nullable Bundle listenerBundle) {
        Intent intent = new Intent(context, SnackbarActivity.class);
        intent.putExtra(EXTRA_MESSAGE, message);
        intent.putExtra(EXTRA_BUTTON_TEXT, buttonText);
        intent.putExtra(EXTRA_LISTENER_KEY, listenerKey);
        if(listenerBundle != null)
            intent.putExtra(EXTRA_LISTENER_BUNDLE, listenerBundle);
        context.startActivity(intent);
    }

    private static final long DELAY = 100;
    private static final String EXTRA_MESSAGE = "utils_slovoed_ui_common.lib.src.main.java.activities.SnackbarActivity.extra_message";
    private static final String EXTRA_BUTTON_TEXT = "utils_slovoed_ui_common.lib.src.main.java.activities.SnackbarActivity.extra_button_text";
    private static final String EXTRA_LISTENER_KEY = "utils_slovoed_ui_common.lib.src.main.java.activities.SnackbarActivity.extra_listener_key";
    private static final String EXTRA_LISTENER_BUNDLE = "utils_slovoed_ui_common.lib.src.main.java.activities.SnackbarActivity.extra_listener_bundle";

    @NonNull
    private static final Map<String, Listener> LISTENERS = new TreeMap<>();

    private Snackbar mSnackbar;
    private String mButtonText;
    private String mListenerKey;
    private Bundle mListenerBundle;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String message = intent.getStringExtra(EXTRA_MESSAGE);
        mButtonText = intent.getStringExtra(EXTRA_BUTTON_TEXT);
        mListenerKey = intent.getStringExtra(EXTRA_LISTENER_KEY);
        mListenerBundle = intent.getBundleExtra(EXTRA_LISTENER_BUNDLE);
        View v = new FrameLayout(this);
        v.setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        v.setBackgroundResource(android.R.color.transparent);
        v.setOnClickListener(this);
        mSnackbar = Snackbar.make(v, message, Snackbar.LENGTH_LONG);
        mSnackbar.addCallback(new SnackbarCallback());
        View text = mSnackbar.getView().findViewById(com.google.android.material.R.id.snackbar_text);
        if(text instanceof TextView)
            ((TextView) text).setLines(4);
        if(!TextUtils.isEmpty(mButtonText))
            mSnackbar.setAction(mButtonText, this);
        setContentView(v);
        v.postDelayed(this, DELAY);
    }

    @Override
    public void onClick(View v) {
        if(mSnackbar != null) {
            mSnackbar.dismiss();
            mSnackbar = null;
        }
    }

    @Override
    public void run() {
        if(mSnackbar != null)
            mSnackbar.show();
    }

    private class SnackbarCallback extends Snackbar.Callback {
        @Override
        public void onDismissed(Snackbar transientBottomBar, int event) {
            super.onDismissed(transientBottomBar, event);
            mSnackbar = null;
            finishAndRemoveTask();
            if((event == Snackbar.Callback.DISMISS_EVENT_ACTION) && (!TextUtils.isEmpty(mButtonText)) && (!TextUtils.isEmpty(mListenerKey))) {
                Listener listener = LISTENERS.get(mListenerKey);
                if(listener != null)
                    listener.onAction(SnackbarActivity.this, mListenerBundle);
            }
        }
    }
}
