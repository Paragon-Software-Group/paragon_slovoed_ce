package com.paragon_software.native_engine.data;

/**
 * Enum with sound formats available in word and sound bases.
 */
public enum SoundFormat
{
  NONE(0),
  SPX(1),
  WAV(2),
  AMR(3),
  MP3(4),
  OGG(5);

  private int id;

  SoundFormat( int _id )
  {
    id = _id;
  }

  public static SoundFormat valueOf( int _id )
  {
    for ( SoundFormat format : SoundFormat.values() )
    {
      if ( format.id == _id )
      {
        return format;
      }
    }
    return NONE;
  }
}