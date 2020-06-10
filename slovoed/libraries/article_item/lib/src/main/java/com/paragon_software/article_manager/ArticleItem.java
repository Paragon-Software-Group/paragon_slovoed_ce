package com.paragon_software.article_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.Serializable;
import java.util.List;

import com.paragon_software.dictionary_manager.Dictionary.DictionaryId;
import com.paragon_software.dictionary_manager.Dictionary.Direction;

public class ArticleItem implements Serializable
{
  @NonNull
  private final DictionaryId dictId;
  @Nullable
  private final Direction    direction;
  private final int listId;
  private final int globalIndex;

  /**
   * Main ArticleItem label. Used as main text in UI search result lists
   */
  @Nullable
  private final String      label;
  @Nullable
  private final String      searchQuery;
  @Nullable
  private final String partOfSpeech;
  @Nullable
  private final String numeration;

  private final boolean isLocked;
  private final LinkInfo[] linkInfo;

  /**
   * Additional ArticleItem label. Used as additional gray text below the main label in UI FTS search result ArticleItem groups
   */
  @Nullable
  private String      ftsHeadword;

  @Nullable
  private String      ftsAnchor;

  @NonNull
  private final List <WordReference> wordReferences;

  @Nullable
  private final byte[] historyElement;

  private boolean hasInternalSound;

  private ListType listType;

  @Nullable
  private String sortKey;

  private ArticleItem( @NonNull final Builder builder )
  {
    dictId = builder.dictId;
    listId = builder.listId;
    globalIndex = builder.globalIndex;
    direction = builder.direction;
    label = builder.label;
    searchQuery = builder.searchQuery;
    partOfSpeech = builder.partOfSpeech;
    numeration = builder.mNumeration;
    isLocked = builder.isLocked;
    ftsHeadword = builder.ftsHeadword;
    historyElement = builder.historyElement;
    hasInternalSound = builder.hasInternalSound;
    ftsAnchor = builder.ftsAnchor;
    wordReferences = builder.wordReferences;
    listType = builder.listType;
    sortKey = builder.sortKey;
    linkInfo = builder.linkInfo;
  }

  public @NonNull
  DictionaryId getDictId()
  {
    return dictId;
  }

  public @Nullable
  Direction getDirection()
  {
    return direction;
  }

  public int getListId()

  {
    return listId;
  }

  public int getGlobalIndex()
  {
    return globalIndex;
  }

  public @Nullable
  String getLabel()
  {
    return label;
  }

  public @Nullable
  String getSearchQuery()
  {
    return searchQuery;
  }
  
  public @Nullable
  String getPartOfSpeech() {
    return partOfSpeech;
  }

  public @Nullable
  String getNumeration() {
    return numeration;
  }

  public
  boolean isLocked() {
    return isLocked;
  }

  public @Nullable
  String getFtsHeadword()
  {
    return ftsHeadword;
  }

  public @Nullable
  String getFtsAnchor()
  {
    return ftsAnchor;
  }

  public @NonNull
  List <WordReference> getWordReferences()
  {
    return wordReferences;
  }

  @Nullable
  public byte[] getHistoryElement()
  {
    return historyElement;
  }

  public boolean isHasInternalSound()
  {
    return hasInternalSound;
  }

  public boolean isAdditional()
  {
    return ListType.AdditionalInfo.equals(listType);
  }

  public boolean isSpecialAdditionalInteractive()
  {
    return ListType.SpecialAdditionalInteractiveInfo.equals(listType);
  }

  public boolean isMorphologyArticle()
  {
    return ListType.MorphologyArticles.equals(listType);
  }

  public LinkInfo[] getLinkInfo() {
    return linkInfo;
  }

  public @Nullable
  String getShowVariantText()
  {
    String showText = ftsHeadword;
    if ( showText == null )
    {
      showText = label;
    }
    return showText;
  }

  public @Nullable
  String getSortKey()
  {
    return sortKey;
  }

  @Override
  public String toString()
  {
    StringBuilder res = new StringBuilder();
    res.append("article:").append(getDictId().toString()).append(':').append(getListId()).append(':')
       .append(getGlobalIndex());
    String l = getLabel();
    if ( l != null )
    {
      if ( l.length() > 0 )
      {
        res.append(':').append(l);
      }
    }
    return res.toString();
  }

  @Override
  public boolean equals( Object o )
  {
    if ( this == o )
    {
      return true;
    }
    if ( o == null || getClass() != o.getClass() )
    {
      return false;
    }

    ArticleItem item = (ArticleItem) o;

    return getListId() == item.getListId() && getGlobalIndex() == item.getGlobalIndex() && getDictId()
        .equals(item.getDictId());
  }

  @Override
  public int hashCode()
  {
    int result = getDictId().hashCode();
    result = 31 * result + getListId();
    result = 31 * result + getGlobalIndex();
    return result;
  }

  public static class Builder
  {
    @NonNull
    private final DictionaryId dictId;
    private final int listId;
    private final int globalIndex;

    @Nullable
    public String searchQuery;
    @Nullable
    private Direction    direction;
    @Nullable
    private String      label;
    @Nullable
    private String partOfSpeech;
    @Nullable
    private String mNumeration;

    private boolean isLocked;
    @Nullable
    private String      ftsHeadword;
    @Nullable
    private String      ftsAnchor;
    @Nullable
    private byte[] historyElement;
    private boolean hasInternalSound;
    @Nullable
    private String sortKey;
    private List <WordReference> wordReferences;

    private ListType listType;
    private LinkInfo[] linkInfo;

    public Builder( @NonNull DictionaryId _dictId, int _listId, int _globalIndex )
    {
      dictId = _dictId;
      listId = _listId;
      globalIndex = _globalIndex;
    }

    public Builder setDirection( @Nullable final Direction _direction )
    {
      direction = _direction;
      return this;
    }

    public Builder setLabel( @Nullable final String _label )
    {
      label = _label;
      return this;
    }

    public Builder setSearchQuery( @Nullable final String _searchQuery )
    {
      searchQuery = _searchQuery;
      return this;
    }

    public Builder setPartOfSpeech(String _partOfSpeech) {
      partOfSpeech = _partOfSpeech;
      return this;
    }


    public Builder setFtsHeadword( @Nullable final String _ftsHeadword )
    {
      ftsHeadword = _ftsHeadword;
      return this;
    }

    public Builder setFtsAnchor(@Nullable final String _ftsAnchor)
    {
      ftsAnchor = _ftsAnchor;
      return this;
    }

    public Builder setWordReferences(@NonNull final List<WordReference> _wordReferences)
    {
      wordReferences = _wordReferences;
      return this;
    }

    public Builder setHistoryElement( @Nullable final byte[] _historyElement )
    {
      historyElement = _historyElement;
      return this;
    }

    public Builder setHasInternalSound( boolean _hasInternalSound )
    {
      hasInternalSound = _hasInternalSound;
      return this;
    }

    public Builder setSortKey(String _sortKey)
    {
      sortKey = _sortKey;
      return this;
    }

      public Builder setListType(ListType listType)
      {
          this.listType = listType;
          return this;
      }

    @NonNull
    public ArticleItem build()
    {
      return new ArticleItem(this);
    }

	  public Builder setNumeration(String _numeration) {
      mNumeration = _numeration;
      return this;
	  }

	  public Builder setLocked(boolean _isLocked) {
      isLocked = _isLocked;
      return this;
	  }

    public Builder setLinkInfo(LinkInfo[] _linkInfo) {
      linkInfo = _linkInfo;
      return this;
    }
  }
}
