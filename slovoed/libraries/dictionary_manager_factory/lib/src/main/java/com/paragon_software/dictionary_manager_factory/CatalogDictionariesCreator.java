package com.paragon_software.dictionary_manager_factory;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryPack;
import com.paragon_software.trial_manager.TrialManagerAPI;

import java.util.Map;

/**
 * Interface for classes creating array of {@link Dictionary} objects initialized by data
 * from PDAHPC catalog.
 */
public interface CatalogDictionariesCreator
{
  Dictionary[] createDictionaries();

  DictionaryPack[] createDictionaryPacks();

  void addFeaturesToTrialBuilder(@NonNull TrialManagerAPI.Builder trialBuilder,
                                 @NonNull Map< Dictionary.DictionaryId, Dictionary > allDictionaries);
}
