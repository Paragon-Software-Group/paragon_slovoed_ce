package com.paragon_software.engine;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.nativewrapper.ArticleSearcher;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.native_engine.HtmlBuilderParams;
import com.paragon_software.native_engine.SyncEngineAPI;

import java.lang.ref.Reference;
import java.lang.ref.SoftReference;

public class SyncSlovoedEngine implements SyncEngineAPI {
    @NonNull
    private Reference<NativeDictionary> mWeakEngine = new SoftReference<>(null);

    private final Context mApplicationContext;

    @NonNull
    private final DictionaryManagerAPI mDictionaryManager;

    public SyncSlovoedEngine(Context context, @NonNull DictionaryManagerAPI dictionaryManager) {
        mApplicationContext = context.getApplicationContext();
        mDictionaryManager = dictionaryManager;
    }

    @Nullable
    @Override
    public synchronized ArticleItem find(@NonNull Dictionary.DictionaryId dictionaryId, @NonNull Dictionary.Direction direction, @NonNull String word) {
        ArticleItem res = null;
        NativeDictionary dictionary = getDictionary(dictionaryId);
        if(dictionary != null) {
            ArticleSearcher searcher = new ArticleSearcher(dictionary);
            res = searcher.find(dictionaryId, word, direction.getLanguageFrom());
        }
        return res;
    }

    @Nullable
    @Override
    public ArticleItem find(@NonNull Dictionary.DictionaryId dictionaryId,
                            int listId,
                            int globalIdx,
                            @Nullable String anchor)
    {
        ArticleItem articleItem = null;
        for (Dictionary dictionary : mDictionaryManager.getDictionaries()) {
            if (dictionary.getId().equals(dictionaryId)) {
                NativeDictionary nativeDictionary = NativeDictionary.open(mApplicationContext, dictionary.getDictionaryLocation(), dictionary.getMorphoInfoList(), false);
                if (nativeDictionary != null) {
                    ArticleSearcher articleSearcher = new ArticleSearcher(nativeDictionary);
                    articleItem = articleSearcher.find(dictionaryId, listId, globalIdx, anchor);
                    nativeDictionary.close();
                    break;
                }
            }
        }
        return articleItem;
    }

    @Nullable
    @Override
    public synchronized String translate(@NonNull ArticleItem article, @NonNull HtmlBuilderParams htmlParams) {
        String res = null;
        NativeDictionary dictionary = getDictionary(article.getDictId());
        if(dictionary != null) {
            int listIdx = article.getListId();
            int localIdx = dictionary.resetList(listIdx, article.getGlobalIndex());
            if(localIdx >= 0)
                res = dictionary.translate(listIdx, localIdx, htmlParams);
        }
        return res;
    }

    @NonNull
    @Override
    public synchronized String[] getBaseForms(@NonNull Dictionary.DictionaryId dictionaryId, @NonNull Dictionary.Direction direction, @NonNull String word) {
        String[] res = new String[0];
        NativeDictionary dictionary = getDictionary(dictionaryId);
        if(dictionary != null)
            res = dictionary.getBaseForms(direction.getLanguageFrom(), word);
        return res;
    }

    @Nullable
    private NativeDictionary getDictionary(@NonNull Dictionary.DictionaryId dictionaryId) {
        NativeDictionary res = null;
        Dictionary dictionary = null;
        for(Dictionary d : mDictionaryManager.getDictionaries())
            if(dictionaryId.equals(d.getId())) {
                dictionary = d;
                break;
            }
        if(dictionary != null) {
            DictionaryLocation location = dictionary.getDictionaryLocation();
            NativeDictionary cached = mWeakEngine.get();
            if(cached != null)
                if(cached.getLocation().equals(location))
                    res = cached;
            if(res == null) {
                res = NativeDictionary.open(mApplicationContext, dictionary, true, false);
                if(res != null)
                    mWeakEngine = new SoftReference<>(res);
            }
        }
        return res;
    }
}
