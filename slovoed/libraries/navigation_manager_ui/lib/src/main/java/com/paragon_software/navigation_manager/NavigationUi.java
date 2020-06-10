package com.paragon_software.navigation_manager;

import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.util.Pair;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import com.google.android.material.navigation.NavigationView;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryFragmentAPI;
import com.paragon_software.navigation_manager_ui.R;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.search_manager.SearchController;
import com.paragon_software.search_manager.SearchFragmentAPI;
import com.paragon_software.toolbar_manager.BaseParagonToolbarAPI;
import com.paragon_software.word_of_day.WotDItem;

import java.lang.ref.WeakReference;

import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.CompositeDisposable;
import io.reactivex.functions.Consumer;
import io.reactivex.schedulers.Schedulers;

public class NavigationUi extends BaseNavigationUi implements LifecycleObserver
{
    public static class Factory implements NavigationUiApi.Factory {
        @Override
        public NavigationUiApi create(@NonNull String uiTag) {
            return new NavigationUi(uiTag);
        }
    }

    @Nullable
    private CompositeDisposable mCompositeDisposable;

    private NavigationView        navigationView;
    private DrawerLayout          mDrawerLayout;
    private LinearLayout          mContentFrame;
    private TextView              mNavigationBadge;

    @Nullable
    private WeakReference<FragmentActivity> mWeakActivity;

    @NonNull
    private final MainFragmentSetter mMainFragmentSetter = new MainFragmentSetter();

    private NavigationUi(@NonNull String tag) {
        super(tag);
    }

    @Override
    public void openNewsItemScreen(int newsIdToShow) {

    }

    @NonNull
    @Override
    public View initNavigationView(@NonNull FragmentActivity activity) {
        activity.setTheme(R.style.SlovoedAppTheme);
        activity.setContentView(R.layout.activity_main);
        activity.getLifecycle().addObserver(this);
        if ( mCompositeDisposable != null )
        {
            mCompositeDisposable.clear();
        }
        mCompositeDisposable = new CompositeDisposable();
        mWeakActivity = new WeakReference<>(activity);
        navigationView = activity.findViewById(R.id.nav_view);
        mDrawerLayout = activity.findViewById(R.id.drawer_layout);
        mContentFrame = activity.findViewById(R.id.content_frame);
        mNavigationBadge = navigationView.getMenu().findItem(R.id.nav_news).getActionView().findViewById(R.id.notifications_badge);
        mDrawerLayout.addDrawerListener(new DrawerListener(activity, mMainFragmentSetter));

        NavigationView.OnNavigationItemSelectedListener navigationListener =
                new NavigationListener(mDrawerLayout, mNavigationController, mMainFragmentSetter);
        navigationView.setNavigationItemSelectedListener(navigationListener);
        NavigationView navigationViewFooter = activity.findViewById(R.id.footer_nav_view);
        navigationViewFooter.setNavigationItemSelectedListener(navigationListener);

        initControllerListeners();

        return mDrawerLayout;
    }

    @Nullable
    @Override
    public BaseParagonToolbarAPI initToolbar(@NonNull AppCompatActivity activity, @Nullable Toolbar toolbar)
    {
        BaseParagonToolbarAPI res = null;
        if(toolbar != null) {
            mContentFrame.addView(toolbar, 0);
            if (toolbar instanceof BaseParagonToolbarAPI)
                res = (BaseParagonToolbarAPI) toolbar;
            activity.setSupportActionBar(toolbar);
            if (null != activity.getSupportActionBar()) {
                ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(activity, mDrawerLayout, toolbar,
                                                                         android.R.string.ok, android.R.string.cancel);
                mDrawerLayout.addDrawerListener(toggle);
                toggle.syncState();
                if (res != null) {
                    res.setDrawerLayout(mDrawerLayout);
                    res.setToggle(toggle);
                }
            }
            if (res != null)
                res.setActionbar(activity.getSupportActionBar());
        }
        return res;
    }

    @Override
    public void selectDefaultFragment(@NonNull FragmentActivity activity) {
        MainFragmentSetter.replaceMainFragment(activity.getSupportFragmentManager(), mNavigationController.getFragment(
            ScreenType.BottomNavigation));
        navigationView.setCheckedItem(R.id.nav_search);
    }

    @Override
    public void openMyDictionaries(@NonNull FragmentActivity activity, final Dictionary.DictionaryId dictionaryId ) {
        final Fragment dictionaryFragment = mNavigationController.getFragment(ScreenType.Dictionaries);
        if(dictionaryFragment != null) {
            final WeakReference<FragmentActivity> weakActivity = new WeakReference<>(activity);
            new Handler().post(new Runnable() {
                @Override
                public void run() {
                    FragmentActivity a = weakActivity.get();
                    if (a != null) {
                        MainFragmentSetter.replaceMainFragment(a.getSupportFragmentManager(), dictionaryFragment);
                        navigationView.setCheckedItem(R.id.nav_catalog);
                        mNavigationController.showTitleInToolbar(a.getString(R.string.navigation_manager_ui_nav_bar_menu_item_catalog));
                        new Handler().post(new Runnable() {
                            @Override
                            public void run() {
                                if (dictionaryFragment instanceof DictionaryFragmentAPI) {
                                    ((DictionaryFragmentAPI) dictionaryFragment).selectMyDictionariesTab(dictionaryId);
                                }
                            }
                        });
                    }
                }
            });
        }
    }

    @Override
    public void showDictionarySample(@NonNull FragmentActivity activity,
                                     @Nullable final Dictionary.DictionaryId dictionaryId,
                                     @Nullable final Dictionary.Direction direction,
                                     @Nullable final String searchText) {
        final Fragment searchFragment = mNavigationController.getFragment(ScreenType.BottomNavigation);
        if (searchFragment != null) {
            if (!TextUtils.isEmpty(searchText))
                mNavigationController.setSearchText(SearchController.CONTROLLER_TYPE_DEFAULT, searchText);
            final WeakReference<FragmentActivity> weakActivity = new WeakReference<>(activity);
            new Handler().post(new Runnable() {
                @Override
                public void run() {
                    FragmentActivity a = weakActivity.get();
                    if (a != null) {
                        mNavigationController.showDictionaryListInToolbar(dictionaryId, direction);
                        MainFragmentSetter.replaceMainFragment(a.getSupportFragmentManager(), searchFragment);
                        navigationView.setCheckedItem(R.id.nav_search);
                        new Handler().post(new Runnable() {
                            @Override
                            public void run() {
                                if (searchFragment instanceof SearchFragmentAPI) {
                                    ((SearchFragmentAPI) searchFragment).selectSearchNavigationItem();
                                }
                            }
                        });
                    }
                }
            });
        }
    }

    @Override
    public void showDictionaryPreview(@NonNull FragmentActivity activity,
                                      @Nullable Dictionary.DictionaryId dictionaryId,
                                      @Nullable Dictionary.Direction direction) {
        this.showDictionarySample(activity, dictionaryId, direction, null);
    }

    @Override
    public void openWotDItemScreen(@NonNull WotDItem wotDItem) {

    }

    @Override
    public void openDownloadManager(@NonNull FragmentActivity activity, final Dictionary.DictionaryId dictionaryId) {
        openMyDictionaries(activity, dictionaryId);
    }

    @Override
    public void openCatalogueAndRestorePurchase(@NonNull FragmentActivity activity) { }

    @Override
    public boolean onCreateOptionsMenu( Menu menu, MenuInflater inflater )
    {
        return false;
    }

    @Override
    public boolean onPrepareOptionsMenu( Menu menu )
    {
        return false;
    }

    @Override
    public boolean onOptionsItemSelected( MenuItem item )
    {
        return false;
    }

    @OnLifecycleEvent( Lifecycle.Event.ON_DESTROY )
    private void onDestroy()
    {
        if ( mCompositeDisposable != null )
        {
            mCompositeDisposable.clear();
        }

        if( mWeakActivity != null )
        {
            FragmentActivity mainActivity = mWeakActivity.get();
            if ( mainActivity != null )
            {
                mainActivity.getLifecycle().removeObserver(this);
            }
        }

    }

    private void initControllerListeners()
    {
        if( mCompositeDisposable != null )
        {
            mCompositeDisposable.add(
                    mNavigationController.getUnreadNewsCount()
                            .subscribeOn(Schedulers.io())
                            .observeOn(AndroidSchedulers.mainThread())
                            .subscribe(new Consumer<Integer>()
                            {
                                @Override
                                public void accept(Integer count)
                                {
                                    updateNewsBadge(count);
                                }
                            }, new Consumer<Throwable>()
                            {
                                @Override
                                public void accept(Throwable throwable)
                                {
                                    throwable.printStackTrace();
                                }
                            }));
        }
    }

    private void updateNewsBadge(int unreadCount) {
        if (unreadCount != 0) {
            mNavigationBadge.setVisibility(View.VISIBLE);
            mNavigationBadge.setText(String.valueOf(unreadCount));
        } else
            mNavigationBadge.setVisibility(View.GONE);
    }

    @Override
    protected void openScreen(Pair<ScreenType, Bundle> screenTypePair) {
//        Do nothing
    }

    @Override
    protected void openScreen(@NonNull ScreenType screenType, FragmentActivity mainActivity, Fragment newFragment, Class newActivityClass, @Nullable Bundle bundle) {
//        Do nothing
    }
}
