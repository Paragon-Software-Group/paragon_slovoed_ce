package com.paragon_software.flash_cards_manager;

import android.app.Dialog;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.RecyclerView;
import androidx.appcompat.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.webkit.WebView;
import android.widget.Button;

import com.paragon_software.flash_cards_manager.FlashcardManagerAPI.ArticleFragmentFactory;
import com.paragon_software.flash_cards_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;
import com.paragon_software.utils_slovoed_ui_common.fragments.SimpleDialog;

import java.util.EnumSet;
import java.util.Map;

public class FcQuizActivity extends AppCompatActivity
                            implements View.OnClickListener,
                                       CollectionView.OnMetadataChanged,
                                       SimpleDialog.Target {
    @NonNull
    private final static String QUIT_DIALOG_TAG = "com.paragon_software.flash_cards_manager.FcQuizActivity.QuitDialog";

    private AbstractQuizCollectionView mCollection;

    private RecyclerView mCardListView;
    private WebView mCardWebView;
    private Button mShowMeaningButton, mStartAgainButton;
    private View mAnswerButtons;
    private Menu mMenu;
    private View mArticleFragment;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        FlashcardManagerAPI flashcardManagerAPI = FlashcardManagerHolder.get();
        mCollection = flashcardManagerAPI.getQuizCollectionView();
        if (mCollection.getCount() <= 0)
        {
            finish();
            return;
        }
        mCollection.setInQuizUI(true);
        if (null == savedInstanceState)
        {
            mCollection.restart();
        }
        mCollection.registerListener(this);
        setContentView(R.layout.activity_fc_quiz);
        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        ActionBar actionBar = getSupportActionBar();
        if(actionBar != null)
            actionBar.setDisplayHomeAsUpEnabled(true);
        mCardListView = findViewById(R.id.card_list);
        mCardListView.setLayoutManager(new QuizCardsLayoutManager(this));
        mCardListView.setAdapter(QuizCardListAdapter.create(mCollection));
        mCardListView.addItemDecoration(new QuizCardListDecoration());
        mCardWebView = findViewById(R.id.card);
        mShowMeaningButton = findViewById(R.id.show_meaning);
        mShowMeaningButton.setOnClickListener(this);
        mStartAgainButton = findViewById(R.id.start_again);
        mStartAgainButton.setOnClickListener(this);
        mAnswerButtons = findViewById(R.id.answer_buttons);
        findViewById(R.id.right).setOnClickListener(this);
        findViewById(R.id.wrong).setOnClickListener(this);
        mArticleFragment = findViewById(R.id.articleFragment);
        ArticleFragmentFactory articleFragmentFactory = flashcardManagerAPI.getArticleFragmentFactory();
        if ( null != articleFragmentFactory && null == savedInstanceState )
        {
            getSupportFragmentManager().beginTransaction()
                                       .replace(R.id.articleFragment, articleFragmentFactory.createArticleFragment())
                                       .commit();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mCollection.unregisterListener(this);
        mCollection.setInQuizUI(false);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        getMenuInflater().inflate(R.menu.fc_quiz, menu);
        mMenu = menu;
        updateUi();
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        boolean res = false;
        if(id == android.R.id.home) {
            onBackPressed();
            res = true;
        }
        else if(id == R.id.play_sound) {
            mCollection.playSound();
            res = true;
        }
        else if(id == R.id.delete) {
            mCollection.remove();
            res = true;
        }
        return res;
    }

    @Override
    public void onBackPressed() {
        QuizMetadata metadata = (QuizMetadata) mCollection.getMetadata();
        if(!EnumSet.of(QuizMetadata.QuizStateType.RESULT_OF_QUIZ, QuizMetadata.QuizStateType.RESULT_OF_QUIZ_CONGRATULATIONS).contains(metadata.quizStateType))
            SimpleDialog.show(this, QUIT_DIALOG_TAG,
                    new SimpleDialog.Builder()
                            .setMessage(getString(R.string.flash_cards_manager_ui_really_want_to_quit))
                            .setPositiveText(getString(R.string.utils_slovoed_ui_common_yes))
                            .setNegativeText(getString(R.string.utils_slovoed_ui_common_no)));
        else
            super.onBackPressed();
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.show_meaning)
            mCollection.showMeaning();
        else if((id == R.id.right) || (id == R.id.wrong))
            mCollection.answer(id == R.id.right);
        else if(id == R.id.start_again)
            mCollection.restart();
    }

    @Override
    public void onMetadataChanged() {
        updateUi();
    }

    @Override
    public void onCreateSimpleDialog(@Nullable String tag, @NonNull Dialog dialog, @Nullable Bundle extra) { }

    @Override
    public void onSimpleDialogButtonPressed(@Nullable String tag, int n, @Nullable Bundle extra) {
        if(QUIT_DIALOG_TAG.equals(tag) && (n == Dialog.BUTTON_POSITIVE))
            super.onBackPressed();
    }

    private void updateUi() {
        QuizMetadata metadata = (QuizMetadata) mCollection.getMetadata();
        metadata.htmlVisibility.applyTo(mCardWebView);
        metadata.articleVisibility.applyTo(mArticleFragment);
        mCardWebView.loadDataWithBaseURL("stub://", getHtml(metadata),
                                         "text/html", "UTF-8", null);
        Map<QuizMetadata.ButtonType, ButtonState> buttons = metadata.buttonState;
        buttons.get(QuizMetadata.ButtonType.ShowMeaning).applyTo(mShowMeaningButton);
        buttons.get(QuizMetadata.ButtonType.StartAgain).applyTo(mStartAgainButton);
        mAnswerButtons.setVisibility(
                (VisibilityState.gone.equals(buttons.get(QuizMetadata.ButtonType.Right).getVisibility()) &&
                VisibilityState.gone.equals(buttons.get(QuizMetadata.ButtonType.Wrong).getVisibility())) ?
                View.GONE : View.VISIBLE);
        buttons.get(QuizMetadata.ButtonType.Pronunciation).applyTo(mMenu, R.id.play_sound);
        buttons.get(QuizMetadata.ButtonType.Delete).applyTo(mMenu, R.id.delete);
    }

    @NonNull
    private String getHtml( @NonNull QuizMetadata metadata )
    {
        switch ( metadata.quizStateType )
        {
            case ARTICLE_FRAGMENT_VISIBLE:
                return "";
            case USER_TRY_TO_RECALL:
                if ( null != metadata.articleItem )
                {
                    return getString(R.string.flash_cards_manager_ui_front_html,
                                     metadata.articleItem.getShowVariantText(),
                                     getString(R.string.flash_cards_manager_ui_show_meaning),
                                     getString(R.string.flash_cards_manager_ui_right),
                                     getString(R.string.flash_cards_manager_ui_wrong));
                }
                break;
            case RESULT_OF_QUIZ:
            case RESULT_OF_QUIZ_CONGRATULATIONS:
                if ( metadata.correctCounter >= 0 && metadata.incorrectCounter >= 0 )
                {
                    return getString(metadata.quizStateType.equals(QuizMetadata.QuizStateType.RESULT_OF_QUIZ)
                                     ? R.string.flash_cards_manager_ui_results_html
                                     : R.string.flash_cards_manager_ui_results_congratulation_html,
                                     metadata.correctCounter,
                                     metadata.incorrectCounter);
                }
                break;
        }
        return "";
    }
}
