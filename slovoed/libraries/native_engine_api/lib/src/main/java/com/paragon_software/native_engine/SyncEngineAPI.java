package com.paragon_software.native_engine;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;

public interface SyncEngineAPI {
    @Nullable
    ArticleItem find(@NonNull Dictionary.DictionaryId dictionaryId, @NonNull Dictionary.Direction direction, @NonNull String word);

    @Nullable
    ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, int listId, int globalIdx, @Nullable String anchor );

    @Nullable
    String translate(@NonNull ArticleItem article, @NonNull HtmlBuilderParams htmlParams);

    @NonNull
    String[] getBaseForms(@NonNull Dictionary.DictionaryId dictionaryId, @NonNull Dictionary.Direction direction, @NonNull String word);
}
