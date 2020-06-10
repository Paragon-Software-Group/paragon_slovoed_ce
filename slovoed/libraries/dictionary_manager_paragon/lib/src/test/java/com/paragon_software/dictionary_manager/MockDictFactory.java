package com.paragon_software.dictionary_manager;

import android.content.Context;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.components.WordBaseComponent;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Currency;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import edu.emory.mathcs.backport.java.util.Collections;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.withSettings;

public abstract class MockDictFactory implements IDictionaryManagerFactory
{
  private Map< String, Dictionary.DictionaryId > featureToDictionaryId = new HashMap<>();

  public MockDictFactory()
  {
  }

  public MockDictFactory( Map< String, Dictionary.DictionaryId > featureToDictionaryId )
  {
    this.featureToDictionaryId = featureToDictionaryId;
  }

  @NonNull
  public static Dictionary.DictionaryId[] getDictionariyIds( @NonNull String[] dictionaryIds )
  {
    Dictionary.DictionaryId[] ids = new Dictionary.DictionaryId[dictionaryIds.length];
    int i = 0;
    for ( String id : dictionaryIds )
    {
      ids[i++] = new Dictionary.DictionaryId(id);
    }
    return ids;
  }

  @NonNull
  @Override
  public Dictionary[] getDictionaries(@NonNull Context context, @NonNull Dictionary.DictionaryId[] dictionaryIds ) {
    String[][] langs = new String[][]{
            {"Greek", "Polish"}, {"Russian", "German"}, {"Swedish", "Bulgarian"}, {"Italian", "Russian"}, {"English", "English"},
            {"Swedish", "Bulgarian"}, {"English", "Russian"}, {"English", "German"}, {"English", "Italian"},
            {"English", "Swedish"}, {"Swedish", "Bulgarian"}, {"Italian", "Russian"}, {"French", "German"},
            {"Swedish", "Bulgarian"}, {"English", "Russian"}, {"English", "German"}, {"English", "Italian"},
            {"English", "Swedish"}, {"Swedish", "Bulgarian"}, {"Italian", "Russian"}, {"French", "German"},
            {"Swedish", "Bulgarian"}, {"English", "Russian"}, {"English", "German"}, {"English", "Italian"},
            {"English", "Swedish"}};

    Dictionary[] ret = new Dictionary[dictionaryIds.length];
    int i = 0;
    for (Dictionary.DictionaryId dictionaryId : dictionaryIds) {
      String featureName = getFeatureNameByDictId(dictionaryId);
      List<DictionaryComponent> components = new ArrayList<>();
      WordBaseComponent wordBaseDemo = new WordBaseComponent("", 123456, true, "1.1", "russ", "engl", "SDCID", new FeatureName(featureName + "_demo"), "123");
      components.add(wordBaseDemo);
      WordBaseComponent wordBaseFull = new WordBaseComponent("", 321321, false, "1.1", "russ", "engl", "SDCID", new FeatureName(featureName), "123");
      components.add(wordBaseFull);
      DictIcon icon = Mockito.mock(DictIcon.class, withSettings().defaultAnswer(CALLS_REAL_METHODS));
      MarketingData marketingData = Mockito.spy(MarketingData.class);
      when(marketingData.getDefaultPriceValue(any(Currency.class))).thenReturn(2L);
      when(marketingData.getShoppings()).thenReturn(Collections.singletonList(new Shopping("id", Collections.singletonMap(Currency.getInstance("USD"), 1L), null)));

      ret[i++] = new Dictionary.Builder(dictionaryId, LocalizedString.from("title" + i)
              , LocalizedString.from("desc" + i), icon)
              .setDictionaryComponents(components)
              .setMarketingData(marketingData)
              .setDirections(createDirections(langs[i][0], langs[i][1]))
              .build();
    }
    return ret;
  }

  @NonNull
  @Override
  public DictionaryPack[] getDictionaryPacks(@NonNull final Context context,
                                             @NonNull final Dictionary.DictionaryId[] dictionariesId){
    return new DictionaryPack[0];
  }

  private Collection<Dictionary.Direction> createDirections(@NonNull String from, @NonNull String to) {
    List<Dictionary.Direction> res = new ArrayList<>(2);
    int l1 = Language.getLangCodeFromShortForm(from.toLowerCase());
    int l2 = Language.getLangCodeFromShortForm(to.toLowerCase());
    res.add(new Dictionary.Direction(l1, l2, null));
    if(l1 != l2)
      res.add(new Dictionary.Direction(l2, l1, null));
    return res;
  }

  private String getFeatureNameByDictId( Dictionary.DictionaryId dictionaryId )
  {
    for ( String featureName : featureToDictionaryId.keySet() )
    {
      if ( featureToDictionaryId.get(featureName).equals(dictionaryId) )
      {
        return featureName;
      }
    }
    return "any-feature-name";
  }

  private abstract class DictIcon implements IDictionaryIcon
  {
  }
}