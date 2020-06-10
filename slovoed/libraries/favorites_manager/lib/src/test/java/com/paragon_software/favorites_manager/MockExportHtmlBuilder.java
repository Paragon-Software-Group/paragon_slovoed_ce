package com.paragon_software.favorites_manager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.directory.Directory;

import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;

import static org.junit.Assert.assertEquals;

public class MockExportHtmlBuilder implements IExportHtmlBuilder {
    final static String HEADER = "header";
    final static String CHOOSER_TITLE = "chooserTitle";
    final static String FOOTER = "footer";
    final static String SUBJECT = "subject";
    final static String EXPORT_FILE_DESCRIPTION = "exportFileDescription";
    @ColorInt
    final static int PART_OF_SPEECH_COLOR = 111;

    private static String ROOT_DIR = "root";

    @Override
    public IExportHtmlBuilder addHeader(@NonNull String header) {
        assertEquals(HEADER, header);
        return this;
    }

    @Override
    public IExportHtmlBuilder addFooter(@NonNull String footer) {
        assertEquals(FOOTER, footer);
        return this;
    }

    @Override
    public IExportHtmlBuilder addDirectory(@NonNull Directory<ArticleItem> directory) {
        assertEquals(ROOT_DIR, directory.getName());
        return this;
    }

    @Override
    public IExportHtmlBuilder addPartOfSpeechColor(int color) {
        assertEquals(PART_OF_SPEECH_COLOR, color);
        return this;
    }

    @Override
    public IExportHtmlBuilder addFavoritesSorter(@NonNull FavoritesSorterAPI favoritesSorter, @NonNull FavoritesSorting favoritesSorting) {
        return this;
    }

    @Override
    public String createHTMLString() {
        return "";
    }

    @NonNull
    @Override
    public String toString() {
        return "";
    }
}
