package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import java.io.Serializable;

import com.google.gson.annotations.SerializedName;
import com.paragon_software.native_engine.PersistentArticle;

class PersistFlashcardBox implements Serializable
{
  // WARNING!!! Don't change @SerializedName values. They already
  // was used in persistent storage of real users devices.
  // (it has so strange names because of ProGuard)
  @SerializedName("a")
  final PersistentArticle  parent;
  @SerializedName("b")
  final PersistFlashcard[] cards;

  PersistFlashcardBox( @NonNull PersistentArticle parent, @NonNull PersistFlashcard[] cards )
  {
    this.parent = parent;
    this.cards = cards;
  }
}
