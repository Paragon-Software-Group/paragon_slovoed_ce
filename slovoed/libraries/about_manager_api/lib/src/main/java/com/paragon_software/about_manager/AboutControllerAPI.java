package com.paragon_software.about_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;

abstract class AboutControllerAPI
{
  abstract void setDictionaryId( @Nullable Dictionary.DictionaryId dictionaryId );

  @NonNull
  abstract AboutSpecs getAboutSpecs();
}
