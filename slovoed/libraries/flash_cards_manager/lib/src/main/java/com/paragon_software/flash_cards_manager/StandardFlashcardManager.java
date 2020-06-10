package com.paragon_software.flash_cards_manager;

import android.content.Context;
import android.net.Uri;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.utils_slovoed.collections.ArrayCollectionView;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Callable;

import io.reactivex.Completable;
import io.reactivex.Observable;
import io.reactivex.Single;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.functions.Consumer;
import io.reactivex.observers.DefaultObserver;
import io.reactivex.schedulers.Schedulers;

import static com.paragon_software.flash_cards_manager.FilterFlashcards.IsFlashcardCheckedAndForFullDictionary;
import static com.paragon_software.flash_cards_manager.FilterFlashcards.IsFlashcardForFullDictionary;

public class StandardFlashcardManager extends FlashcardManagerAPI
                                      implements DictionaryManagerAPI.IDictionaryListObserver
{
    private static final String FLASHCARDS_SORT_MODE_KEY = "com.paragon_software.flash_cards_manager.FLASHCARDS_SAVE_KEY";

    @Nullable
    private FlashcardExchangerAPI mFlashcardExchanger;

    @Nullable
    private ScreenOpenerAPI mScreenOpener;

    @Nullable
    private EngineSerializerAPI mEngineSerializer;

    @Nullable
    private SettingsManagerAPI mSettingsManager;

    @Nullable
    private DictionaryManagerAPI mDictionaryManager;

    @Nullable
    private SoundManagerAPI mSoundManager;

    @Nullable
    private ArticleFragmentFactory mArticleFragmentFactory;

    @NonNull
    private final Set< Notifier > mNotifiers = new HashSet<>();

    @Nullable
    private ArrayCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> mFlashcards;

    @Nullable
    private FlashcardPersistLoader                                     mFlashcardPersistLoader;

    @Nullable
    private Observable< ArrayList< FlashcardBox > > loadObservable;

    @NonNull
    private CollectionView.OnItemRangeChanged mPersistSaver = new PersistSaver();

    @NonNull
    private Map< Dictionary.DictionaryId, AbstractFlashcardCollectionView > flashcardCollectionViewMap =
        new HashMap<>();

    @NonNull
    private Map< Dictionary.DictionaryId, AbstractQuizCollectionView > quizCollectionViewMap = new HashMap<>();

    private boolean mIsImportRunning;
    private boolean mIsExportRunning;

    public StandardFlashcardManager() {
        FlashcardManagerHolder.set(this);
    }

    @Override
    public void registerFlashcardExchanger(@NonNull FlashcardExchangerAPI flashcardExchanger) {
        mFlashcardExchanger = flashcardExchanger;
    }

    @Override
    public void registerScreenOpener(@NonNull ScreenOpenerAPI screenOpener) {
        mScreenOpener = screenOpener;
    }

    @Override
    public void registerEngineSerializer(@NonNull EngineSerializerAPI engineSerializer) {
        mEngineSerializer = engineSerializer;
        initFlashcardPersistLoader();
    }

    @Override
    public void registerSettingsManager(@NonNull SettingsManagerAPI settingsManager) {
        mSettingsManager = settingsManager;
        initFlashcardPersistLoader();
    }

    @Override
    public void registerDictionaryManager(@NonNull DictionaryManagerAPI dictionaryManager) {
        mDictionaryManager = dictionaryManager;
        mDictionaryManager.registerDictionaryListObserver(this);
    }

    @Override
    public void registerSoundManager(@NonNull SoundManagerAPI soundManager) {
        mSoundManager = soundManager;
    }

    @Override
    public void registerArticleFragmentFactory(@NonNull ArticleFragmentFactory factory) {
        mArticleFragmentFactory = factory;
    }

    private void initFlashcardPersistLoader()
    {
        if ( null != mEngineSerializer && null != mSettingsManager && null == mFlashcardPersistLoader )
        {
            mFlashcardPersistLoader =
                new FlashcardPersistLoader(new FlashcardPersistConverter(mEngineSerializer), mSettingsManager);
            loadFlashcardsFromPersist();
        }
    }

    private void loadFlashcardsFromPersist()
    {
        if ( null != mDictionaryManager && mDictionaryManager.getDictionaries().size() > 0 && null != mFlashcardPersistLoader )
        {
            if ( null == loadObservable )
            {
                loadObservable = Observable.fromCallable(
                    new Callable< ArrayList< FlashcardBox > >()
                    {
                        @Override
                        public ArrayList< FlashcardBox > call() throws Exception
                        {
                            return mFlashcardPersistLoader.load();
                        }
                    });
            }
            if ( null != loadObservable )
            {
                loadObservable.subscribeOn(Schedulers.computation())
                        .observeOn(AndroidSchedulers.mainThread())
                        .subscribe(new OnFlashcardsLoadedObserver());
            }
        }
    }

    @Override
    public void onDictionaryListChanged()
    {
        loadFlashcardsFromPersist();
    }

    @Override
    public void addWord( @NonNull Context context, @NonNull ArticleItem word ) {
        if((mFlashcards != null) && (indexOf(word) < 0)) {
            mFlashcards.add(FlashcardBox.create(word));
            notifyCardsChanged();
        }
    }

    @Override
    public void removeWord( @NonNull Context context, @NonNull ArticleItem word ) {
        int idx = indexOf(word);
        if((mFlashcards != null) && (idx >= 0)) {
            mFlashcards.remove(idx);
            notifyCardsChanged();
        }
    }

    @Override
    public boolean canAddWord( @NonNull ArticleItem word ) {
        return (mFlashcards != null) && (indexOf(word) < 0);
    }

    @Override
    public boolean canRemoveWord( @NonNull ArticleItem word ) {
        return indexOf(word) >= 0;
    }

    private int indexOf( @NonNull ArticleItem word )
    {
        int index = Integer.MIN_VALUE;
        for ( int i = 0 ; null != mFlashcards && i < mFlashcards.getCount() ; i++ )
        {
            FlashcardBox item = mFlashcards.getItem(i);
            if ( word.equals(item.getParent()) )
            {
                index = i;
                break;
            }
        }
        return index;
    }

    private void notifyCardsChanged()
    {
        for ( Notifier notifier : mNotifiers )
        {
            if ( notifier instanceof CardsChangedListener )
            {
                ( (CardsChangedListener) notifier ).onCardsChanged();
            }
        }
    }

    @Override
    public void registerNotifier(@NonNull Notifier notifier) {
        mNotifiers.add(notifier);
    }

    @Override
    public void unregisterNotifier(@NonNull Notifier notifier) {
        mNotifiers.remove(notifier);
    }

    @NonNull
    @Override
    AbstractFlashcardCollectionView getFlashcardCollectionView()
    {
        Dictionary.DictionaryId dictionaryId = getDictionaryId();
        if ( null == dictionaryId )
        {
            return new EmptyFlashcardCollectionView();
        }
        else
        {
            return getFlashcardCollectionView(dictionaryId);
        }
    }

    @NonNull
    @Override
    AbstractFlashcardCollectionView getFlashcardCollectionView( @NonNull Dictionary.DictionaryId dictionaryId )
    {
        AbstractFlashcardCollectionView cv = flashcardCollectionViewMap.get(dictionaryId);
        if ( null == cv )
        {
            IsFlashcardForFullDictionary predicate = new IsFlashcardForFullDictionary(dictionaryId);
            flashcardCollectionViewMap.put(dictionaryId, cv = new FlashcardCollectionView(
                new TransformFlashcardDataToView(new FilterFlashcards(mFlashcards, predicate), loadSortMode())));
        }
        return cv;
    }

    @Nullable
    private Dictionary.DictionaryId getDictionaryId()
    {
        Dictionary.DictionaryId dictionaryId;
        if ( null != mDictionaryManager )
        {
            DictionaryAndDirection dnd = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
            if ( null != dnd )
            {
                dictionaryId = dnd.getDictionaryId();
            }
            else
            {
                Iterator< Dictionary > iterator = mDictionaryManager.getDictionaries().iterator();
                dictionaryId = iterator.hasNext() ? iterator.next().getId() : null;
            }
        }
        else
        {
            throw new IllegalStateException("Flashcard manager does not initialized properly. Dictionary manager is null");
        }
        return dictionaryId;
    }

    @Override
    public void randomizeQuiz() {
        Dictionary.DictionaryId dictionaryId = getDictionaryId();
        if(dictionaryId != null) {
            IsFlashcardCheckedAndForFullDictionary predicate = new IsFlashcardCheckedAndForFullDictionary(dictionaryId);
            RandomizeCardsCv randomCardsCv = new RandomizeCardsCv(new FilterFlashcards(mFlashcards, predicate));
            quizCollectionViewMap.put(dictionaryId, new TransformFlashcardDataToQuizView(mSoundManager, randomCardsCv));
        }
    }

    @NonNull
    @Override
    AbstractQuizCollectionView getQuizCollectionView()
    {
        Dictionary.DictionaryId dictionaryId = getDictionaryId();
        if ( null == dictionaryId )
        {
            return new EmptyQuizCollectionView();
        }
        else
        {
            return getQuizCollectionView(dictionaryId);
        }
    }

    @NonNull
    @Override
    AbstractQuizCollectionView getQuizCollectionView( @NonNull Dictionary.DictionaryId dictionaryId )
    {
        if(!quizCollectionViewMap.containsKey(dictionaryId))
            randomizeQuiz();
        return quizCollectionViewMap.get(dictionaryId);
    }

    @Override
    @Nullable
    ArticleFragmentFactory getArticleFragmentFactory()
    {
        return mArticleFragmentFactory;
    }

    @Override
    void showQuizArticle( @NonNull ArticleItem... articleItems )
    {
        if ( null != mScreenOpener && articleItems.length > 0 )
        {
            mScreenOpener.showArticleActivity(articleItems[0], null);
        }
    }

    @Override
    public Single<File> exportFlashcards()
    {
        if(mIsExportRunning)
        {
            return Single.error(new IllegalStateException("Export already running"));
        }

        if(mFlashcardExchanger == null
                || mDictionaryManager == null
                || mDictionaryManager.getDictionaryAndDirectionSelectedByUser() == null)
        {
            return Single.error(new IllegalStateException("FlashcardExchanger|DictionaryManager is null"));
        }

        if(mFlashcards == null || mFlashcards.getCount() == 0)
        {
            return Single.error(new IllegalStateException("mFlashcards is null or empty"));
        }

        mIsExportRunning = true;

        List<FlashcardBox> flashcardBoxes = new ArrayList<>(mFlashcards.getCount());
        for(int i = 0; i < mFlashcards.getCount(); i++)
        {
            flashcardBoxes.add(mFlashcards.getItem(i));
        }

        return mFlashcardExchanger.getExportFlashcardsFile(
                mDictionaryManager.getDictionaryAndDirectionSelectedByUser().getDictionaryId().toString(),
                flashcardBoxes)
                .doOnSuccess(new Consumer<File>()
                {
                    @Override
                    public void accept(File file)
                    {
                        mIsExportRunning = false;
                    }
                })
                .doOnError(new Consumer<Throwable>()
                {
                    @Override
                    public void accept(Throwable throwable)
                    {
                        mIsExportRunning = false;
                    }
                });
    }

    @Override
    public Completable importFlashcards(File file)
    {
        if( mFlashcardPersistLoader == null )
        {
            return Completable.error(new IllegalStateException("FlashcardPersistLoader is null"));
        }

        if( mFlashcardExchanger == null )
        {
            return Completable.error(new IllegalStateException("FlashcardExchanger is null"));
        }

        if( mIsImportRunning )
        {
            return Completable.error(new IllegalStateException("Import already running") );
        }

        mIsImportRunning = true;
        return mFlashcardExchanger.importFlashcards(Uri.fromFile(file))
                .doOnSuccess(new Consumer<List<FlashcardBox>>()
                {
                    @Override
                    public void accept(List<FlashcardBox> flashcardBoxes)
                    {
                        mIsImportRunning = false;
                        saveFlashcardBoxes(flashcardBoxes);
                        mFlashcardPersistLoader.save(mFlashcards);
                    }
                })
                .doOnError(new Consumer<Throwable>()
                {
                    @Override
                    public void accept(Throwable throwable)
                    {
                        mIsImportRunning = false;
                    }
                })
                .ignoreElement();
    }

    private FlashcardMetadata.SortMode loadSortMode() {
        int n = 0;
        if(mSettingsManager != null)
            try {
                n = mSettingsManager.load(FLASHCARDS_SORT_MODE_KEY, 0);
            }
            catch (Exception ignore) { }
        return FlashcardMetadata.SortMode.values()[n];
    }

    void saveSortMode(@NonNull FlashcardMetadata.SortMode sortMode) {
        int n = Arrays.binarySearch(FlashcardMetadata.SortMode.values(), sortMode);
        if(mSettingsManager != null)
            try {
                mSettingsManager.save(FLASHCARDS_SORT_MODE_KEY, n, true);
            }
            catch (Exception ignore) {}
    }

    private void saveFlashcardBoxes( List< FlashcardBox > flashcardBoxes )
    {
        if (null == mFlashcards )
        {
            mFlashcards = new ArrayCollectionView<>();
        }
        mFlashcards.unregisterListener(mPersistSaver);
        mFlashcards.clear();
        mFlashcards.add(flashcardBoxes);
        mFlashcards.registerListener(mPersistSaver);
    }

    class OnFlashcardsLoadedObserver extends DefaultObserver< ArrayList< FlashcardBox > >
    {
        @Override
        public void onNext( ArrayList< FlashcardBox > result )
        {
            saveFlashcardBoxes(result);
            notifyCardsChanged();
        }

        @Override
        public void onError( Throwable e )
        {
            e.printStackTrace();
        }

        @Override
        public void onComplete()
        {
        }
    }

    private class PersistSaver implements CollectionView.OnItemRangeChanged
    {
        @Override
        public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int startPosition, int itemCount )
        {
            if ( null != mFlashcards && null != mFlashcardPersistLoader )
            {
                mFlashcardPersistLoader.save(mFlashcards);
            }
        }
    }
}
