package com.paragon_software.favorites_manager;

import android.graphics.Typeface;
import android.os.Build;
import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;
import androidx.core.text.HtmlCompat;
import android.text.Html;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.text.style.StyleSpan;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.directory.Directory;

import java.util.ArrayList;
import java.util.List;

public class ExportHtmlBuilder implements IExportHtmlBuilder
{
    private static final String HTML_OPEN_TAG = "<html>";
    private static final String HTML_CLOSE_TAG = "</html>";

    private static final String HEAD_ENCODING_TAG = "<head><meta charset=\"UTF-8\"></head>";

    private static final String BODY_OPEN_TAG = "<body>";
    private static final String BODY_CLOSE_TAG = "</body>";

    private static final String FIRST_HEADER_OPEN_TAG = "<h1>";
    private static final String FIRST_HEADER_CLOSE_TAG = "</h1>";

    private static final String SECOND_HEADER_OPEN_TAG = "<h2>";
    private static final String SECOND_HEADER_CLOSE_TAG = "</h2>";

    private static final String PARAGRAPH_OPEN_TAG = "<p>";
    private static final String PARAGRAPH_CLOSE_TAG = "</p>";

    private static final String LIST_OPEN_TAG = "<ul style=\"list-style:none;list-style-type:none;\">";
    private static final String LIST_CLOSE_TAG = "</ul>";

    private static final String LIST_ITEM_OPEN_TAG = "<li>";
    private static final String LIST_ITEM_CLOSE_TAG = "</li>";

    private String mHeader;
    private String mFooter;
    private Directory<ArticleItem> mDirectory;

    private FavoritesSorterAPI mFavoritesSorter;
    private FavoritesSorting   mFavoritesSorting;

    @ColorInt
    private int mPartOfSpeechColor;

    @Override
    public IExportHtmlBuilder addHeader(@NonNull String header )
    {
        mHeader = header;
        return this;
    }

    @Override
    public IExportHtmlBuilder addFooter(@NonNull String footer )
    {
        mFooter = footer;
        return this;
    }

    @Override
    public IExportHtmlBuilder addDirectory(@NonNull Directory<ArticleItem> directory )
    {
        mDirectory = directory;
        return this;
    }

    @Override
    public IExportHtmlBuilder addPartOfSpeechColor(@ColorInt int color)
    {
        mPartOfSpeechColor = color;
        return this;
    }

    @Override
    public IExportHtmlBuilder addFavoritesSorter(@NonNull FavoritesSorterAPI favoritesSorter,
                                                 @NonNull FavoritesSorting favoritesSorting )
    {
        mFavoritesSorter = favoritesSorter;
        mFavoritesSorting = favoritesSorting;
        return this;
    }

    @Override
    public String createHTMLString()
    {
        return  HTML_OPEN_TAG +
                HEAD_ENCODING_TAG +
                BODY_OPEN_TAG +
                wrapTag(mHeader, FIRST_HEADER_OPEN_TAG, FIRST_HEADER_CLOSE_TAG) +
                createHtmlStringFromDirectory()  +
                wrapTag(mFooter, PARAGRAPH_OPEN_TAG, PARAGRAPH_CLOSE_TAG) +
                BODY_CLOSE_TAG +
                HTML_CLOSE_TAG;
    }

    @Override
    public @NonNull String toString() {
        String exportString;
        String htmlString = createHTMLString();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            exportString = Html.fromHtml(htmlString, HtmlCompat.FROM_HTML_MODE_LEGACY).toString();
        } else {
            exportString = Html.fromHtml(htmlString).toString();
        }
        return exportString;
    }

    private String createHtmlStringFromDirectory()
    {
        StringBuilder builder = new StringBuilder(LIST_OPEN_TAG);

        convertDirectoryToHTMLString(mDirectory, builder);

        return builder.append(LIST_CLOSE_TAG).toString();
    }

    private void convertDirectoryToHTMLString( Directory<ArticleItem> directory, StringBuilder builder )
    {
        if( directory.getChildList().isEmpty() )
        {
            wrapArticlesList(directory, builder);
        }
        else
        {
            List< Directory<ArticleItem> > directories = new ArrayList<>(directory.getChildList());
            List< Directory<ArticleItem> > sortDirectories = mFavoritesSorter.sortDirectories(directories, mFavoritesSorting);

            for ( Directory<ArticleItem> childDirectory : sortDirectories )
            {
                wrapDirectoryListItemTag(childDirectory, builder);
            }

            wrapArticlesList(directory, builder);
        }
    }

    private void wrapArticlesList( Directory<ArticleItem> directory, StringBuilder builder )
    {
        List< ArticleItem > articles = new ArrayList<>(directory.getItems());
        List< ArticleItem > sortedArticles = mFavoritesSorter.sort(articles, mFavoritesSorting);
        if( !sortedArticles.isEmpty() )
        {
            builder.append(LIST_OPEN_TAG);

            for (ArticleItem item : sortedArticles)
            {
                builder.append(LIST_ITEM_OPEN_TAG)
                        .append(wrapPartOfSpeechText(item))
                        .append(LIST_ITEM_CLOSE_TAG);
            }
            builder.append(LIST_CLOSE_TAG);
        }
    }

    private void wrapDirectoryListItemTag( Directory<ArticleItem> directory, StringBuilder builder )
    {
        if(directory.getParent() == null )
        {
            return;
        }

        builder.append(LIST_ITEM_OPEN_TAG);
        builder.append(LIST_OPEN_TAG);

        builder.append(wrapTag(directory.getName(), SECOND_HEADER_OPEN_TAG, SECOND_HEADER_CLOSE_TAG));

        convertDirectoryToHTMLString(directory, builder);

        builder.append(LIST_CLOSE_TAG);
        builder.append(LIST_ITEM_CLOSE_TAG);
    }

    private StringBuilder wrapPartOfSpeechText( ArticleItem articleItem )
    {
        StringBuilder builder = new StringBuilder();

        int partOfSpeechStartIndex = -1;
        int partOfSpeechEndIndex = -1;

        SpannableStringBuilder wordBuilder = new SpannableStringBuilder(articleItem.getShowVariantText());
        if( articleItem.getPartOfSpeech() != null )
        {
            wordBuilder.append(" ");

            partOfSpeechStartIndex = wordBuilder.length();
            wordBuilder.append(articleItem.getPartOfSpeech());
            partOfSpeechEndIndex = wordBuilder.length();
        }

        if(partOfSpeechStartIndex != -1 && partOfSpeechEndIndex != -1)
        {
            appendPartOfSpeechSpanToWord(wordBuilder, partOfSpeechStartIndex, partOfSpeechEndIndex);
        }

        builder
                .append(PARAGRAPH_OPEN_TAG)
                .append(Html.toHtml(wordBuilder))
                .append(PARAGRAPH_CLOSE_TAG);

        return builder;
    }

    private String wrapTag(String content, String openTag, String closeTag)
    {
        return openTag + content + closeTag;
    }

    private void appendPartOfSpeechSpanToWord(SpannableStringBuilder builder, int startIndex, int endIndex)
    {
        builder.setSpan(new ForegroundColorSpan(mPartOfSpeechColor), startIndex, endIndex, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        builder.setSpan(new StyleSpan(Typeface.ITALIC), startIndex, endIndex, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
}
