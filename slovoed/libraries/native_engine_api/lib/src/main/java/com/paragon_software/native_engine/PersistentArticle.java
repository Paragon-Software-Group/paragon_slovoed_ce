package com.paragon_software.native_engine;

import androidx.annotation.NonNull;

import java.io.Serializable;

import com.google.gson.annotations.SerializedName;
import com.paragon_software.article_manager.ArticleItem;


public class PersistentArticle implements Serializable
{
  // WARNING!!! Don't change @SerializedName values. They already
  // was used in persistent storage of real users devices.
  // (it has so strange names because of ProGuard)
  @SerializedName("a")
  private final String dictId;
  @SerializedName("b")
  private final byte[] historyElement;

  public PersistentArticle( @NonNull ArticleItem articleItem )
  {
    dictId = articleItem.getDictId().toString();
    historyElement = articleItem.getHistoryElement();
  }

  public String getDictId()
  {
    return dictId;
  }

  public byte[] getHistoryElement()
  {
    return historyElement;
  }
}
