package com.paragon_software.history_manager;

import androidx.annotation.ColorInt;

import com.paragon_software.article_manager.ArticleItem;

import java.util.List;

public interface IExportHTMLBuilder
{
    IExportHTMLBuilder addHeader(String header);
    IExportHTMLBuilder addFooter(String footer);
    IExportHTMLBuilder addContent(List<ArticleItem> list);
    IExportHTMLBuilder addPartOfSpeechColor(@ColorInt int color);

    String createHTMLString();
}
