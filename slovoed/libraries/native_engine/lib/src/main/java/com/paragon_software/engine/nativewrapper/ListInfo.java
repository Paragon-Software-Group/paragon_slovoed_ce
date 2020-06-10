package com.paragon_software.engine.nativewrapper;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.ArrayList;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

public class ListInfo
{
  //  public static final int SHORT_NAME = 0;
  //  public static final int FULL_NAME  = 1;
  public enum Name
  {
    Short,
    Full
  }

  @NonNull
  private static final WordVariant[] ALL_VARIANTS = WordVariant.values();

  @NonNull
  public final ListType listType;

  public final int listTypeOffset;
  public final int languageFrom;
  public final int languageTo;

  @NonNull
  private final List<LocalizedString> localizedStrings;
  @NonNull
  private final Map< WordVariant, Integer > variants;

  ListInfo(Context context,  @NonNull NativeFunctions functions, int listIndex, int listIndexForStrings )
  {
    int[] typeInfo = (int[]) functions.call(NativeFunctions.PUBLIC.GET_LIST_TYPE, listIndex);
    listType = ListType.valueOf(typeInfo[0]);
    listTypeOffset = typeInfo[1];

    int[] languages = (int[]) functions.call(NativeFunctions.PUBLIC.GET_LIST_LANGUAGES, listIndex);
    languageFrom = languages[0];
    languageTo = languages[1];

    localizedStrings = buildLocalizedStrings(context, functions, listIndexForStrings);
    variants = buildVariants(functions, listIndex);
  }

  @NonNull
  public LocalizedString getString(@NonNull Name name )
  {
    return localizedStrings.get(name.ordinal());
  }

  @NonNull
  public int[] getVariants( @NonNull WordVariant[] values )
  {
    int[] res = new int[values.length];
    for ( int i = 0 ; i < values.length ; i++ )
    {
      Integer n = variants.get(values[i]);
      if ( n != null )
      {
        res[i] = n;
      }
      else
      {
        res[i] = -1;
      }
    }
    return res;
  }

  private static List<LocalizedString> buildLocalizedStrings(Context context, NativeFunctions functions, int listIndex )
  {
    int n = Name.values().length;
    Object[] builders = new LocalizedStringBuilder[n];
    for ( int i = 0 ; i < n ; i++ )
      builders[i] = new LocalizedStringBuilder(context);
    functions.call(NativeFunctions.PUBLIC.GET_LIST_LOCALIZED_STRINGS, listIndex, builders);
    List<LocalizedString> res = new ArrayList<>(n);
    for ( Object builder : builders )
      res.add(((LocalizedStringBuilder) builder).build());
    return res;
  }

  private static Map< WordVariant, Integer > buildVariants( NativeFunctions functions, int listIndex )
  {
    int[] variants = (int[]) functions.call(NativeFunctions.PUBLIC.GET_LIST_VARIANTS, listIndex);
    EnumMap< WordVariant, Integer > res = new EnumMap<>(WordVariant.class);
    for ( int i = 0 ; i < variants.length ; i++ )
    {
      res.put(ALL_VARIANTS[variants[i]], i);
    }
    return res;
  }
}
