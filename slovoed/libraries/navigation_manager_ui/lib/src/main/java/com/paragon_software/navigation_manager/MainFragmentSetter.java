package com.paragon_software.navigation_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.paragon_software.navigation_manager_ui.R;

class MainFragmentSetter {
    private static final int MAIN_FRAGMENT_ID = R.id.fragment_frame;

    private Fragment mCurrentFragment;

    void setPendingMainFragment(@Nullable Fragment mainFragment) {
        mCurrentFragment = mainFragment;
    }

    void commitMainFragmentTransaction(@NonNull FragmentManager fragmentManager) {
        if ( null != mCurrentFragment ) {
            replaceMainFragment(fragmentManager, mCurrentFragment);
            mCurrentFragment = null;
        }
    }

    static void replaceMainFragment(@NonNull FragmentManager fragmentManager, @NonNull Fragment fragment) {
        FragmentTransaction transaction = fragmentManager.beginTransaction();
        transaction.setCustomAnimations(android.R.anim.slide_in_left, android.R.anim.slide_out_right);
        transaction.replace(MAIN_FRAGMENT_ID, fragment);
        transaction.commit();
    }
}
