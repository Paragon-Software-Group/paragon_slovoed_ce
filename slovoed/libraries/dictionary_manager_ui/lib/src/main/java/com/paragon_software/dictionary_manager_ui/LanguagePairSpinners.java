package com.paragon_software.dictionary_manager_ui;

import android.content.Context;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Spinner;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.arch.core.util.Function;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.LanguageStrings;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

class LanguagePairSpinners
{
  private final Spinner                       spinnerFrom;
  private final Spinner                       spinnerTo;
  private List< LanguagePair >                languagePairs = new ArrayList<>();
  private final OnLanguagePairChangedListener listener;
  private       LanguageArrayAdapter          adapterFrom;
  private       LanguageArrayAdapter          adapterTo;

  private final Comparator< Integer > comparatorFrom;
  private final Comparator< Integer > comparatorTo;

  private static class LanguageComparator implements Comparator< Integer > {
    private Context appContext;
    private Function<LanguageStrings, LocalizedString> lambda;

    LanguageComparator(Context _context, Function<LanguageStrings, LocalizedString> _lambda) {
      appContext = _context.getApplicationContext();
      lambda = _lambda;
    }

    @Override
    public int compare( Integer left, Integer right ) {
      if ( (left == null) || (right == null) )
        return (left == null) ? Integer.MIN_VALUE : Integer.MAX_VALUE;
      else {
        DictionaryManagerAPI dictionaryManager = DictionaryManagerHolder.getManager();
        String leftStr = lambda.apply(dictionaryManager.getLanguageStrings(appContext, left)).get();
        String rightStr = lambda.apply(dictionaryManager.getLanguageStrings(appContext, right)).get();
        return leftStr.compareTo(rightStr);
      }
    }
  }

  LanguagePairSpinners( Context context,
                        @NonNull Spinner spinnerFrom, @NonNull Spinner spinnerTo,
                        OnLanguagePairChangedListener listener )
  {
    this.spinnerFrom = spinnerFrom;
    this.spinnerTo = spinnerTo;
    this.listener = listener;
    this.comparatorFrom = new LanguageComparator(context, LanguageStrings::getDirectionFrom);
    this.comparatorTo = new LanguageComparator(context, LanguageStrings::getDirectionTo);
  }

  void fill(LanguagePair initially)
  {
    if (!setLanguagePairs()) return;

    {
      Set<Integer> fromLangs = getFromLangs(null);
      fromLangs.add(null);
      Integer[] array = fromLangs.toArray(new Integer[]{});
      Arrays.sort(array, comparatorFrom);
      adapterFrom = new LanguageArrayAdapter(spinnerFrom.getContext(), array, LanguageStrings::getDirectionFrom);
      spinnerFrom.setAdapter(adapterFrom);
    }

    {
      Set<Integer> toLangs = getToLangs(null);
      toLangs.add(null);
      Integer[] array = toLangs.toArray(new Integer[]{});
      Arrays.sort(array, comparatorTo);
      adapterTo = new LanguageArrayAdapter(spinnerTo.getContext(), array, LanguageStrings::getDirectionTo);
      spinnerTo.setAdapter(adapterTo);
    }

    Integer initiallyLangTo = null;
    if (null != initially) {
      int indexFrom = null == initially.getFrom() ? 0 : findInitialFromIndex(initially.getFrom());
      spinnerFrom.setSelection(indexFrom);
      initiallyLangTo = initially.getTo();
    }

    spinnerFrom.setOnItemSelectedListener(getSelectedListenerFromLang(initiallyLangTo));
    spinnerTo.setOnItemSelectedListener(getSelectedListenerToLang());

  }

  private boolean setLanguagePairs() {
    DictionaryManagerAPI dictionaryManagerAPI = DictionaryManagerHolder.getManager();
    Collection<com.paragon_software.dictionary_manager.Dictionary> dictionaries = dictionaryManagerAPI.getDictionaries();

    List<LanguagePair> langPairs = new ArrayList<>();
    for (Dictionary dictionary : dictionaries)
      for(Dictionary.Direction direction : dictionary.getDirections())
        langPairs.add(new LanguagePair(direction.getLanguageFrom(), direction.getLanguageTo()));
    if (!languagePairs.equals(langPairs)) {
      languagePairs = langPairs;
      return true;
    } else return false;
  }

  private int findInitialFromIndex( Integer from )
  {
    Set< Integer > fromLangs = getFromLangs(null);
    fromLangs.add(null);
    Integer[] array = fromLangs.toArray(new Integer[]{});
    Arrays.sort(array, comparatorFrom);
    return searchInArray(from, array);
  }

  private Set< Integer > getFromLangs( Integer toLanguage )
  {
    HashSet< Integer > langs = new HashSet<>();
    for ( LanguagePair pair : languagePairs )
    {
      if ( (null == toLanguage) || Objects.equals(pair.to, toLanguage))
      {
        langs.add(pair.from);
      }
    }
    return langs;
  }

  @NonNull
  private AdapterView.OnItemSelectedListener getSelectedListenerFromLang( final Integer initiallyLangTo )
  {
    return new AdapterView.OnItemSelectedListener()
    {
      Integer initiallySelectedLanguage = initiallyLangTo;
      @Override
      public void onItemSelected( AdapterView< ? > adapterView, View view, int i, long l )
      {
        updateAdapterToLangs(adapterFrom.getItem(i), initiallySelectedLanguage);
        initiallySelectedLanguage = null;
      }

      @Override
      public void onNothingSelected( AdapterView< ? > adapterView )
      {
      }
    };
  }

  private Set< Integer > getToLangs( Integer fromLanguage )
  {
    HashSet< Integer > langs = new HashSet<>();
    for ( LanguagePair pair : languagePairs )
    {
      if ( (null == fromLanguage) || Objects.equals(pair.from, fromLanguage) )
      {
        langs.add(pair.to);
      }
    }
    return langs;
  }

  @NonNull
  private AdapterView.OnItemSelectedListener getSelectedListenerToLang()
  {
    return new AdapterView.OnItemSelectedListener()
    {
      @Override
      public void onItemSelected( AdapterView< ? > adapterView, View view, int i, long l )
      {
        updateAdapterFromLangs(adapterTo.getItem(i));
      }

      @Override
      public void onNothingSelected( AdapterView< ? > adapterView )
      {
      }
    };
  }

  private void updateAdapterToLangs( Integer selectedFromLanguage, Integer initiallySelectedLangTo)
  {
    Integer previousSelectedItem = initiallySelectedLangTo;
    if ( (previousSelectedItem == null) )
    {
      int selectedItemPosition = spinnerTo.getSelectedItemPosition();
      previousSelectedItem = adapterTo.getItem(selectedItemPosition);
    }

    Set< Integer > toLangs = getToLangs(selectedFromLanguage);
    if ( toLangs.size() > 1 )
    {
      toLangs.add(null);
    }
    Integer[] array = toLangs.toArray(new Integer[]{});

    Arrays.sort(array, comparatorTo);
    int selectedPosition = searchInArray(previousSelectedItem, array);
    if ( (initiallySelectedLangTo != null) || !adapterTo.isCurrentDataEquals(array) )
    {
      adapterTo =
          new LanguageArrayAdapter(spinnerTo.getContext(), array, LanguageStrings::getDirectionTo);
      spinnerTo.setAdapter(adapterTo);
      if ( selectedPosition > 0 && selectedPosition < array.length )
      {
        spinnerTo.setSelection(selectedPosition);
      }
    }
  }

  private void updateAdapterFromLangs( Integer selectedToLanguage )
  {
    Integer selectedFromLanguage = adapterFrom.getItem(spinnerFrom.getSelectedItemPosition());
    listener.onLanguagePairChanged(selectedFromLanguage, selectedToLanguage);
    if ( (selectedFromLanguage != null) )
    {
      return;
    }

    Set< Integer > fromLangs = getFromLangs(selectedToLanguage);
    fromLangs.add(null);
    Integer[] array = fromLangs.toArray(new Integer[]{});

    Arrays.sort(array, comparatorFrom);
    if ( !adapterFrom.isCurrentDataEquals(array) )
    {
      adapterFrom =
          new LanguageArrayAdapter(spinnerTo.getContext(), array, LanguageStrings::getDirectionFrom);
      spinnerFrom.setAdapter(adapterFrom);
    }
  }

  private int searchInArray( Integer previousSelectedItem, Integer[] array )
  {
    int selectedPosition = Integer.MIN_VALUE;

    for ( int i = 0 ; i < array.length ; i++ )
    {
      if ( Objects.equals(array[i], previousSelectedItem) )
      {
        selectedPosition = i;
      }
    }
    return selectedPosition;
  }

  interface OnLanguagePairChangedListener
  {
    void onLanguagePairChanged( Integer fromLanguage, Integer toLanguage );
  }

  static class LanguagePair
  {
    @Nullable
    private final Integer from;

    @Nullable
    private final Integer to;

    LanguagePair( @Nullable Integer from, @Nullable Integer to )
    {
      this.from = from;
      this.to = to;
    }

    @Nullable
    Integer getFrom()
    {
      return from;
    }

    @Nullable
    Integer getTo()
    {
      return to;
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;
      LanguagePair that = (LanguagePair) o;
      return Objects.equals(getFrom(), that.getFrom()) &&
              Objects.equals(getTo(), that.getTo());
    }

    @Override
    public int hashCode() {
      return Objects.hash(getFrom(), getTo());
    }
  }
}
