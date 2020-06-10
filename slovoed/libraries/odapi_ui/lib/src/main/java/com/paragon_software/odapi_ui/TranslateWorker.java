package com.paragon_software.odapi_ui;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Picture;
import android.os.Handler;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.text.Html;
import android.util.TypedValue;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.native_engine.HtmlBuilderParams;
import com.paragon_software.native_engine.SyncEngineAPI;
import com.paragon_software.settings_manager.ApplicationSettings;

import java.util.Collection;
import java.util.concurrent.CountDownLatch;
import java.util.regex.Pattern;

class TranslateWorker extends QueryWorkerAbstract {
    private static final Pattern STYLE_PATTERN = createPatternForTag("style");
    private static final Pattern SCRIPT_PATTERN = createPatternForTag("script");

    TranslateWorker(@NonNull Query query, @NonNull String value) {
        super(query, value);
    }

    @NonNull
    @Override
    Result getResult() {
        Result res;
        SyncEngineAPI engine = getEngine();
        Context context = getContext();
        String mode = getMode();
        if(mode != null)
            if((engine != null) && (context != null) && (mHandler != null)) {
                Collection<DictionaryAndDirection> dnds = getDictionariesAndDirections();
                DictionaryAndDirection dnd = getSelectedDictionaryAndDirection();
                if (dnd != null)
                    if (dnds.contains(dnd)) {
                        ArticleItem article = engine.find(dnd.getDictionaryId(), dnd.getDirection(), mValue);
                        if (article != null)
                            res = translate(context, engine, article, mode);
                        else
                            res = searchInOtherDictionaries(context, engine, dnd, dnds);
                    }
                    else
                        if(!dnds.isEmpty())
                            res = searchInOtherDictionaries(context, engine, dnd, dnds);
                        else
                            res = openCatalog(context, dnd.getDictionaryId());
                else
                    if(!dnds.isEmpty()) {
                        ArticleItem article = null;
                        for(DictionaryAndDirection d : dnds) {
                            article = engine.find(d.getDictionaryId(), d.getDirection(), mValue);
                            if(article != null)
                                break;
                        }
                        if(article != null)
                            res = translate(context, engine, article, mode);
                        else
                            res = showOnNothingFound(context);
                    }
                    else
                        res = openCatalog(context, null);
            }
            else
                res = new CommonErrorResult(CommonErrorResult.Kind.AppStateError, "Initialization error");
        else
            res = new WrongParamsResult("Mode not found");
        return res;
    }

    @NonNull
    private Result translate(@NonNull Context context, @NonNull SyncEngineAPI engine, @NonNull ArticleItem article, @NonNull String mode) {
        Result res;
        HtmlBuilderParams htmlParams = new HtmlBuilderParams.Builder().setScale(
            getArticleManager() != null ? getArticleManager().getArticleScale() : ApplicationSettings.getDefaultArticleScale()).create();
        String translation = engine.translate(article, htmlParams);
        if (translation != null)
            if (Query.Specific.TranslateAsText.equals(mQuery.specific))
                res = new TextTranslateResult(mode, translation, stripHtml(translation));
            else if (Query.Specific.TranslateAsImage.equals(mQuery.specific))
                if ((mQuery.width > 0) && (mQuery.height > 0)) {
                    Bitmap bmp = HtmlRenderer.render(context, mHandler, mQuery.width, mQuery.height, translation);
                    res = new ImageTranslateResult(mode, bmp);
                }
                else {
                    String param = (mQuery.width <= 0) ? "width" : "height";
                    res = new WrongParamsResult(param + " param must be grater than 0");
                }
            else if (mQuery.specific != null)
                res = new WrongParamsResult("Query \"" + mQuery.specific.getName() + "\" is unsupported");
            else
                res = new WrongParamsResult("Query not found");
        else
            res = new CommonErrorResult(CommonErrorResult.Kind.DatabaseIsUnavailable, "Can't open database");
        return res;
    }

    @NonNull
    private Result searchInOtherDictionaries(@NonNull Context context, @NonNull SyncEngineAPI engine, @NonNull DictionaryAndDirection dnd,  @NonNull Collection<DictionaryAndDirection> dnds) {
        ArticleItem firstArticle = null;
        ArticleItem firstArticleInSameDirection = null;
        ArticleItem article;
        Result res;
        for (DictionaryAndDirection d : dnds) {
            article = engine.find(d.getDictionaryId(), d.getDirection(), mValue);
            if (article != null) {
                if (firstArticle == null)
                    firstArticle = article;
                if (d.getDirection().equals(dnd.getDirection())) {
                    firstArticleInSameDirection = article;
                    break;
                }
            }
        }
        article = (firstArticleInSameDirection != null) ? firstArticleInSameDirection : firstArticle;
        if (article != null)
            res = openArticle(context, dnd.getDictionaryId(), article, article == firstArticleInSameDirection);
        else
            res = showOnNothingFound(context);
        return res;
    }

    @Nullable
    private String getMode() {
        String res = null;
        if(mQuery.mode != null)
            res = mQuery.mode.getName();
        return res;
    }

    // https://stackoverflow.com/questions/6502759/how-to-strip-or-escape-html-tags-in-android
    private static String stripHtml(@NonNull String html) {
        html =STYLE_PATTERN.matcher(html).replaceFirst("");
        html = SCRIPT_PATTERN.matcher(html).replaceFirst("");
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N)
            html = Html.fromHtml(html, Html.FROM_HTML_MODE_LEGACY).toString();
        else
            html = Html.fromHtml(html).toString();
        return html;
    }

    private static Pattern createPatternForTag(@NonNull String tag) {
        return Pattern.compile("<" + tag + "[^>]*?>.*?</" + tag + ">", Pattern.DOTALL);
    }

    private static class HtmlRenderer implements Runnable, WebView.PictureListener {
        @NonNull
        static Bitmap render(Context context, Handler handler, int width, int height, @NonNull String html) {
            HtmlRenderer renderer = new HtmlRenderer(context, width, height, html);
            handler.post(renderer);
            try {
                renderer.mLatch.await();
            }
            catch (InterruptedException ignore) { }
            return renderer.mBmp;
        }

        private final Context mApplicationContext;
        private int mWidth;
        private int mHeight;

        @NonNull
        private final String mHtml;

        @NonNull
        private final CountDownLatch mLatch = new CountDownLatch(1);

        @NonNull
        private final Bitmap mBmp;

        HtmlRenderer(Context context, int width, int height, @NonNull String html) {
            mApplicationContext = context.getApplicationContext();
            mWidth = width;
            mHeight = height;
            mHtml = html;
            mBmp = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.ARGB_8888);
        }

        @Override
        public void run() {
            final WebView wv = new WebView(mApplicationContext);
            wv.getSettings().setRenderPriority(WebSettings.RenderPriority.HIGH);
            wv.setVerticalFadingEdgeEnabled(true);
            wv.setFadingEdgeLength((int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 20, mApplicationContext.getResources().getDisplayMetrics()));
            wv.setScrollBarStyle(View.SCROLLBARS_INSIDE_OVERLAY);
            wv.setVerticalScrollbarOverlay(true);
            wv.setVerticalScrollBarEnabled(false);
            wv.setHorizontalScrollbarOverlay(true);
            wv.setHorizontalScrollBarEnabled(false);
            wv.getSettings().setUseWideViewPort(true);
            wv.measure(View.MeasureSpec.makeMeasureSpec(mWidth, View.MeasureSpec.EXACTLY), View.MeasureSpec.makeMeasureSpec(mHeight, View.MeasureSpec.EXACTLY));
            wv.layout(0, 0, wv.getMeasuredWidth(), wv.getMeasuredHeight());
            wv.setPictureListener(this);
            wv.loadDataWithBaseURL("file:///", mHtml, "text/html", "utf-8", null);
        }

        @Override
        public void onNewPicture(WebView wv, @Nullable Picture picture) {
            wv.setPictureListener(null);
            wv.draw(new Canvas(mBmp));
            mLatch.countDown();
        }
    }
}
