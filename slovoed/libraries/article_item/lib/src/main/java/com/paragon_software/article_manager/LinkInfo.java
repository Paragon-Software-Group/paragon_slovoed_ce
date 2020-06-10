package com.paragon_software.article_manager;

public class LinkInfo {
  
  final int mListId;
  final int mWordIndex;
  final String mDictId;
  final String mKey;
  
  public LinkInfo(int listId, int wordIndex, String dictId, String key) {
    this.mListId = listId;
    this.mWordIndex = wordIndex;
    this.mDictId = dictId;
    this.mKey = key;
  }
  
  public int getListId() {
    return mListId;
  }
  
  public int getWordIndex() {
    return mWordIndex;
  }
  
  public String getDictId() {
    return mDictId;
  }
  
  public String getKey() {
    return mKey;
  }
}
