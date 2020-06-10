package com.paragon_software.engine.rx.getadditional;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.ArticleItemFactory;
import com.paragon_software.engine.nativewrapper.ListInfo;
import com.paragon_software.engine.nativewrapper.ListType;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.EngineTaskRunner;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import io.reactivex.Observer;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Function;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

public class GetAdditional extends EngineTaskRunner<Params> {
    @NonNull
    private final Subject< Task<Params>> mSource = PublishSubject.create();

    @Nullable
    private Task<Params> mCurrentTask = null;

    @NonNull
    private WeakReference<ResultContainer> mWeakResultContainer = new WeakReference<>(null);

    public CollectionView<ArticleItem, Void> getAdditionalArticles(@Nullable Dictionary.DictionaryId dictionaryId) {
        assertInitCalled();
        ResultContainer resultContainer = mWeakResultContainer.get();
        if(resultContainer == null) {
            resultContainer = new ResultContainer();
            mWeakResultContainer = new WeakReference<>(resultContainer);
            subscribe(resultContainer.getObserver());
        }
        Dictionary dictionary = null;
        for(Dictionary d : dictionaryManager.getDictionaries())
            if(d.getId().equals(dictionaryId)) {
                dictionary = d;
                break;
            }
        emitNewTask(new Params(dictionary));
        return resultContainer;
    }

    private void subscribe(@NonNull Observer<Task<Result>> observer) {
        mSource.doOnNext(new SaveCurrentTaskAndNotifyResultContainer())
                .compose(new RunOn<Task<Params>>(RunOn.What.Observe, RunOn.Where.Computation, 200L))
                .map(new DoSearch())
                .compose(new RunOn<Task<Result>>(RunOn.What.Observe, RunOn.Where.Caller))
                .filter(new OnlyUncanceled())
                .doOnNext(new DiscardCurrentTask())
                .subscribe(observer);

    }

    @Override
    protected void emitTask(Task<Params> task) {
        mSource.onNext(task);
    }

    @Override
    protected Collection<Task<Params>> getCurrentTasks() {
        Collection<Task<Params>> res = Collections.emptySet();
        if(mCurrentTask != null)
            res = Collections.singleton(mCurrentTask);
        return res;
    }

    private class SaveCurrentTaskAndNotifyResultContainer implements Consumer<Task<Params>> {
        @Override
        public void accept(Task<Params> task ) {
            if(mCurrentTask != null)
                mCurrentTask.cancel();
            mCurrentTask = task;
            ResultContainer resultContainer = mWeakResultContainer.get();
            if(resultContainer != null)
                resultContainer.toggleProgress(true);
        }
    }

    //only handle first level of hierarchy
    private class DoSearch implements Function<Task<Params>, Task<Result>> {
        @Override
        public Task<Result> apply(Task<Params> task) {
            Params params = task.get();
            Result res = new Result(null);
            if(params.dictionary != null) {
                NativeDictionary dictionary = NativeDictionary.open(applicationContext, params.dictionary, false, false);
                if(dictionary != null) {
                    SparseArray<ListInfo> listInfo = dictionary.getLists(ListType.AdditionalInfo);
                    int n = listInfo.size();
                    List<List<ArticleItem>> lists = new ArrayList<>(n);
                    for(int i = 0; i < n; i++) {
                        int listIndex = listInfo.keyAt(i);
                        int m = dictionary.getListCurrentSize(listIndex);
                        List<ArticleItem> list = new ArrayList<>(m);
                        for (int j = 0; j < m; j++)
                            list.add(ArticleItemFactory.createNormal(params.dictionary.getId(), dictionary, listIndex, j));
                        lists.add(list);
                    }
                    res = new Result(lists);
                    dictionary.close();
                }
            }
            return task.spawn(res);
        }
    }

    private class DiscardCurrentTask implements Consumer<Task<Result>> {
        @Override
        public void accept(Task<Result> task ) {
            mCurrentTask = null;
        }
    }
}
