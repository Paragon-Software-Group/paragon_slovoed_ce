package com.paragon_software.dictionary_manager_ui;

import android.util.Pair;

import com.paragon_software.dictionary_manager.DictionaryPrice;
import com.paragon_software.dictionary_manager.FeatureName;

interface BuyClickListener
{
  void onBuyClick( DictionaryPrice.PeriodSubscription period );
}
