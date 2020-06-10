package com.paragon_software.engine.nativewrapper;

import androidx.annotation.NonNull;

import java.util.Arrays;
import java.util.Collections;
import java.util.EnumSet;
import java.util.Set;

public enum ListType
{
  Unknown(0x000),
  Dictionary(0x001, Group.Main),
  Catalog(0x002, Group.Main),
  AdditionalInfo(0x003, Group.Additional),
  RegularSearch(0x004),
  Sound(0x005),
  FullTextSearchBase(0x100, 0x10F, Group.Fts),
  FullTextSearchHeadword(0x110, 0x11F, Group.Fts),
  FullTextSearchContent(0x120, 0x12F, Group.Fts),
  FullTextSearchTranslation(0x130, 0x13F, Group.Fts),
  FullTextSearchExample(0x140, 0x14F, Group.Fts),
  FullTextSearchDefinition(0x150, 0x15F, Group.Fts),
  FullTextSearchPhrase(0x160, 0x16F, Group.Fts),
  FullTextSearchIdiom(0x170, 0x1FE, Group.Fts),
  FullTextSearchLast(0x1FF, Group.Fts),
  Hidden(0x200),
  DictionaryForSearch(0x201),
  MorphologyBaseForm(0x202),
  MorphologyInflectionForm(0x203),
  GrammaticTest(0x204),
  SpecialAdditionalInfo(0x300, 0x3FF, Group.Additional),
  MergedDictionary(0x400),
  SpecialAdditionalInteractiveInfo(0x500, 0x5FF),
  MorphologyArticles(0x600),
  ArticlesHideInfo(0x601),
  GameArticles(0x602),
  FlashCardsFront(0x603),
  FlashCardsBack(0x604),
  InApp(0x605, Group.Main),
  FullTextAuxiliary(0x606),
  TextBook(0x607),
  Tests(0x608),
  SubjectIndex(0x609),
  PopupArticles(0x60A),
  SimpleSearch(0x60B),
  DictionaryUsageInfo(0x60C),
  CustomList(0x60D),
  SlideShow(0x60E),
  Map(0x60F),
  KES(0x610),
  FC(0x611),
  Atomic(0x612),
  PageNumerationIndex(0x613),
  BinaryResource(0x614),
  ExternResourcePriority(0x615, 0x624),
  ExternBaseName(0x625),
  StructuredMetadataStrings(0x626),
  CSSDataStrings(0x627),
  ArticleTemplates(0x628),
  AuxiliarySearchList(0x629),
  Enchiridion(0x62A),
  WordOfTheDay(0x62B),
  PreloadedFavourites(0x62C);

  public enum Group
  {
    Main,
    Fts,
    Additional
  }

  @NonNull
  private static final ListType[] all = ListType.values();

  static
  {
    for ( int i = 0 ; i < all.length ; i++ )
    {
      if ( i > 0 )
      {
        if ( all[i].start <= all[i - 1].end )
        {
          throw new ExceptionInInitializerError(all[i].toString());
        }
      }
    }
  }

  @NonNull
  private final Set< Group > groups;

  private final int start;
  private final int end;

  ListType( int _start, int _end, Group... _groups )
  {
    groups = arrayToSet(_groups);
    start = _start;
    end = _end;
    if ( start >= end )
    {
      throw new ExceptionInInitializerError();
    }
  }

  ListType( int n, Group... _groups )
  {
    groups = arrayToSet(_groups);
    start = end = n;
  }

  boolean belongsToGroup( @NonNull Group group )
  {
    return groups.contains(group);
  }

  @NonNull
  static ListType valueOf( int n )
  {
    ListType res = null;
    int first = 0;
    int last = all.length - 1;
    while ( first <= last )
    {
      int middle = ( first + last ) / 2;
      if ( all[middle].end < n )
      {
        first = middle + 1;
      }
      else if ( all[middle].start > n )
      {
        last = middle - 1;
      }
      else
      {
        res = all[middle];
        break;
      }
    }
    return ( res != null ) ? res : Unknown;
  }

  @NonNull
  private static < T extends Enum< T > > Set< T > arrayToSet( @NonNull T[] array )
  {
    Set< T > res;
    if ( array.length == 0 )
    {
      res = Collections.emptySet();
    }
    else if ( array.length == 1 )
    {
      res = Collections.singleton(array[0]);
    }
    else
    {
      res = EnumSet.copyOf(Arrays.asList(array));
    }
    return res;
  }
}
