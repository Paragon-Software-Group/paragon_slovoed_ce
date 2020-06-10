package com.paragon_software.navigation_manager;

import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.OpenFavoritesScreen;
import com.paragon_software.analytics_manager.events.OpenHistoryScreen;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.navigation_manager_ui.R;
import com.paragon_software.search_manager.SearchFragmentAPI;


public class SlovoedBottomNavigationFragment extends Fragment
		implements BottomNavigationView.OnNavigationItemSelectedListener,
		SearchFragmentAPI {

	private BottomNavigationView bottomNavigationView;
	private NavigationControllerApi mController;

	@Override
	public void onCreate(@Nullable Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mController = NavigationHolder.getNavigationManager().getController(NavigationControllerType.DEFAULT_CONTROLLER);
		DictionaryManagerHolder.getManager().loadOnlineDictionaryStatusInformation(this);
	}

	@Nullable
	@Override
	public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
							 @Nullable Bundle savedInstanceState) {
		View view = inflater.inflate(R.layout.fragment_slovoed_bottom_navigation, container, false);
		initializeBottomNavigation(view);
		return view;
	}

	@Override
	public void onResume() {
		super.onResume();
		replaceFragment(bottomNavigationView.getSelectedItemId());
	}

	private void initializeBottomNavigation(View fragmentView) {
		bottomNavigationView = fragmentView.findViewById(R.id.navigation);
		bottomNavigationView.setOnNavigationItemSelectedListener(this);
	}


	@Override
	public boolean onNavigationItemSelected(@NonNull MenuItem item) {
		logAnalyticsTabSelection(item.getItemId());
		return replaceFragment(item.getItemId());
	}

	private void logAnalyticsTabSelection(int itemId) {
		if (itemId == R.id.action_favorites) {
			AnalyticsManagerAPI.get().logEvent(new OpenFavoritesScreen());
		} else if (itemId == R.id.action_navigation) {
			AnalyticsManagerAPI.get().logEvent(new OpenHistoryScreen());
		}
	}

	@Override
	public void selectSearchNavigationItem() {
		bottomNavigationView.setSelectedItemId(R.id.action_search);
	}

	private boolean replaceFragment(int itemId) {
		if (itemId == R.id.action_favorites) {
			mController.historySelectionModeOff();
			replaceFragment(mController.getFragment(ScreenType.Favorites));
		} else if (itemId == R.id.action_navigation) {
			mController.favoritesSelectionModeOff();
			replaceFragment(mController.getFragment(ScreenType.History));
		} else {
			mController.historySelectionModeOff();
			mController.favoritesSelectionModeOff();
			mController.showDictionaryListInToolbar();
			replaceFragment(mController.getFragment(ScreenType.Search));
		}
		return true;
	}

	private void replaceFragment(@Nullable Fragment fragment) {
		if (fragment != null) {
			FragmentManager fragmentManager = getChildFragmentManager();
			FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
			String tag = fragment.getClass().toString();
			fragmentTransaction.replace(R.id.fragmentContainer, findFragment(fragmentManager, fragment, tag), tag);
			fragmentTransaction.commit();
		}
	}

	/**
	 * Trying to find existing fragment with specified tag in FragmentManager.
	 */
	private @Nullable Fragment findFragment( FragmentManager fragmentManager, @Nullable Fragment fragment, String tag )
	{
		// TODO Find out why FragmentManager contains new instances of previously added fragments after device orientation change
		// NOTE This method is needed to prevent app crashing when using SimpleDialog with nested fragment as target
		Fragment res = fragmentManager.findFragmentByTag(tag);
		if (res == null)
			res = fragment;
		return res;
	}
}
