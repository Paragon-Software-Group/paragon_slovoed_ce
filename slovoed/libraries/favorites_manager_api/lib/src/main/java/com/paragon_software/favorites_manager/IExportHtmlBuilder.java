package com.paragon_software.favorites_manager;

import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.directory.Directory;

public interface IExportHtmlBuilder
{
    IExportHtmlBuilder addHeader(@NonNull String header );
    IExportHtmlBuilder addFooter(@NonNull  String footer);
    IExportHtmlBuilder addDirectory(@NonNull Directory<ArticleItem> directory );
    IExportHtmlBuilder addPartOfSpeechColor(@ColorInt int color);
    IExportHtmlBuilder addFavoritesSorter(@NonNull FavoritesSorterAPI favoritesSorter,
                                          @NonNull FavoritesSorting favoritesSorting );

    String createHTMLString();
}
