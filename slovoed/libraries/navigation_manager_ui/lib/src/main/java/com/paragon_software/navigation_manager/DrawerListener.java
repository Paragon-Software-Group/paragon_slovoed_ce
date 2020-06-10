package com.paragon_software.navigation_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import com.google.android.material.navigation.NavigationView;
import androidx.fragment.app.FragmentActivity;
import androidx.drawerlayout.widget.DrawerLayout;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

import java.lang.ref.WeakReference;

class DrawerListener extends DrawerLayout.SimpleDrawerListener {
    @NonNull
    private final WeakReference<FragmentActivity> mWeakActivity;

    @NonNull
    private final MainFragmentSetter mMainFragmentSetter;

    DrawerListener(@NonNull FragmentActivity activity,
                   @NonNull MainFragmentSetter mainFragmentSetter) {
        mWeakActivity = new WeakReference<>(activity);
        mMainFragmentSetter = mainFragmentSetter;
    }

    @Override
    public void onDrawerStateChanged(int newState) {
        super.onDrawerStateChanged(newState);
        FragmentActivity activity = mWeakActivity.get();
        if ((DrawerLayout.STATE_SETTLING == newState) && (activity != null)) {
            View view = activity.getCurrentFocus();
            InputMethodManager imm = (InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
            if ((null != view) && (null != imm))
                imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
        }
    }

    @Override
    public void onDrawerClosed( View drawerView ) {
        FragmentActivity activity = mWeakActivity.get();
        if(activity != null)
            mMainFragmentSetter.commitMainFragmentTransaction(activity.getSupportFragmentManager());
    }
}
