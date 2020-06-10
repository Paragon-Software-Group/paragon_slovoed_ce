package com.paragon_software.utils_slovoed.convertValues;

public class ConverterBaseId {


  public static String convertOald10SoundBase(String soundBaseIdx) {
    //560f, 5611, 5610, 5612, 5613, 5614 разделенные звуковые базы для Ios которые соответствуют полным базам 560b, 560c, 5608
    switch (soundBaseIdx.toLowerCase()) {
      case "560f":
      case "5611":
        return "560b";
      case "5610":
      case "5612":
        return "560c";
      case "5613":
      case "5614":
        return "5608";
      default:
        return soundBaseIdx;
    }
  }

}
