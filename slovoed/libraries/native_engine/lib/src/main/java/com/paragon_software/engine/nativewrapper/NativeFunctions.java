package com.paragon_software.engine.nativewrapper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

class NativeFunctions
{
  private enum PRIVATE
  {
    //service
    GET_NATIVE_CONTEXT_POINTER_SIZE,
    GET_ENGINE_VERSION,
    OPEN_SDC_VIA_JAVA_READER,
    OPEN_SDC_VIA_DESCRIPTOR,
    CLOSE_SDC,
    ADD_MORPHO_VIA_JAVA_READER,
    ADD_MORPHO_VIA_DESCRIPTOR
  }

  enum PUBLIC
  {
    //service
    SWITCH_DIRECTION,

    //info
    GET_DICTIONARY_VERSION,
    GET_LIST_COUNT,
    GET_DICTIONARY_LOCALIZED_STRINGS,
    GET_LIST_TYPE,
    GET_LIST_LANGUAGES,
    GET_LIST_LOCALIZED_STRINGS,
    GET_LIST_VARIANTS,

    //fill
    GET_WORD_COUNT,
    GET_WORD_BY_LOCAL_INDEX,
    GET_REAL_WORD_LOCATION,
    GET_PATH_FROM_GLOBAL_INDEX,
    GET_PRELOADED_FAVORITES,
    GET_CURRENT_WORD_STYLIZED_VARIANT,
    GET_EXTERNAL_IMAGE,

    //search
    RESET_LIST,
    GET_WORD_BY_TEXT,
    FULL_TEXT_SEARCH,
    DID_YOU_MEAN_SEARCH,
    WILD_CARD_SEARCH,
    ANAGRAM_SEARCH,
    FUZZY_SEARCH,
    DELETE_ALL_SEARCH_LISTS,
    GET_BASE_FORMS,
    GET_WORD_REFERENCE_IN_LIST,
    GET_ARTICLE_LINKS,
    GET_QUERY_REFERENCE_IN_PHRASE,
    GET_WORD_REFERENCE,
    GET_HIGHLIGHT_WORD_REFERENCE_INCLUDING_WHITESPACE,
    GET_QUERY_HIGHLIGHT_DATA,

    //article
    TRANSLATE,
    GET_HISTORY_ELEMENT_BY_GLOBAL_INDEX,
    GET_ARTICLE_INDEXES_BY_HISTORY_ELEMENT,
    SWIPE,
    STARTS_WITH,

    //sound
    GET_SOUND_INDEX_BY_WORD_GLOBAL_INDEX,
    GET_SOUND_INDEX_BY_TEXT,
    PLAY_SOUND_BY_INDEX,
    CONVERT_SPX;
  }

  private static final int NATIVE_POINTER_SIZE;
  private static final int PRIVATE_CALLS_COUNT = PRIVATE.values().length;
  static final int ENGINE_VERSION;
  static final int ENGINE_BUILD;

  private final byte[] mNativeContextPointer = new byte[NATIVE_POINTER_SIZE];

  private volatile boolean mClosed = false;

  static
  {
    System.loadLibrary("native-engine");
    byte[] nullArray = new byte[0];
    NATIVE_POINTER_SIZE = (Integer) ncall(PRIVATE.GET_NATIVE_CONTEXT_POINTER_SIZE, nullArray);
    int[] versionAndBuild = (int[]) ncall(PRIVATE.GET_ENGINE_VERSION, nullArray);
    ENGINE_VERSION = versionAndBuild[0];
    ENGINE_BUILD = versionAndBuild[1];
  }

  @Nullable
  static NativeFunctions openFromJavaReader( @NonNull JavaReader javaReader, String cacheDir)
  {
    NativeFunctions res = new NativeFunctions();
    int error = (Integer) ncall(PRIVATE.OPEN_SDC_VIA_JAVA_READER, res.mNativeContextPointer, javaReader.getCallbacks(), cacheDir);
    if ( error != 0 )
    {
      res = null;
    }
    return res;
  }

  @Nullable
  static NativeFunctions openFromDescriptor( int descriptor, long offset, long size, String cacheDir)
  {
    NativeFunctions res = new NativeFunctions();
    int error = (Integer) ncall(PRIVATE.OPEN_SDC_VIA_DESCRIPTOR, res.mNativeContextPointer, descriptor, offset, size, cacheDir);
    if ( error != 0 )
    {
      res = null;
    }
    return res;
  }

  int addMorphoFromDescriptor(int descriptor, long offset, long size)
  {
    return  (Integer) ncall(PRIVATE.ADD_MORPHO_VIA_DESCRIPTOR, mNativeContextPointer, descriptor, offset, size);
  }

  int addMorphoFromReader( @NonNull JavaReader javaReader)
  {
    return (Integer) ncall(PRIVATE.ADD_MORPHO_VIA_JAVA_READER, mNativeContextPointer, javaReader.getCallbacks());
  }

  int close()
  {
    int res = (Integer) ncall(PRIVATE.CLOSE_SDC, mNativeContextPointer);
    mClosed = true;
    return res;
  }

  interface Callback
  {
    @NonNull
    Object function( Object... args );
  }

  @NonNull
  Object call( PUBLIC id, Object... args )
  {
    if ( mClosed )
    {
      throw new IllegalStateException("dictionary already closed");
    }
    else
    {
      return ncall(PRIVATE_CALLS_COUNT + id.ordinal(), mNativeContextPointer, args);
    }
  }

  @NonNull
  private static Object ncall( PRIVATE id, @NonNull byte[] nativeContextPointer, Object... args )
  {
    return ncall(id.ordinal(), nativeContextPointer, args);
  }

  @SuppressWarnings( "unused" )
  @NonNull
  private static Object ncallback( Object function, @NonNull Object[] args )
  {
    Object res = 0;
    if ( function instanceof Callback )
    {
      res = ( (Callback) function ).function(args);
    }
    return res;
  }

  @NonNull
  private static native Object ncall( int id, @NonNull byte[] nativeContextPointer, Object... args );
}
