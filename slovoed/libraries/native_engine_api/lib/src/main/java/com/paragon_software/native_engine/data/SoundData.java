package com.paragon_software.native_engine.data;

/**
 * Class for working with sound data from word and sound bases.
 */
public class SoundData
{
  /**
   * Array with a sound data bytes.
   */
  final byte[] data;

  /**
   * Sample rate of a sound data.
   */
  final int frequency;

  /**
   * Sound data encoding format.
   */
  final SoundFormat format;

  public SoundData( byte[] _data, int _frequency, int _format )
  {
    data = _data;
    frequency = _frequency;
    format = SoundFormat.valueOf(_format);
  }

  public byte[] getData()
  {
    return data;
  }

  public int getFrequency()
  {
    return frequency;
  }

  public SoundFormat getFormat()
  {
    return format;
  }

}
