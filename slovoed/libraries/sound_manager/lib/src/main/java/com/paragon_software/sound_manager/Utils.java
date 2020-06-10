package com.paragon_software.sound_manager;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

class Utils {

  private static String bytesToHexString(byte[] bytes) {
    final String HEX_ARRAY = "0123456789ABCDEF";

    if (bytes.length == 0) {
      return "";
    }

    StringBuilder hex = new StringBuilder(2 * bytes.length);
    for (byte b : bytes) {
      hex.append(HEX_ARRAY.charAt((b & 0xF0) >> 4))
          .append(HEX_ARRAY.charAt((b & 0x0F)));
    }

    return hex.toString().toUpperCase();
  }

  static String getSig(String params) {
    try {
      MessageDigest md = MessageDigest.getInstance("MD5");
      return Utils.bytesToHexString(md.digest(params.getBytes()));
    } catch (NoSuchAlgorithmException ex) {
      ex.printStackTrace();
    }
    return "";
  }

  static String getSoundBaseVersion(Dictionary.DictionaryId dictionaryId, String soundBaseIdx) {
    for (Dictionary dict : DictionaryManagerHolder.getManager().getDictionaries()) {
      if (dict.getId().equals(dictionaryId)) {
        for (DictionaryComponent dictComp : dict.getDictionaryComponents()) {
          if (dictComp.getSdcId().equalsIgnoreCase(soundBaseIdx)) {
            return dictComp.getVersion();
          }
        }
      }
    }
    return "";
  }
}
