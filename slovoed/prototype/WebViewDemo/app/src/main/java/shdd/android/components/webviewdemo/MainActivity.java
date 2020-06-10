package shdd.android.components.webviewdemo;

import android.net.Uri;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.inputmethod.EditorInfo;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.EditText;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.browser.customtabs.CustomTabsIntent;

public class MainActivity extends AppCompatActivity
                          implements TextView.OnEditorActionListener {
    EditText mAddress;
    WebView mContent;
    Menu mMenu;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mAddress = findViewById(R.id.address);
        mContent = findViewById(R.id.content);
        mContent.setWebViewClient(new WebViewClient());
        mContent.getSettings().setJavaScriptEnabled(true);
        mContent.getSettings().setDomStorageEnabled(true);
        mContent.getSettings().setDatabaseEnabled(true);
        mContent.getSettings().setAppCachePath(getCacheDir().getAbsolutePath());
        mContent.getSettings().setAppCacheEnabled(true);
        mContent.getSettings().setAllowUniversalAccessFromFileURLs(true);
        mAddress.requestFocus();
        mAddress.setOnEditorActionListener(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mMenu = menu;
        getMenuInflater().inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        String address = mAddress.getText().toString();
        if((!address.startsWith("http://")) && (!address.startsWith("https://")))
            address = "http://" + address;
        if(id == R.id.show)
            mContent.loadUrl(address);
        else if(id == R.id.open) {
            CustomTabsIntent.Builder b = new CustomTabsIntent.Builder();
            CustomTabsIntent customTabsIntent = b.enableUrlBarHiding().build();
            customTabsIntent.launchUrl(this, Uri.parse(address));
        }
        return true;
    }

    @Override
    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        if(actionId == EditorInfo.IME_ACTION_GO)
            onOptionsItemSelected(mMenu.findItem(R.id.show));
        return true;
    }
}
