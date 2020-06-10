package com.paragon_software.flash_cards_manager;

import android.os.Bundle;
import android.os.Handler;
import androidx.annotation.NonNull;
import com.google.android.material.tabs.TabLayout;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.RecyclerView;
import androidx.appcompat.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import com.paragon_software.flash_cards_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;

public class FcSelectActivity extends AppCompatActivity
                              implements FlashCardListAdapter.LongClickListener,
                                         CollectionView.OnItemRangeChanged,
                                         CollectionView.OnMetadataChanged,
                                         TabLayout.OnTabSelectedListener {
    private FlashCardListAdapter mAdapter;
    private Toolbar mNormalToolbar, mDeleteToolbar;
    private Menu mMenu;
    RecyclerView mRecyclerView;
    TabLayout mSortTabs;

    @NonNull
    private final AbstractFlashcardCollectionView mCollection =
            FlashcardManagerHolder.get().getFlashcardCollectionView();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_fc_select);
        mRecyclerView = findViewById(R.id.card_list);
        mAdapter = new FlashCardListAdapter(this);
        mAdapter.setData(mCollection);
        mRecyclerView.setAdapter(mAdapter);
        mNormalToolbar = findViewById(R.id.toolbar_normal);
        mDeleteToolbar = findViewById(R.id.toolbar_delete);
        mSortTabs = findViewById(R.id.sort_tabs);
        TabLayout.Tab tab = mSortTabs.getTabAt((((FlashcardMetadata) mCollection.getMetadata())
                .sortMode == FlashcardMetadata.SortMode.Alphabetical) ? 0 : 1);
        if(tab != null)
            tab.select();
        mSortTabs.addOnTabSelectedListener(this);
        mCollection.registerListener(this);
        updateToolbar();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mCollection.unregisterListener(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        getMenuInflater().inflate(R.menu.fc_select, menu);
        FlashcardMetadata metadata = getMetadata();
        metadata.deleteButtonVisibility.applyTo(menu, R.id.delete);
        menu.findItem(R.id.select_all1).setVisible(
                FlashcardMetadata.Mode.Selection.equals(metadata.mode));
        menu.findItem(R.id.select_all2).setVisible(
                FlashcardMetadata.Mode.Regular.equals(metadata.mode));
        mMenu = menu;
        updateSelectionCaption();
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        boolean res = false;
        if(id == android.R.id.home) {
            onBackPressed();
            res = true;
        }
        else if((id == R.id.select_all1) || (id == R.id.select_all2)) {
            boolean allSelected = mCollection.getCheckedCount() >= mCollection.getCount();
            mCollection.checkAll(!allSelected);
            res = true;
        }
        else if(id == R.id.delete) {
            for(int i = mCollection.getCount() - 1;
                (i >= 0) && FlashcardMetadata.Mode.Selection.equals(( getMetadata() ).mode);
                i--)
                if (mCollection.getItem(i).checked)
                    mCollection.remove(i);
            res = true;
        }
        return res;
    }

    @Override
    public boolean onLongClick(int position) {
        boolean res = false;
        if(FlashcardMetadata.Mode.Regular.equals(( getMetadata() ).mode)) {
            mCollection.activateSelectionMode(position);
            res = true;
        }
        return res;
    }

    @Override
    public void onBackPressed() {
        if(FlashcardMetadata.Mode.Selection.equals(( getMetadata() ).mode))
            mCollection.activateRegularMode();
        else
            super.onBackPressed();
    }

    @Override
    public void onItemRangeChanged(CollectionView.OPERATION_TYPE type, int startPosition, int itemCount) {
        if(FlashcardMetadata.Mode.Selection.equals(( getMetadata() ).mode)) {
            int n = mCollection.getCheckedCount();
            if(n > 0)
                mDeleteToolbar.setTitle(getString(R.string.flash_cards_manager_ui_delete_count, n));
            else
                scheduleRegularMode();
        }
        updateSelectionCaption();
        if(mCollection.getCount() == 0)
            finish();
    }

    @Override
    public void onMetadataChanged() {
        updateToolbar();
    }

    private void updateToolbar() {
        Toolbar toolbarToShow = null, toolbarToHide = null;
        switch ( getMetadata().mode) {
            case Regular:
                toolbarToShow = mNormalToolbar;
                toolbarToHide = mDeleteToolbar;
                break;
            case Selection:
                toolbarToShow = mDeleteToolbar;
                toolbarToHide = mNormalToolbar;
                break;
        }
        toolbarToShow.setVisibility(View.VISIBLE);
        setSupportActionBar(toolbarToShow);
        ActionBar actionBar = getSupportActionBar();
        if(actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setDisplayShowTitleEnabled(true);
        }
        toolbarToHide.setVisibility(View.GONE);
    }

    private FlashcardMetadata getMetadata()
    {
        return (FlashcardMetadata) mCollection.getMetadata();
    }

    private void updateSelectionCaption() {
        boolean canSelectMore = mCollection.getCheckedCount() < mCollection.getCount();
        for(int menuId : new int[] {R.id.select_all1, R.id.select_all2})
            mMenu.findItem(menuId).setTitle(canSelectMore ?
                                                    R.string.flash_cards_manager_ui_select_all :
                                                    R.string.flash_cards_manager_ui_deselect_all);
    }

    @Override
    public void onTabSelected(TabLayout.Tab tab) {
        mCollection.setSortMode((tab.getPosition() == 0) ?
                                        FlashcardMetadata.SortMode.Alphabetical :
                                        FlashcardMetadata.SortMode.ByProgress);
    }

    @Override
    public void onTabUnselected(TabLayout.Tab tab) { }

    @Override
    public void onTabReselected(TabLayout.Tab tab) { }

    private void scheduleRegularMode() {
        Handler handler = new Handler();
        handler.post(new Runnable() {
            @Override
            public void run() {
                mCollection.activateRegularMode();
            }
        });
    }
}
