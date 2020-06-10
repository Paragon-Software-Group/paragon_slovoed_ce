/*
 * slovoed
 *
 *  Created on: 13.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.about_manager;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.paragon_software.about_manager_ui.R;
import com.paragon_software.utils_slovoed.pdahpc.About;
import com.paragon_software.utils_slovoed.pdahpc.AboutProvider;
import com.paragon_software.utils_slovoed.pdahpc.Catalog;
import com.paragon_software.utils_slovoed.pdahpc.CatalogAboutProvider;
import com.paragon_software.utils_slovoed.pdahpc.PDAHPCDataParser;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Calendar;

public class AboutFragment extends Fragment
{

  private static final int    MIN_COPYRIGHT_YEAR    = 2020;
  private WebView            contentWebView;
  @Nullable
  private AboutControllerAPI mController;

  @Override
  public void onCreate( @Nullable Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    Bundle arguments = getArguments();
    if ( null != arguments )
    {
      String controllerId = arguments.getString(AboutManagerAPI.EXTRA_CONTROLLER_ID);
      if ( null != controllerId )
      {
        AboutManagerAPI aboutManager = AboutMangerHolder.getManager();
        mController = aboutManager.getController(controllerId);
      }
    }
  }

  @Nullable
  @Override
  public View onCreateView( @NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                            @Nullable Bundle savedInstanceState )
  {
    View view = inflater.inflate(R.layout.fragment_about, container, false);
    contentWebView = view.findViewById(R.id.about_content_view);
    contentWebView.setBackgroundColor(Color.TRANSPARENT);
    if (null != mController)
    {
      AboutSpecs aboutSpecs = mController.getAboutSpecs();
      ArrayList<LocalizedString> soundAndMorpho = new ArrayList<>();
      if ( null != aboutSpecs.getSoundInfo() )
      {
        soundAndMorpho.addAll(aboutSpecs.getSoundInfo());
      }
      if ( null != aboutSpecs.getMorphoInfo() )
      {
        soundAndMorpho.addAll(aboutSpecs.getMorphoInfo());
      }
      fillAbout(aboutSpecs.getEngineVersion(),
                soundAndMorpho,
                aboutSpecs.getNumberOfWords(),
                aboutSpecs.getBaseVersion(),
                aboutSpecs.getProductName(),
                aboutSpecs.getDictionaryId());
    }
    return view;
  }

  public void fillAbout(String version, ArrayList<LocalizedString> soundAndMorphoInfo, String numberWords, String baseVersion, LocalizedString productName, String dictionaryId)
  {
    About about = PDAHPCDataParser.parseAbout(getContext());
    Catalog catalog = PDAHPCDataParser.parseCatalog(getContext());

    AboutProvider aboutProvider = new AboutProvider(about, catalog.getLocale().getDefault());
    CatalogAboutProvider catalogAboutProvider = new CatalogAboutProvider(catalog.getAbout(), catalog.getLocale().getDefault());

    String filledContent = getAboutPageTemplate(dictionaryId, aboutProvider)
            .replaceAll("\\$\\{" + "about_name" + "\\}", catalogAboutProvider.getName().get())
            .replaceAll("\\$\\{" + "version" + "\\}", createVersionString())
            .replaceAll("\\$\\{" + "copyright" + "\\}", catalogAboutProvider.getCopyright().get())
            .replaceAll("\\$\\{" + "year" + "\\}", String.valueOf(Math.max(MIN_COPYRIGHT_YEAR, Calendar.getInstance().get(Calendar.YEAR))))
            .replaceAll("\\$\\{" + "web" + "\\}", catalogAboutProvider.getWeb().get())
            .replaceAll("\\$\\{" + "label_search_engine" + "\\}", getString(R.string.about_manager_ui_search_engine_label))
            .replaceAll("\\$\\{" + "engine_version" + "\\}", "" + version)
            .replaceAll("\\$\\{" + "label_faq" + "\\}", getString(R.string.about_manager_ui_faq_label))
            .replaceAll("\\$\\{" + "faq" + "\\}", catalogAboutProvider.getFaq().get())
            .replaceAll("\\$\\{" + "label_support" + "\\}", getString(R.string.about_manager_ui_support_label))
            .replaceAll("\\$\\{" + "email_support" + "\\}", catalogAboutProvider.getSupportEmail().get())
            .replaceAll("\\$\\{" + "email_body" + "\\}", "")
            .replaceAll("\\$\\{" + "product_name" + "\\}", productName.get())
            .replaceAll("\\$\\{" + "base_version" + "\\}", baseVersion)
            .replaceAll("\\$\\{" + "label_entries_number" + "\\}", getString(R.string.about_manager_ui_entries_number_label))
            .replaceAll("\\$\\{" + "word_number" + "\\}", numberWords)
            .replaceAll("\\$\\{" + "label_provided_by" + "\\}", getString(R.string.about_manager_ui_provided_by_label))
            .replaceAll("\\$\\{" + "provided" + "\\}", catalogAboutProvider.getProvidedBrand().get())
            .replaceAll("\\$\\{" + "dict_info" + "\\}", createDictionaryInfoString(soundAndMorphoInfo, catalogAboutProvider));

    contentWebView.loadDataWithBaseURL("about:///", filledContent,
            "text/html", "utf-8", null);

    contentWebView.setWebViewClient(new WebViewClient() {
      @Override
      public boolean shouldOverrideUrlLoading(WebView view, String url) {
        if (url.startsWith("mailto:")) {
          Intent emailIntent = new Intent(Intent.ACTION_SENDTO);
          emailIntent.setData(Uri.parse(url));
          emailIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
          startActivity(emailIntent);
          return true;
        } else {
          Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
          startActivity(browserIntent);
          return true;
        }
      }
    });
  }

  private String createDictionaryInfoString(ArrayList<LocalizedString> soundAndMorphoInfo, CatalogAboutProvider catalogAboutProvider) {
    if (soundAndMorphoInfo != null) {
      StringBuilder result = new StringBuilder();
      for (LocalizedString info : soundAndMorphoInfo) {
        result.append(info.get()).append("<br>").append(createCopyrightString(catalogAboutProvider)).append("<br><br>");
      }
      return result.toString();
    }
    return "";
  }

  private String createVersionString()
  {
    try
    {
      return getActivity().getPackageManager().getPackageInfo(getActivity().getPackageName(), 0).versionName;
    }
    catch ( PackageManager.NameNotFoundException exception )
    {
      exception.printStackTrace();
    }
    return "";
  }

  private String createCopyrightString(CatalogAboutProvider catalogAboutProvider)
  {
    String res = catalogAboutProvider.getCopyright().get();
    if(TextUtils.isEmpty(res)) {
      int currentYear = Calendar.getInstance().get(Calendar.YEAR);
      int copyrightYear = Math.max(MIN_COPYRIGHT_YEAR, currentYear);
      res = getString(R.string.about_manager_ui_copyright, copyrightYear);
    }
    return res;
  }

  private String getAboutPageTemplate(String selectedDictionaryId, AboutProvider aboutProvider)
  {
    LocalizedString productAbout = aboutProvider.getProductAbout(selectedDictionaryId);
    if( TextUtils.isEmpty(productAbout.get()) )
    {
      String appAbout = aboutProvider.getAppAbout().get();
      if( !TextUtils.isEmpty(appAbout) )
      {
        return appAbout;
      }
      else
      {
        return getAboutPageTemplateFromAsset();
      }
    }
    else
    {
      return productAbout.get();
    }
  }

  private String getAboutPageTemplateFromAsset()
  {
    StringBuilder resString = new StringBuilder();
    try (BufferedReader reader = new BufferedReader(new InputStreamReader(
            getContext().getAssets().open("about_page.html"), StandardCharsets.UTF_8)))
    {
      String line;
      while ((line = reader.readLine()) != null)
      {
        resString.append(line);
      }
    }
    catch (IOException ignored) { }
    return resString.toString();
  }
}
