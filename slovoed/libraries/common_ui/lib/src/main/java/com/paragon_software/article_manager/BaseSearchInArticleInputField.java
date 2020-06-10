package com.paragon_software.article_manager;

import android.content.Context;
import android.os.Handler;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.paragon_software.common_ui.R;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

/**
 * TODO: document your custom view class.
 */
abstract class BaseSearchInArticleInputField extends RelativeLayout
{
  @NonNull
  Handler mHandler = new Handler();
  @Nullable
  private SetEditTextRightPadding mSetEditTextRightPadding = null;
  private int                     mLeftPadding             = 0;
  private int                     mBottomPadding           = 0;
  private int                     mLastRightPaddingSet     = 0;

  @NonNull
  private final HashMap< OnTextChangedListener, TextWatcher > mTextListeners = new HashMap<>(1);

  public BaseSearchInArticleInputField(Context context )
  {
    super(context);
    init(context);
  }

  public BaseSearchInArticleInputField(Context context, @Nullable AttributeSet attrs )
  {
    super(context, attrs);
    init(context);
  }

  public BaseSearchInArticleInputField(Context context, @Nullable AttributeSet attrs, int defStyleAttr )
  {
    super(context, attrs, defStyleAttr);
    init(context);
  }

  protected abstract int getEditTextInputId();

  protected abstract int getSearchResultViewId();

  void setSearchText( @NonNull String searchText )
  {
    getEditText().setText(searchText);
  }

  void setResultText( @NonNull String resultText )
  {
    TextView textView = getTextView();
    textView.setText(resultText);
    textView.setVisibility(resultText.length() > 0 ? VISIBLE : GONE);
  }

  void selectAll()
  {
    getEditText().selectAll();
  }

  void addOnTextChangedListener( @NonNull OnTextChangedListener listener )
  {
    TextWatcher textWatcher = new EditWatcher(listener);
    mTextListeners.put(listener, textWatcher);
    getEditText().addTextChangedListener(textWatcher);
  }

  void removeOnTextChangedListener( @NonNull OnTextChangedListener listener )
  {
    TextWatcher textWatcher = mTextListeners.remove(listener);
    if ( textWatcher != null )
    {
      getEditText().removeTextChangedListener(textWatcher);
    }
  }

  @Override
  protected void onFinishInflate()
  {
    super.onFinishInflate();
    getEditText().setOnFocusChangeListener(getFocusListener());
    getEditText().setOnEditorActionListener(new EditorActionListener(mTextListeners));
  }

  @Override
  protected void onLayout( boolean changed, int l, int t, int r, int b )
  {
    super.onLayout(changed, l, t, r, b);
    int newTextViewWidth = getTextViewWidth();
    int editTextRightPadding = mLastRightPaddingSet;
    if ( editTextRightPadding == newTextViewWidth )
    {
      clearPaddingSetter();
    }
    else
    {
      if ( mSetEditTextRightPadding != null )
      {
        int futurePadding = mSetEditTextRightPadding.getRightPaddingToSet();
        if ( futurePadding != newTextViewWidth )
        {
          clearPaddingSetter();
        }
      }
      if ( mSetEditTextRightPadding == null )
      {
        mSetEditTextRightPadding = new SetEditTextRightPadding(this, newTextViewWidth);
        mHandler.post(mSetEditTextRightPadding);
      }
    }
  }

  private void init( Context context )
  {
    mLeftPadding = context.getResources()
                          .getDimensionPixelSize(R.dimen.article_manager_ui_search_in_article_input_field_left_padding);
    mBottomPadding = context.getResources().getDimensionPixelSize(
        R.dimen.article_manager_ui_search_in_article_input_field_bottom_padding);
  }

  private EditText __editText = null;

  private EditText getEditText()
  {
    if ( __editText == null )
    {
      __editText = findViewById(getEditTextInputId());
    }
    return __editText;
  }

  private TextView __textView = null;

  private TextView getTextView()
  {
    if ( __textView == null )
    {
      __textView = findViewById(getSearchResultViewId());
    }
    return __textView;
  }

  private int getTextViewWidth()
  {
    int res = 0;
    TextView textView = getTextView();
    if ( textView != null )
    {
      if ( textView.getVisibility() != View.GONE )
      {
        ViewGroup.MarginLayoutParams layoutParams = (ViewGroup.MarginLayoutParams) textView.getLayoutParams();
        res = textView.getWidth() + layoutParams.leftMargin + layoutParams.rightMargin;
      }
    }
    return res;
  }

  private void clearPaddingSetter()
  {
    if ( mSetEditTextRightPadding != null )
    {
      mHandler.removeCallbacks(mSetEditTextRightPadding);
      mSetEditTextRightPadding = null;
    }
  }

  private static class SetEditTextRightPadding implements Runnable
  {
    @NonNull
    private final WeakReference<BaseSearchInArticleInputField> searchInArticleInputField;
    private final int                                        rightPaddingToSet;

    SetEditTextRightPadding(@NonNull BaseSearchInArticleInputField _searchInArticleInputField, int _rightPaddingToSet )
    {
      searchInArticleInputField = new WeakReference<>(_searchInArticleInputField);
      rightPaddingToSet = _rightPaddingToSet;
    }

    int getRightPaddingToSet()
    {
      return rightPaddingToSet;
    }

    @Override
    public void run()
    {
      BaseSearchInArticleInputField field = searchInArticleInputField.get();
      if ( field != null )
      {
        EditText editText = field.getEditText();
        if ( editText != null )
        {
          editText.setPadding(field.mLeftPadding, 0, rightPaddingToSet, field.mBottomPadding);
          field.mLastRightPaddingSet = rightPaddingToSet;
        }
        field.mSetEditTextRightPadding = null;
      }
    }
  }


  private FocusListener __focusListener = null;

  private FocusListener getFocusListener()
  {
    if ( __focusListener == null )
    {
      __focusListener = new FocusListener();
    }
    return __focusListener;
  }

  private class FocusListener implements View.OnFocusChangeListener
  {

    @Override
    public void onFocusChange( View v, boolean hasFocus )
    {
      if ( ( v.getId() == getEditTextInputId() ) && hasFocus )
      {
        KeyboardHelper.showKeyboard(v);
      }
    }
  }

  private static class EditWatcher implements TextWatcher
  {

    @NonNull
    private final OnTextChangedListener listener;

    EditWatcher( @NonNull OnTextChangedListener _listener )
    {
      listener = _listener;
    }

    @Override
    public void beforeTextChanged( CharSequence s, int start, int count, int after )
    {

    }

    @Override
    public void onTextChanged( CharSequence s, int start, int before, int count )
    {

    }

    @Override
    public void afterTextChanged( Editable s )
    {
      listener.onTextChanged(s.toString());
    }
  }

  private static class EditorActionListener implements TextView.OnEditorActionListener
  {
    @NonNull
    private final Map< OnTextChangedListener, TextWatcher > listeners;

    EditorActionListener( @NonNull Map< OnTextChangedListener, TextWatcher > _listeners )
    {
      listeners = _listeners;
    }

    @Override
    public boolean onEditorAction( TextView v, int actionId, KeyEvent event )
    {
      boolean res = false;
      if ( actionId == EditorInfo.IME_ACTION_SEARCH )
      {
        for ( OnTextChangedListener listener : listeners.keySet() )
        {
          KeyboardHelper.hideKeyboard(v);
          listener.onTextChanged(v.getText().toString());
        }
        res = true;
      }
      return res;
    }
  }
}
