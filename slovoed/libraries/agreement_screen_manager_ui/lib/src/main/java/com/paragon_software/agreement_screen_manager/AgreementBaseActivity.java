package com.paragon_software.agreement_screen_manager;

import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatButton;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.widget.ScrollView;
import android.widget.TextView;

import com.paragon_software.agreement_screen_manager_ui.R;


public abstract class AgreementBaseActivity extends AppCompatActivity implements View.OnClickListener
{
    public static final String HIDE_BUTTONS_EXTRA = "HIDE_BUTTONS_EXTRA";
    public static final String HIDE_ACTIONBAR_EXTRA = "HIDE_ACTIONBAR_EXTRA";

    protected static final String MAIN_TEXT_FIRST_CHARACTER_OFFSET = "MAIN_TEXT_FIRST_CHARACTER_OFFSET";

    protected ScrollView mScrollView;

    protected AppCompatButton mAgreeButton;

    protected AppCompatButton mDisagreeButton;

    protected TextView mAgreeInstructionText;

    protected TextView mMainText;

    protected TextView mLastUpdatedText;

    protected TextView mSubjectToChangeText;

    @Override
    protected void onCreate(Bundle savedInstanceState )
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_agreement);

        mScrollView = findViewById(R.id.scroll_view);

        mAgreeInstructionText = findViewById(R.id.agree_instruction_text);
        mAgreeInstructionText.setText(getAgreeInstructionText());

        mMainText = findViewById(R.id.main_text);
        mMainText.setText(getMainText());

        mLastUpdatedText = findViewById(R.id.last_updated);
        mLastUpdatedText.setText(getLastUpdatedText());

        mSubjectToChangeText = findViewById(R.id.subject_to_change);
        mSubjectToChangeText.setText(getOnlineInvitationText());

        mMainText.setMovementMethod(LinkMovementMethod.getInstance());
        mSubjectToChangeText.setMovementMethod(LinkMovementMethod.getInstance());

        boolean hideButtons = getIntent().getBooleanExtra(HIDE_BUTTONS_EXTRA, false);
        boolean hideActionBar = getIntent().getBooleanExtra(HIDE_ACTIONBAR_EXTRA, true);

        if ( hideButtons )
        {
            findViewById(R.id.buttons_container).setVisibility(View.GONE);
            mAgreeInstructionText.setVisibility(View.GONE);
        }
        else
        {
            mAgreeButton = findViewById(R.id.agreement_button);
            mAgreeButton.setOnClickListener(getAgreeButtonClickListener());

            mDisagreeButton = findViewById(R.id.disagreement_button);
            mDisagreeButton.setOnClickListener(this);
        }
        ActionBar actionBar = getSupportActionBar();
        if ( actionBar != null )
        {
            actionBar.setDisplayHomeAsUpEnabled(true);
            if ( hideActionBar )
            {
                actionBar.hide();
            }
        }
    }

    @Override
    public void onClick( View view )
    {
        if ( view.getId() == mDisagreeButton.getId() )
        {
            finish();
        }
    }

    @NonNull
    private String getLastUpdatedText()
    {
        return getString(R.string.agreement_screen_manager_ui_last_updated, getLastUpdatedDate());
    }

    @NonNull
    protected abstract View.OnClickListener getAgreeButtonClickListener();

    @NonNull
    protected abstract String getAgreeInstructionText();

    @NonNull
    protected abstract CharSequence getMainText();

    @NonNull
    protected abstract CharSequence getOnlineInvitationText();

    @NonNull
    protected abstract String getLastUpdatedDate();

    // Save first visible character offset of the main text view to restore scroll position of this view more correctly
    // considering its word wrapping.
    // See also: https://stackoverflow.com/questions/15678489/best-way-save-restore-textview-position-in-scrollview
    @Override
    protected void onSaveInstanceState(Bundle outState)
    {
        super.onSaveInstanceState(outState);
        final int firstVisibleLineOffset = mMainText.getLayout().getLineForVertical(mScrollView.getScrollY());
        final int firstVisibleCharacterOffset = mMainText.getLayout().getLineStart(firstVisibleLineOffset);
        outState.putInt(MAIN_TEXT_FIRST_CHARACTER_OFFSET, firstVisibleCharacterOffset);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        final int firstVisibleCharacterOffset = savedInstanceState.getInt(MAIN_TEXT_FIRST_CHARACTER_OFFSET);
        mScrollView.post(new Runnable()
        {
            public void run()
            {
                final int firstVisibleLineOffset = mMainText.getLayout().getLineForOffset(firstVisibleCharacterOffset);
                final int pixelOffset = mMainText.getLayout().getLineTop(firstVisibleLineOffset);
                mScrollView.scrollTo(0, pixelOffset);
            }
        });
    }

    @Override
    public boolean onSupportNavigateUp()
    {
        super.onBackPressed();
        return true;
    }
}
