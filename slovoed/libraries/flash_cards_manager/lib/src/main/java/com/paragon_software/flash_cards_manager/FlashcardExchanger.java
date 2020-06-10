package com.paragon_software.flash_cards_manager;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.util.Base64;
import android.util.Log;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.SyncEngineAPI;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.CharArrayWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
import java.util.zip.ZipException;

import io.reactivex.Single;

public class FlashcardExchanger implements FlashcardExchangerAPI
{
    private static final String EXPORT_FILES_CHILD_DIR = "export";
    private static final String EXPORT_FILE_NAME_PLACEHOLDER = "flashcards_%s.bin";

    private static final String PRODUCT_ID_FIELD_NAME = "product";
    private static final String CARD_BOXES_FIELD_NAME = "mCardBoxes";

    private static final String FLASH_CARD_BOX_LOCAL_ID_FIELD_NAME = "mLocalId";
    private static final String FLASH_CARD_BOX_TYPE_FIELD_NAME = "mType";
    private static final String FLASH_CARD_BOX_PARENT_ARTICLE_FIELD_NAME = "mParentArticle";
    private static final String FLASH_CARD_BOX_ENABLE_FIELD_NAME = "mEnabled";
    private static final String FLASH_CARD_BOX_CARDS_FIELD_NAME = "mCards";

    private static final String FLASH_CARD_ENABLE_FILED_NAME = "mEnabled";
    private static final String FLASH_CARD_FRONT_ARTICLE_FILED_NAME = "mFrontSideArticle";
    private static final String FLASH_CARD_BACK_ARTICLE_FILED_NAME = "mBackSideArticle";
    private static final String FLASH_CARD_TRY_COUNT_FILED_NAME = "mTryCount";
    private static final String FLASH_CARD_RIGHT_ANS_COUNT_FILED_NAME = "mRightAnsCount";
    private static final String FLASH_CARD_LAST_SHOW_TIME = "mLastShowTime";
    private static final String FLASH_CARD_SEQUENCE_NUMBER_LINK = "mSequenceNumberFlashCardLink";
    private static final int    FLASH_CARD_LAST_SHOW_TIME_DEFAULT = 0;
    private static final int    FLASH_CARD_SEQUENCE_NUMBER_LINK_DEFAULT = -1;

    private static final String ARTICLE_SDC_ID_FIELD_NAME = "mSdcId";
    private static final String ARTICLE_HEADWORD_FIELD_NAME = "mHeadword";
    private static final String ARTICLE_LIST_ID_FIELD_NAME = "mListId";
    private static final String ARTICLE_GLOBAL_ARTICLE_ID_FIELD_NAME = "mGlobalArticleId";
    private static final String ARTICLE_ENGINE_FIELD_NAME = "mEngine";

    private static final int MAX_FILE_SIZE = 50 * 1024 * 1024;

    private final Context mContext;
    private final SyncEngineAPI mEngine;

    FlashcardExchanger(Context context, SyncEngineAPI engine)
    {
        this.mContext = context;
        this.mEngine = engine;
    }

    @Override
    public Single< File > getExportFlashcardsFile(final String productId, final List<FlashcardBox> flashcardBoxes )
    {
        return Single.fromCallable(new Callable< File >()
        {
            @Override
            public File call() throws JSONException
            {
                return packToZip(productId, getExportFlashcardsJson(productId, flashcardBoxes));
            }
        });
    }

    @Override
    public Single< List< FlashcardBox > > importFlashcards(final Uri fileUri )
    {
        return Single.fromCallable(new Callable< List < FlashcardBox > >()
        {
            @Override
            public List< FlashcardBox > call() throws IOException, JSONException
            {
                return getFlashcardsBoxListFromJsonObject(unpackZipAndGetJsonObject(fileUri));
            }
        });
    }

    private JSONObject getExportFlashcardsJson( String productId, List<FlashcardBox> flashcardBoxes )
            throws JSONException
    {
        JSONObject jsonObject = new JSONObject();
        jsonObject.put(PRODUCT_ID_FIELD_NAME, productId);

        JSONArray jsonArray = new JSONArray();
        for (int i = 0; i < flashcardBoxes.size(); i++)
        {
            FlashcardBox flashcardBox = flashcardBoxes.get(i);
            jsonArray.put(getJsonObjectFromFlashcardBox(i, flashcardBox));

        }
        jsonObject.put(CARD_BOXES_FIELD_NAME, jsonArray);

        return jsonObject;
    }

    private List<FlashcardBox> getFlashcardsBoxListFromJsonObject( JSONObject object )
            throws JSONException
    {
        if( object.has(PRODUCT_ID_FIELD_NAME) && object.has(CARD_BOXES_FIELD_NAME) )
        {
            String productId = object.getString(PRODUCT_ID_FIELD_NAME);
            JSONArray jsonBoxes = object.getJSONArray(CARD_BOXES_FIELD_NAME);

            if(jsonBoxes.length() > 0)
            {
                List<FlashcardBox> flashcardBoxes = new ArrayList<>(jsonBoxes.length());
                for(int i = 0; i < jsonBoxes.length(); i++ )
                {
                    try
                    {
                        flashcardBoxes.add(getFlashcardBoxFromJsonObject(productId, jsonBoxes.getJSONObject(i)));
                    }
                    catch (ArticleParseException ignore)
                    {
                    }
                }
                return flashcardBoxes;
            }
            {
                return new ArrayList<>();
            }
        }
        else
        {
            throw new IllegalArgumentException("Json object doesn't contain productId");
        }
    }

    private FlashcardBox getFlashcardBoxFromJsonObject( String productId, JSONObject object )
            throws JSONException, ArticleParseException
    {
        JSONArray cardsArray = object.getJSONArray(FLASH_CARD_BOX_CARDS_FIELD_NAME);
        List<Flashcard> flashcards = new ArrayList<>();
        if(cardsArray.length() > 0)
        {
            for(int i = 0; i < cardsArray.length(); i++)
            {
                flashcards.add(getFlashcardFromJson(productId, cardsArray.getJSONObject(i)));
            }
        }

        ArticleItem parentArticle =
                getArticleFromJsonObject(productId, object.getJSONObject(FLASH_CARD_BOX_PARENT_ARTICLE_FIELD_NAME), true);

        boolean isChecked = object.getBoolean(FLASH_CARD_BOX_ENABLE_FIELD_NAME);

        return FlashcardBox.setChecked(new FlashcardBox(parentArticle, flashcards), isChecked);
    }

    private JSONObject getJsonObjectFromFlashcardBox( int index, FlashcardBox flashcardBox )
            throws JSONException
    {
        JSONObject object = new JSONObject();
        object.put(FLASH_CARD_BOX_LOCAL_ID_FIELD_NAME, index);
        object.put(FLASH_CARD_BOX_TYPE_FIELD_NAME, 1); // TODO: fink about type enum from old duden
        object.put(FLASH_CARD_BOX_PARENT_ARTICLE_FIELD_NAME, getJsonObjectFromArticle(flashcardBox.getParent()));
        object.put(FLASH_CARD_BOX_ENABLE_FIELD_NAME, flashcardBox.isChecked());

        JSONArray cardsJson = new JSONArray();
        for (Flashcard flashcard : flashcardBox.getCards())
        {
            cardsJson.put(getJsonObjectFromFlashcard(flashcard));
        }
        object.put(FLASH_CARD_BOX_CARDS_FIELD_NAME, cardsJson);

        return object;
    }

    private ArticleItem getArticleFromJsonObject( String productId, JSONObject object, boolean throwException )
            throws JSONException, ArticleParseException
    {
        Dictionary.DictionaryId dictionaryId =
                new Dictionary.DictionaryId(productId);
        int listId = object.getInt(ARTICLE_LIST_ID_FIELD_NAME);
        int globalArticleId = object.getInt(ARTICLE_GLOBAL_ARTICLE_ID_FIELD_NAME);
        String headword = object.getString(ARTICLE_HEADWORD_FIELD_NAME);

        ArticleItem item = mEngine.find(dictionaryId, listId, globalArticleId, null);
        if(item == null
                || !headword.equals(item.getShowVariantText())
                || item.isLocked()
                || !item.getDictId().equals(dictionaryId))
        {
            if(throwException)
            {
                throw new ArticleParseException();
            }
        }

        return item;
    }

    private JSONObject getJsonObjectFromArticle( ArticleItem articleItem )
            throws JSONException
    {
        if(articleItem == null)
        {
            return new JSONObject();
        }
        JSONObject object = new JSONObject();
        object.put(ARTICLE_SDC_ID_FIELD_NAME, articleItem.getDictId().toString());
        object.put(ARTICLE_HEADWORD_FIELD_NAME, articleItem.getShowVariantText());
        object.put(ARTICLE_LIST_ID_FIELD_NAME, articleItem.getListId());
        object.put(ARTICLE_GLOBAL_ARTICLE_ID_FIELD_NAME, articleItem.getGlobalIndex());
        if(articleItem.getHistoryElement() != null)
        {
            object.put(ARTICLE_ENGINE_FIELD_NAME, Base64.encodeToString(articleItem.getHistoryElement(), Base64.DEFAULT));
        }
        return object;
    }

    private Flashcard getFlashcardFromJson( String productId, JSONObject object )
            throws JSONException, ArticleParseException
    {
        boolean isChecked = object.getBoolean(FLASH_CARD_ENABLE_FILED_NAME);
        ArticleItem frontArticle = getArticleFromJsonObject(productId, object.getJSONObject(FLASH_CARD_FRONT_ARTICLE_FILED_NAME), false);
        ArticleItem backArticle = getArticleFromJsonObject(productId, object.getJSONObject(FLASH_CARD_BACK_ARTICLE_FILED_NAME), false);
        int tryCount = object.getInt(FLASH_CARD_TRY_COUNT_FILED_NAME);
        int rightAnsCount = object.getInt(FLASH_CARD_RIGHT_ANS_COUNT_FILED_NAME);
        return new Flashcard(isChecked, tryCount, rightAnsCount, frontArticle, backArticle);
    }

    private JSONObject getJsonObjectFromFlashcard( Flashcard flashcard )
            throws JSONException
    {
        JSONObject object = new JSONObject();
        object.put(FLASH_CARD_ENABLE_FILED_NAME, flashcard.isChecked());
        object.put(FLASH_CARD_FRONT_ARTICLE_FILED_NAME, getJsonObjectFromArticle(flashcard.getFront()));
        object.put(FLASH_CARD_BACK_ARTICLE_FILED_NAME, getJsonObjectFromArticle(flashcard.getBack()));
        object.put(FLASH_CARD_TRY_COUNT_FILED_NAME, flashcard.getShowCount());
        object.put(FLASH_CARD_RIGHT_ANS_COUNT_FILED_NAME, flashcard.getRightAnsCount());
        object.put(FLASH_CARD_LAST_SHOW_TIME, FLASH_CARD_LAST_SHOW_TIME_DEFAULT);
        object.put(FLASH_CARD_SEQUENCE_NUMBER_LINK, FLASH_CARD_SEQUENCE_NUMBER_LINK_DEFAULT);
        return object;
    }

    private InputStream checkExtensionAndGetInputStream(Uri uri, ContentResolver contentResolver)
            throws IOException
    {
        InputStream is = null;
        try
        {
            is = contentResolver.openInputStream(uri);
            if(is != null)
            {
                is = new BufferedInputStream(is);
                if(((GZIPInputStream.GZIP_MAGIC & 0xff) != is.read()) &&
                        ((GZIPInputStream.GZIP_MAGIC >> 8) & 0xff) != is.read())
                {
                    throw new ZipException("It is not a zip package");
                }
            }
        }
        catch (ZipException ze)
        {
            throw new ZipException("It is not a zip package");
        }
        finally
        {
            if(is != null)
            {
                is.close();
            }
        }
        return contentResolver.openInputStream(uri);
    }

    private File packToZip(String productId, JSONObject jsonObject)
    {
        File exportDir = new File(mContext.getFilesDir(), EXPORT_FILES_CHILD_DIR);
        if(!exportDir.exists())
        {
            exportDir.mkdir();
        }
        File exportFile = new File(exportDir, String.format(EXPORT_FILE_NAME_PLACEHOLDER, productId));
        try (GZIPOutputStream gzipOutputStream = new GZIPOutputStream(new FileOutputStream(exportFile)))
        {
            gzipOutputStream.write(jsonObject.toString().getBytes());
        }
        catch (IOException ignore)
        {
        }
        return exportFile;
    }

    private JSONObject unpackZipAndGetJsonObject(Uri fileUri) throws IOException, JSONException
    {
        InputStream is = checkExtensionAndGetInputStream(fileUri, mContext.getContentResolver());
        if(is != null)
        {
            GZIPInputStream gzip = new GZIPInputStream(is);
            InputStreamReader isr = new InputStreamReader(gzip, StandardCharsets.UTF_8);
            BufferedReader bufferedReader = new BufferedReader(isr);
            CharArrayWriter wr = createCharArrayWriter(bufferedReader);
            return new JSONObject(wr.toString());
        }
        else
        {
            throw new IllegalArgumentException();
        }
    }

    @NonNull
    private CharArrayWriter createCharArrayWriter(BufferedReader bufferedReader)
            throws IOException
    {
        CharArrayWriter wr = new CharArrayWriter();
        char[] buf = new char[100];
        int read;
        while((read = bufferedReader.read(buf, 0, buf.length)) > -1)
        {
            wr.write(buf, 0, read);
            if (wr.size() > MAX_FILE_SIZE)
            {
                throw new IOException("File size is too big");
            }
        }
        return wr;
    }

    private static class ArticleParseException extends Exception
    {
    }
}
