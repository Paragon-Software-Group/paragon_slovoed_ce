package com.paragon_software.history_manager;

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

import java.util.List;

public class ExportHtmlBuilder implements IExportHTMLBuilder
{
    private static final String HTML_OPEN_TAG = "<html>";
    private static final String HTML_CLOSE_TAG = "</html>";

    private static final String HEAD_ENCODING_TAG = "<head><meta charset=\"UTF-8\"></head>";

    private static final String BODY_OPEN_TAG = "<body>";
    private static final String BODY_CLOSE_TAG = "</body>";

    private static final String PARAGRAPH_OPEN_TAG = "<p>";
    private static final String PARAGRAPH_CLOSE_TAG = "</p>";

    protected String header;
    protected String footer;
    private List<ArticleItem> list;

    @ColorInt
    private int partOfSpeechColor;

    @Override
    public IExportHTMLBuilder addHeader(String header)
    {
        this.header = header;
        return this;
    }

    @Override
    public IExportHTMLBuilder addFooter(String footer)
    {
        this.footer = footer;
        return this;
    }

    @Override
    public IExportHTMLBuilder addContent(List<ArticleItem> list)
    {
        this.list = list;
        return this;
    }

    @Override
    public IExportHTMLBuilder addPartOfSpeechColor( @ColorInt int color)
    {
        this.partOfSpeechColor = color;
        return this;
    }

    @Override
    public String createHTMLString()
    {
        return
                HTML_OPEN_TAG +
                HEAD_ENCODING_TAG +
                BODY_OPEN_TAG +
                wrapParagraph( header ) +
                convertArticlesListToHtmlString()  +
                wrapParagraph( footer )  +
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

    private String convertArticlesListToHtmlString()
    {
        SpannableStringBuilder wordsBuilder = new SpannableStringBuilder();

        int historyWordsSize = list.size();
        for (int i = 0; i < historyWordsSize; i++)
        {
            ArticleItem articleItem = list.get(i);

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

            wordsBuilder
                    .append(PARAGRAPH_OPEN_TAG)
                    .append(Html.toHtml(wordBuilder))
                    .append(PARAGRAPH_CLOSE_TAG);
        }
        return wordsBuilder.toString();
    }

    private String wrapParagraph(String content)
    {
        return PARAGRAPH_OPEN_TAG + content + PARAGRAPH_CLOSE_TAG;
    }

    private void appendPartOfSpeechSpanToWord(SpannableStringBuilder builder, int startIndex, int endIndex)
    {
        builder.setSpan(new ForegroundColorSpan(partOfSpeechColor), startIndex, endIndex, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        builder.setSpan(new StyleSpan(Typeface.ITALIC), startIndex, endIndex, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
}
