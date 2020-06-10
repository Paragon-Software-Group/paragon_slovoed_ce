/*
 * search_all_dictionary
 *
 *  Created on: 16.06.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.engine;

final class NativeFunctions
{
  static final int                                                                                LOAD_DICTIONARY  = 1;
  static final int                                                                                CLOSE_DICTIONARY = 2;
  static final int                                                                                SEARCH_WORD      = 3;
  static final java.util.concurrent.atomic.AtomicBoolean                                          stopSearch       =
      new java.util.concurrent.atomic.AtomicBoolean(false);
  static final java.util.concurrent.atomic.AtomicReference< java.util.concurrent.CountDownLatch > stopSync         =
      new java.util.concurrent.atomic.AtomicReference<>(null);

  static
  {
    System.loadLibrary("native-engine");
  }

  static ReadResult readerFunction( Object object, int size, int offset )
  {
    if ( object instanceof NativeEngine.InputStreamWrapper )
    {
      NativeEngine.InputStreamWrapper stream = ( (NativeEngine.InputStreamWrapper) object );
      int result = stream.read(offset, size);
      return new ReadResult(result, stream.tempBuffer);
    }
    return null;
  }

  @SuppressWarnings( "unused" )
  static Object initDictionaryOptions( long id, String productName, String dictionaryName, String dictionaryNameShort,
                                       String dictionaryClass, String dictionaryLanguagePair,
                                       String dictionaryLanguagePairShort, String authorName )
  {
    return new com.paragon_software.search_all_dictionary.dictionary_manager.Dictionary.DictionaryOptions(id,
                                                                                                          productName,
                                                                                                          dictionaryName,
                                                                                                          dictionaryNameShort,
                                                                                                          dictionaryClass,
                                                                                                          dictionaryLanguagePair,
                                                                                                          dictionaryLanguagePairShort,
                                                                                                          authorName);
  }

  @SuppressWarnings( "unused" )
  static void searchResult( Object arg1, String arg )
  {
    if ( arg1 instanceof Callback )
    {
      ( (Callback) ( arg1 ) ).function(new com.paragon_software.search_all_dictionary.search_manager.SearchResult(arg));
    }
  }

  static native Object native_1( int id, final Object arg1 );

  static native Object native_2( int id, final long arg1, final Object arg2, final Object arg3 );

  interface Callback
  {
    void function( final Object word );
  }

  private static final class ReadResult
  {
    final int    size;
    final byte[] data;

    ReadResult( int size, byte[] data )
    {
      this.size = size;
      this.data = data;
    }
  }
}
