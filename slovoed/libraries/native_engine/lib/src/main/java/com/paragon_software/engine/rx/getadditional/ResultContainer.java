package com.paragon_software.engine.rx.getadditional;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.engine.destructionqueue.ManagedResource;
import com.paragon_software.engine.destructionqueue.ResourceHolder;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.utils_slovoed.collections.CachedCollectionView;

import java.lang.ref.WeakReference;
import java.util.List;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

class ResultContainer extends CachedCollectionView<ArticleItem, Void> {
    @Nullable
    private ResourceHolder<ObserverImpl> mObserverHolder = null;

    Observer<Task<Result>> getObserver() {
        if(mObserverHolder == null)
            mObserverHolder = ResourceHolder.create(this, new ObserverImpl(this));
        return mObserverHolder.getResource();
    }

    private static class ObserverImpl implements Observer<Task<Result>>, ManagedResource {
        @NonNull
        private final WeakReference<ResultContainer> mWeakParent;

        private Disposable mDisposable;

        ObserverImpl(@NonNull ResultContainer parent) {
            mWeakParent = new WeakReference<>(parent);
        }

        @Override
        public void onSubscribe(Disposable d) {
            mDisposable = d;
        }

        @Override
        public void onNext(Task<Result> resultTask) {
            ResultContainer parent = mWeakParent.get();
            if(parent != null) {
                parent.resetCache();
                List<List<ArticleItem>> result = resultTask.get().items;
                for(List<ArticleItem> items : result)
                    parent.cacheItems(items);
                parent.toggleProgress(false);
            }
        }

        @Override
        public void onError(Throwable e) {

        }

        @Override
        public void onComplete() {

        }

        @Override
        public void free() {
            if(mDisposable != null)
                if(!mDisposable.isDisposed())
                    mDisposable.dispose();
        }
    }
}
