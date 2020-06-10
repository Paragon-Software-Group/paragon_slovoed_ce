package com.paragon_software.navigation_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.google.android.material.navigation.NavigationView;
import androidx.fragment.app.Fragment;
import androidx.drawerlayout.widget.DrawerLayout;
import android.view.MenuItem;

import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.native_engine.EngineVersion;
import com.paragon_software.navigation_manager_ui.R;
import com.paragon_software.utils_slovoed_ui.MailUtils;

class NavigationListener implements NavigationView.OnNavigationItemSelectedListener {
    @NonNull
    private final DrawerLayout mDrawerLayout;

    @NonNull
    private final NavigationControllerApi mNavigationController;

    @NonNull
    private final MainFragmentSetter         mMainFragmentSetter;

    NavigationListener(@NonNull DrawerLayout drawerLayout,
                       @NonNull NavigationControllerApi navigationController,
                       @NonNull MainFragmentSetter mainFragmentSetter ) {
        mDrawerLayout = drawerLayout;
        mNavigationController = navigationController;
        mMainFragmentSetter = mainFragmentSetter;
    }

    @Override
    public boolean onNavigationItemSelected( @NonNull MenuItem item ) {
        // set item as selected to persist highlight
        item.setChecked(true);
        int itemId = item.getItemId();
        if(itemId == R.id.nav_search) {
            setMainFragment(mNavigationController.getFragment(ScreenType.BottomNavigation));
            mNavigationController.showDictionaryListInToolbar();
        }
        else if(itemId == R.id.nav_info) {
            setMainFragment(mNavigationController.getFragment(ScreenType.Information));
            changeToolbarLabel(item.getTitle());
        }
        else if(itemId == R.id.nav_catalog) {
            setMainFragment(mNavigationController.getFragment(ScreenType.Dictionaries));
            changeToolbarLabel(item.getTitle());
        }
        else if(itemId == R.id.nav_flashcards) {
            setMainFragment(mNavigationController.getFragment(ScreenType.Flashcards));
            changeToolbarLabel(item.getTitle());
        }
        else if(itemId == R.id.nav_settings) {
            setMainFragment(mNavigationController.getFragment(ScreenType.Settings));
            changeToolbarLabel(item.getTitle());
        }
        else if(itemId == R.id.nav_news) {
            changeToolbarLabel(item.getTitle());
            setMainFragment(mNavigationController.getFragment(ScreenType.NewsList));
        }
        else if(itemId == R.id.nav_report_problem) {
            EngineVersion engineVersion = mNavigationController.getEngineVersion();
            if(engineVersion != null)
                MailUtils.reportProblem(mDrawerLayout.getContext(), engineVersion.getVersion(), engineVersion.getBuild());
        }
        else
            return false;
        mDrawerLayout.closeDrawers();
        return true;
    }

    private void setMainFragment( @Nullable Fragment fragment ) {
        if(fragment != null)
            mMainFragmentSetter.setPendingMainFragment(fragment);
    }

    private void changeToolbarLabel( @NonNull final CharSequence newLabel ) {
        mNavigationController.showTitleInToolbar(newLabel.toString());
    }
}
