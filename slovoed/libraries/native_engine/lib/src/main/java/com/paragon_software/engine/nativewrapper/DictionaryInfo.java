package com.paragon_software.engine.nativewrapper;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.native_engine.DictionaryInfoAPI;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.ArrayList;
import java.util.List;

public class DictionaryInfo extends DictionaryInfoAPI
{

  public final int majorVersion;
  public final int minorVersion;

  final int mainListCount;

  @NonNull
  private final List<LocalizedString> localizedStrings;

  DictionaryInfo(Context context,  @NonNull NativeFunctions functions )
  {
    int[] version = (int[]) functions.call(NativeFunctions.PUBLIC.GET_DICTIONARY_VERSION);
    majorVersion = version[0];
    minorVersion = version[1];

    mainListCount = (int) functions.call(NativeFunctions.PUBLIC.GET_LIST_COUNT);
    localizedStrings = buildLocalizedStrings(context, functions);
  }

  @Override
  @NonNull
  public LocalizedString getString(@NonNull Name name )
  {
    return localizedStrings.get(name.ordinal());
  }

  private static List<LocalizedString> buildLocalizedStrings(Context context, NativeFunctions functions )
  {
    int n = Name.values().length;
    Object[] builders = new LocalizedStringBuilder[n];
    for ( int i = 0 ; i < n ; i++ )
      builders[i] = new LocalizedStringBuilder(context);
    functions.call(NativeFunctions.PUBLIC.GET_DICTIONARY_LOCALIZED_STRINGS, builders);
    List<LocalizedString> res = new ArrayList<>(n);
    for ( Object builder : builders )
      res.add(((LocalizedStringBuilder) builder).build());
    return res;
  }
}
