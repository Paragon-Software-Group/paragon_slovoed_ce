package com.paragon_software.information_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.EnumSet;
import java.util.Objects;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;

public class InformationItem
{
  @NonNull
  private final InformationItemType type;
  @Nullable
  private final String              text;
  @Nullable
  private Dictionary.DictionaryId dictionaryId;
  @Nullable
  private ArticleItem articleItem;

  private InformationItem( @NonNull InformationItemType type )
  {
    this(type, null);
  }

  private InformationItem( @NonNull InformationItemType type, @Nullable String text )
  {
    this.type = type;
    this.text = text;
  }

  @Nullable
  public String getText()
  {
    if ( null != articleItem )
    {
      return articleItem.getShowVariantText();
    }
    else
    {
      return text;
    }
  }

  @Nullable
  public static InformationItem create( @NonNull InformationItemType type, @Nullable String text,
                                        @Nullable Dictionary.DictionaryId selectedDictionaryId )
  {
    InformationItem informationItem = null;
    if ( EnumSet.of(InformationItemType.PRIVACY_POLICY, InformationItemType.EULA, InformationItemType.ABOUT, InformationItemType.RATE_APP)
                .contains(type) )
    {
      if ( null != text )
      {
        informationItem = new InformationItem(type, text);
        informationItem.setDictionaryId(selectedDictionaryId);
      }
      else
      {
        throw new IllegalArgumentException("Cant initialize information item (" + type + ") without text");
      }
    }
    return informationItem;
  }

  @NonNull
  public static InformationItem createAdditionalArticleItem( @NonNull ArticleItem articleItem )
  {
    InformationItem informationItem = new InformationItem(InformationItemType.ADDITIONAL_ARTICLES);
    informationItem.setArticleItem(articleItem);
    return informationItem;
  }

  @NonNull
  public InformationItemType getType()
  {
    return type;
  }

  private void setDictionaryId( @Nullable Dictionary.DictionaryId dictionaryId )
  {
    this.dictionaryId = dictionaryId;
  }

  @Nullable
  Dictionary.DictionaryId getDictionaryId()
  {
    return dictionaryId;
  }

  private void setArticleItem( @Nullable ArticleItem articleItem )
  {
    this.articleItem = articleItem;
  }

  @Nullable
  ArticleItem getArticleItem()
  {
    return articleItem;
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

    InformationItem that = (InformationItem) o;

    if ( type != that.type )
    {
      return false;
    }
    if ( !Objects.equals(text, that.text) )
    {
      return false;
    }
    //noinspection SimplifiableIfStatement
    if ( !Objects.equals(dictionaryId, that.dictionaryId) )
    {
      return false;
    }
    return !Objects.equals(articleItem, that.articleItem);
  }

  @Override
  public int hashCode()
  {
    int result = type.hashCode();
    result = 31 * result + ( text != null ? text.hashCode() : 0 );
    result = 31 * result + ( dictionaryId != null ? dictionaryId.hashCode() : 0 );
    result = 31 * result + ( articleItem != null ? articleItem.hashCode() : 0 );
    return result;
  }
}
