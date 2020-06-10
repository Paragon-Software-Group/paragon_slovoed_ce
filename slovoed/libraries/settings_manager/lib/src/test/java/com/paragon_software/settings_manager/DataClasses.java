package com.paragon_software.settings_manager;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * Created by belyshov on 14/02/2018.
 */

class DataIntStringByteList implements Serializable
{
  private int dataInt;
  private String               dataString       = "";
  private byte[]               dataByteArray    = new byte[0];
  private ArrayList< Integer > dataIntegerArray = new ArrayList<>();

  static DataIntStringByteList getStubData()
  {
    DataIntStringByteList value = new DataIntStringByteList();
    value.dataInt = 0xdead;
    value.dataString = "0xdead";
    value.dataByteArray = new byte[4];
    value.dataByteArray[0] = 0xd;
    value.dataByteArray[1] = 0xe;
    value.dataByteArray[2] = 0xa;
    value.dataByteArray[3] = 0xd;
    value.dataIntegerArray = new ArrayList<>();
    for ( int i = 0 ; i < 100 ; ++i )
    {
      value.dataIntegerArray.add(i * i);
    }
    return value;
  }

  boolean equals( final DataIntStringByteList other )
  {
    return other.dataInt == this.dataInt && other.dataString.equals(this.dataString) && this.dataIntegerArray
        .equals(other.dataIntegerArray) && Arrays.equals(this.dataByteArray, other.dataByteArray);
  }
}



