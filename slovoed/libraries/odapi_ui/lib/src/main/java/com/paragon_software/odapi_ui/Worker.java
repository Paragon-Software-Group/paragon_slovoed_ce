package com.paragon_software.odapi_ui;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.widget.Toast;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.filter.FilterTypeSimple;
import com.paragon_software.native_engine.SyncEngineAPI;
import com.paragon_software.splash_screen_manager.ISplashScreenManager;
import com.paragon_software.utils_slovoed_ui_common.activities.SnackbarActivity;

import java.io.Serializable;
import java.lang.ref.WeakReference;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;

abstract class Worker {
    static final String EXTRA_CLIENT = "client";
    static final String EXTRA_QUERY = "query";
    static final String EXTRA_VALUE = "value";

    static final String CONTROLLER_KEY = "ODAPI";

    private static final String SNACKBAR_CALLBACK_OPEN_ARTICLE = "com.paragon_software.odapi_ui.Worker.SNACKBAR_CALLBACK_OPEN_ARTICLE";
    private static final String SNACKBAR_CALLBACK_OPEN_CATALOG = "com.paragon_software.odapi_ui.Worker.SNACKBAR_CALLBACK_OPEN_CATALOG";
    private static final String EXTRA_ARTICLE_ITEM = "com.paragon_software.odapi_ui.Worker.EXTRA_ARTICLE_ITEM";
    private static final String EXTRA_DICTIONARY_ID = "com.paragon_software.odapi_ui.Worker.XTRA_DICTIONARY_ID";

    @NonNull
    private static volatile WeakReference<Context> mWeakContext = new WeakReference<>(null);

    private static volatile DictionaryManagerAPI mDictionaryManager;
    private static volatile SyncEngineAPI mEngine;
    private static volatile ISplashScreenManager mSplashscreenManager;
    private static ArticleManagerAPI mArticleManager;
    private static Class<? extends Activity> mActivityClass;

    static volatile Handler mHandler;

    static {
        SnackbarActivity.addListener(SNACKBAR_CALLBACK_OPEN_ARTICLE, new OpenArticleSnackbarCallback());
        SnackbarActivity.addListener(SNACKBAR_CALLBACK_OPEN_CATALOG, new OpenCatalogSnackbarCallback());
    }

    static void setContext(Context context) {
        mWeakContext = new WeakReference<>(context);
        mHandler = new Handler();
    }

    static void registerDictionaryManager(@NonNull DictionaryManagerAPI dictionaryManager) {
        mDictionaryManager = dictionaryManager;
    }

    static void registerEngine(@NonNull SyncEngineAPI engine){
        mEngine = engine;
    }

    static void registerSplashscreenManager(@NonNull ISplashScreenManager splashScreenManager) {
        mSplashscreenManager = splashScreenManager;
    }

    static void registerArticleManager(@NonNull ArticleManagerAPI articleManager) {
        mArticleManager = articleManager;
        if(mActivityClass != null)
            mArticleManager.registerArticleUI(CONTROLLER_KEY, mActivityClass);
    }

    static void registerArticleActivity(@NonNull Class<? extends Activity> activityClass) {
        mActivityClass = activityClass;
        if(mArticleManager != null)
            mArticleManager.registerArticleUI(CONTROLLER_KEY, mActivityClass);
    }

    @Nullable
    static Context getContext() {
        return mWeakContext.get();
    }

    @Nullable
    static DictionaryManagerAPI getDictionaryManager() {
        return mDictionaryManager;
    }

    @Nullable
    static DictionaryAndDirection getSelectedDictionaryAndDirection() {
        DictionaryAndDirection res = null;
        if(mDictionaryManager != null)
            res = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
        return res;
    }

    @NonNull
    static Collection<DictionaryAndDirection> getDictionariesAndDirections() {
        Collection<DictionaryAndDirection> res = Collections.emptySet();
        DictionaryControllerAPI dictionaryController = getDictionaryController();
        if(dictionaryController != null) {
            Dictionary[] allDictionaries = dictionaryController.getDictionaries();
            res = new HashSet<>(allDictionaries.length * 2);
            for(Dictionary d : allDictionaries) {
                Collection<Dictionary.Direction> directions = d.getDirections();
                if(directions != null)
                    for(Dictionary.Direction dir : directions)
                        if(dir != null)
                            res.add(new DictionaryAndDirection(d.getId(), dir));
            }
        }
        return res;
    }

    @Nullable
    static SyncEngineAPI getEngine() {
        return mEngine;
    }

    @Nullable
    static ArticleManagerAPI getArticleManager() {
        return mArticleManager;
    }

    static Result showOnNothingFound(Context context) {
        if((context != null) && (mHandler != null))
            mHandler.post(new ShowToast(context, context.getString(R.string.utils_slovoed_ui_common_no_result_found)));
        return new CommonErrorResult(CommonErrorResult.Kind.AppStateError, "Word not found.");
    }

    static Result openArticle(Context context, @NonNull Dictionary.DictionaryId notFoundId, @NonNull ArticleItem articleItem, boolean goodDirection) {
        String notFoundTitle = "";
        if(mDictionaryManager != null)
            for(Dictionary dictionary : mDictionaryManager.getDictionaries())
                if(notFoundId.equals(dictionary.getId())) {
                    notFoundTitle = dictionary.getTitle().get();
                    break;
                }
        if((context != null) && (mHandler != null)) {
            String title = context.getString(goodDirection ? R.string.odapi_ui_nothing_found_in_current_dir_good : R.string.odapi_ui_nothing_found_in_current_dir_bad, notFoundTitle);
            String action = context.getString(goodDirection ? R.string.odapi_ui_view_action_good : R.string.odapi_ui_view_action_bad);
            Bundle bundle = new Bundle(1);
            bundle.putSerializable(EXTRA_ARTICLE_ITEM, articleItem);
            mHandler.post(new ShowSnackbar(context, title, action, SNACKBAR_CALLBACK_OPEN_ARTICLE, bundle));
        }
        return new CommonErrorResult(CommonErrorResult.Kind.AppStateError, "Word not found.");
    }

    static Result openCatalog(Context context, @Nullable Dictionary.DictionaryId dictionaryId) {
        if((context != null) && (mHandler != null)) {
            Bundle bundle = null;
            if(dictionaryId != null) {
                bundle = new Bundle(1);
                bundle.putSerializable(EXTRA_DICTIONARY_ID, dictionaryId);
            }
            mHandler.post(new ShowSnackbar(context, context.getString(R.string.odapi_ui_no_dictionaries), context.getString(R.string.utils_slovoed_ui_common_download), SNACKBAR_CALLBACK_OPEN_CATALOG, bundle));
        }
        return new CommonErrorResult(CommonErrorResult.Kind.DatabaseIsUnavailable, "Can't open database");
    }

    @Nullable
    static Boolean checkLicense() {
        Boolean res = Boolean.TRUE;
        Context context = getContext();
        if((mSplashscreenManager != null) && (context != null) && (mHandler != null)) {
            if (mSplashscreenManager.haveNotShownScreens()) {
                mHandler.post(new RunSplashscreens());
                res = Boolean.FALSE;
            }
        }
        else
            res = null;
        return res;
    }

    @NonNull
    abstract Result getResult();

    private static volatile DictionaryControllerAPI __dictionaryController = null;
    @Nullable
   static DictionaryControllerAPI getDictionaryController() {
        if(__dictionaryController == null)
            if(mDictionaryManager != null) {
                __dictionaryController = mDictionaryManager.createController(CONTROLLER_KEY);
                if(__dictionaryController != null) {
                    DictionaryFilter filter = mDictionaryManager.getFilterFactory().createByType(FilterTypeSimple.SHARE_AND_API);
                    if(filter != null)
                        __dictionaryController.installFilter(filter);
                }
            }
        return __dictionaryController;
    }

    public abstract void trackAnalyticsEvent( @NonNull Message message );

    private static class OpenArticleSnackbarCallback implements SnackbarActivity.Listener {
        @Override
        public void onAction(Context context, @Nullable Bundle bundle) {
            if((mArticleManager != null) && (bundle != null)) {
                Serializable s = bundle.getSerializable(EXTRA_ARTICLE_ITEM);
                if(s instanceof ArticleItem) {
                    ArticleItem articleItem = (ArticleItem) s;
                    Dictionary.Direction direction = articleItem.getDirection();
                    if((mDictionaryManager != null) && (direction != null)) {
                        DictionaryAndDirection dnd = new DictionaryAndDirection(articleItem.getDictId(), direction);
                        mDictionaryManager.setDictionaryAndDirectionSelectedByUser(dnd);
                    }
                    mArticleManager.showArticleActivity(articleItem, CONTROLLER_KEY, context);
                }
            }
        }
    }

    private static class OpenCatalogSnackbarCallback implements SnackbarActivity.Listener {
        @Override
        public void onAction(Context context, @Nullable Bundle bundle) {
            if(mDictionaryManager != null) {
                Dictionary.DictionaryId id = null;
                if(bundle != null) {
                    Serializable s = bundle.getSerializable(EXTRA_DICTIONARY_ID);
                    if(s instanceof Dictionary.DictionaryId)
                        id = (Dictionary.DictionaryId) s;
                }
                if(id == null) {
                    Collection<Dictionary> dictionaries = mDictionaryManager.getDictionaries();
                    if (!dictionaries.isEmpty())
                        id = dictionaries.iterator().next().getId();
                }
                if(id != null)
                    mDictionaryManager.openMyDictionariesUI(context, id);
            }
        }
    }

    private static class ShowToast implements Runnable {
        @NonNull
        final WeakReference<Context> weakContext;

        @NonNull
        final String message;

        ShowToast(Context _context, @NonNull String _message) {
            weakContext = new WeakReference<>(_context);
            message = _message;
        }

        @Override
        public void run() {
            Context context = weakContext.get();
            if(context != null)
                Toast.makeText(context, message, Toast.LENGTH_LONG).show();
        }
    }

    private static class ShowSnackbar extends ShowToast {
        @NonNull
        private final String buttonText;

        @NonNull
        private final String listenerKey;

        @Nullable
        private final Bundle listenerBundle;

        ShowSnackbar(Context _context, @NonNull String _message, @NonNull String _buttonText, @NonNull String _listenerKey, @Nullable Bundle _listenerBundle) {
            super(_context, _message);
            buttonText = _buttonText;
            listenerKey = _listenerKey;
            listenerBundle = _listenerBundle;
        }

        @Override
        public void run() {
            Context context = weakContext.get();
            if(context != null)
                SnackbarActivity.show(context, message, buttonText, listenerKey, listenerBundle);
        }
    }

    private static class RunSplashscreens implements Runnable {
        @Override
        public void run() {
            Context context = mWeakContext.get();
            if((context != null) && (mSplashscreenManager != null)) {
                mSplashscreenManager.reset();
                mSplashscreenManager.showNext(context);
            }
        }
    }
}
