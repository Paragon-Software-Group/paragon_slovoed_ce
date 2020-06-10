package com.paragon_software.dictionary_manager_ui;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.arch.core.util.Function;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.LanguageStrings;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.Arrays;

class LanguageArrayAdapter extends ArrayAdapter<Integer>
{
  @LayoutRes
  private static final int DROPDOWN_LAYOUT_ID = android.R.layout.simple_spinner_dropdown_item;

  @StringRes
  private static final int ALL_LANGUAGES_STRING_ID = R.string.dictionary_manager_ui_language_all;

  private final LayoutInflater layoutInflater;

  @NonNull
  private final Integer[] data;

  @NonNull
  private final Function<LanguageStrings, LocalizedString> lambda;

  LanguageArrayAdapter(Context context, @NonNull Integer[] data, @NonNull Function<LanguageStrings, LocalizedString> lambda )
  {
    super(context, DROPDOWN_LAYOUT_ID, data);
    this.layoutInflater = LayoutInflater.from(context);
    this.data = data;
    this.lambda = lambda;
  }

  @NonNull
  @Override
  public View getView(int position, View convertView, @NonNull ViewGroup parent) {
    if(convertView == null)
      convertView = layoutInflater.inflate(DROPDOWN_LAYOUT_ID, parent, false);
    Integer language = getItem(position);
    if(convertView instanceof TextView) {
      TextView textView = (TextView) convertView;
      DictionaryManagerAPI dictionaryManager = DictionaryManagerHolder.getManager();
      if(language != null)
        textView.setText(lambda.apply(dictionaryManager.getLanguageStrings(getContext(), language)).get());
      else
        textView.setText(ALL_LANGUAGES_STRING_ID);
    }
    return convertView;
  }

  @Override
  public View getDropDownView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
    return getView(position, convertView, parent);
  }

  boolean isCurrentDataEquals(@NonNull Integer[] anotherData ) {
      return Arrays.equals(data, anotherData);
  }
}
