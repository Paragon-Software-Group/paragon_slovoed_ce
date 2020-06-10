package com.paragon_software.flash_cards_manager;

import androidx.annotation.Nullable;

import java.io.Serializable;

import com.google.gson.annotations.SerializedName;
import com.paragon_software.native_engine.PersistentArticle;

class PersistFlashcard implements Serializable

{
  // WARNING!!! Don't change @SerializedName values. They already
  // was used in persistent storage of real users devices.
  // (it has so strange names because of ProGuard)
  @SerializedName("a")
  final boolean           checked;
  @SerializedName("b")
  final long              showCount;
  @SerializedName("c")
  final long              rightAnsCount;
  @SerializedName("d")
  @Nullable
  final PersistentArticle front;
  @SerializedName("e")
  @Nullable
  final PersistentArticle back;

  PersistFlashcard( boolean checked, long showCount, long rightAnsCount, @Nullable PersistentArticle front, @Nullable PersistentArticle back )
  {
    this.checked = checked;
    this.showCount = showCount;
    this.rightAnsCount = rightAnsCount;
    this.front = front;
    this.back = back;
  }
}
