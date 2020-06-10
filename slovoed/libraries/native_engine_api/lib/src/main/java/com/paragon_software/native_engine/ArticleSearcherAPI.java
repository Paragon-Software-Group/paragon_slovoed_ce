package com.paragon_software.native_engine;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;

public interface ArticleSearcherAPI
{
    ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, @NonNull String next, int directionFrom );

    ArticleItem findLinkedWotDArticleItem(@NonNull Dictionary.DictionaryId dictionaryId, @NonNull String entryId, int directionFrom);
    ArticleItem findPractisePronunciationArticleItem(@NonNull Dictionary.DictionaryId dictionaryId, @NonNull String next, int directionFrom);

    Object[] getSoundInfoFromExternalKey(@NonNull String entryId, int directionFrom);

    ArticleItem find( @NonNull Dictionary.DictionaryId dictionaryId, int listId, int globalIdx, @Nullable String label );

    @Nullable
    ArticleItem findForSwipe( @NonNull ArticleItem currentArticleItem, boolean swipeForward );

	int[] getQueryHighlightData( int listIndex, @NonNull String articleText, @NonNull String headword, @NonNull String phrase, @NonNull String query );
}
