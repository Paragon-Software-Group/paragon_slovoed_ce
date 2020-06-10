package com.paragon_software.article_manager;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryPrice;
import com.paragon_software.dictionary_manager.IDictionaryIcon;

class DictionaryInfo
{
  String                       title                    = null;
  IDictionaryIcon              icon                     = null;
  Dictionary.DICTIONARY_STATUS edition                  = null;
  DictionaryPrice              price                    = null;
  boolean                      hasHideOrSwitchBlocks    = false;
  boolean                      isFullWordBaseAccessible = false;
  int                          trialLengthInMinutes     = 0;
}
