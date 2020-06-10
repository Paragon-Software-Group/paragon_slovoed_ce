package com.paragon_software.navigation_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ShowArticleOptions;

import java.util.List;

public class StandardTabletScreenOpener extends StandardScreenOpener {
    StandardTabletScreenOpener(NavigationManager navigationManager, String articleControllerId) {
        super(navigationManager, articleControllerId);
    }

    @Override
    public boolean showArticle(@Nullable ArticleItem article, @Nullable ShowArticleOptions showArticleOptions,
                               Context context) {
        if (mNavigationManager.getArticleManager() != null) {
            return mNavigationManager.getArticleManager()
                    .showArticle(article, showArticleOptions, mArticleControllerId, context);
        }
        return false;
    }

    @Override
    public boolean showArticleFromSeparateList(@NonNull List<ArticleItem> separateArticles, int currentArticleIndex, Context context) {
        if (mNavigationManager.getArticleManager() != null) {
            return mNavigationManager.getArticleManager()
                    .showArticleFromSeparateList(separateArticles, currentArticleIndex, mArticleControllerId, context);
        }
        return false;
    }
}
