package com.paragon_software.article_manager;

import android.annotation.SuppressLint;
import android.content.Context;
import androidx.annotation.NonNull;
import android.view.View;
import android.webkit.WebView;

import java.util.LinkedList;
import java.util.List;

class WebViewWrapper implements SearchTarget, WebView.FindListener
{
  @NonNull
  private final WebView mWebView;

  private int mCurrent = 0;
  private int mTotal = -1;

  @NonNull
  private final List< OnStateChangedListener > mListeners = new LinkedList<>();

  private boolean mSearching = false;

  @SuppressLint( "SetJavaScriptEnabled" )
  WebViewWrapper(Context context) {
    mWebView = new WebView(context.getApplicationContext());
    mWebView.setLayerType(View.LAYER_TYPE_SOFTWARE, null);
    mWebView.getSettings().setJavaScriptEnabled(true);
    mWebView.getSettings().setUseWideViewPort(true);
    mWebView.setFindListener(this);
  }

  @NonNull
  WebView getView() {
    return mWebView;
  }

  void rememberSearchPosition() {

  }

  void updateSearchPosition() {
    if(mTotal > 0) {
      mCurrent = 0;
      notifyListeners();
    }
  }

  @Override
  public void search( @NonNull String text ) {
    clearMatches();
    if ( text.length() > 0 ) {
      mCurrent = 0;
      mTotal = -1;
      mSearching = true;
      mWebView.findAllAsync(text);
      notifyListeners();
    }
  }

  @Override
  public void findNext() {
    if ( mTotal > 0 ) {
      mSearching = true;
      mWebView.findNext(true);
      notifyListeners();
    }
  }

  @Override
  public void findPrevious() {
    if ( mTotal > 0 ) {
      mSearching = true;
      mWebView.findNext(false);
      notifyListeners();
    }
  }

  @Override
  public void clearMatches() {
    mSearching = false;
    mWebView.clearMatches();
    mCurrent = 0;
    mTotal = -1;
    notifyListeners();
  }

  @Override
  public boolean canNavigate() {
    return !( mSearching || ( mTotal <= 0 ) );
  }

  @Override
  public void onFindResultReceived( int activeMatchOrdinal, int numberOfMatches, boolean isDoneCounting ) {
    if ( mSearching ) {
      mCurrent = activeMatchOrdinal;
      mTotal = numberOfMatches;
      mSearching = !isDoneCounting;
      notifyListeners();
    }
  }

  @NonNull
  @Override
  public String getResult() {
    String res;
    if ( mTotal < 0 )
      res = "";
    else if ( mTotal == 0 )
      res = "0/0";
    else
      res = String.valueOf(mCurrent + 1) + "/" + String.valueOf(mTotal);
    return res;
  }

  @Override
  public void registerListener( @NonNull OnStateChangedListener listener ) {
    if ( !mListeners.contains(listener) )
      mListeners.add(listener);
  }

  @Override
  public void unregisterListener( @NonNull OnStateChangedListener listener ) {
    mListeners.remove(listener);
  }

  private void notifyListeners() {
    for ( OnStateChangedListener listener : mListeners )
      listener.onSearchStateChanged();
  }
}
