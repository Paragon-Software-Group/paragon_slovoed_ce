package com.paragon_software.about_manager;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.EngineInformationAPI;

public class AboutManagerFactory implements AboutManagerFactoryAPI
{
  @Override
  public AboutManagerAPI create( @NonNull DictionaryManagerAPI dictionaryManager,
                                 @NonNull EngineInformationAPI engineInformation )
  {
    return new AboutManagerParagon(dictionaryManager, engineInformation);
  }
}
