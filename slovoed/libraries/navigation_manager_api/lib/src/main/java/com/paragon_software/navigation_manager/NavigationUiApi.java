package com.paragon_software.navigation_manager;

import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.fragment.app.FragmentActivity;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.toolbar_manager.BaseParagonToolbarAPI;
import com.paragon_software.word_of_day.WotDItem;

public interface NavigationUiApi
{
    interface Factory {
        NavigationUiApi create(@NonNull String uiTag);
    }

    @NonNull
    View initNavigationView(@NonNull FragmentActivity activity);

    @Nullable
    BaseParagonToolbarAPI initToolbar(@NonNull AppCompatActivity activity, @Nullable Toolbar toolbar);

    void selectDefaultFragment(@NonNull FragmentActivity activity);

    void openMyDictionaries(@NonNull FragmentActivity activity, Dictionary.DictionaryId dictionaryId );

    void showDictionarySample(@NonNull FragmentActivity activity,
                              @Nullable Dictionary.DictionaryId dictionaryId,
                              @Nullable Dictionary.Direction direction,
                              @Nullable String searchText);

    void showDictionaryPreview(@NonNull FragmentActivity activity,
                               @Nullable Dictionary.DictionaryId dictionaryId,
                               @Nullable Dictionary.Direction direction );

    void openWotDItemScreen(@NonNull WotDItem wotDItem);

    void openNewsItemScreen(int newsIdToShow);

    void openDownloadManager(@NonNull FragmentActivity activity,
                             @Nullable Dictionary.DictionaryId dictionaryId);
    void openCatalogueAndRestorePurchase(@NonNull FragmentActivity activity);

    boolean onBackPressed(@NonNull FragmentActivity activity);

    boolean onCreateOptionsMenu( Menu menu, MenuInflater inflater );
    boolean onPrepareOptionsMenu( Menu menu );
    boolean onOptionsItemSelected( MenuItem item );
}
