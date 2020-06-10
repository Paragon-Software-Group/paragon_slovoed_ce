package com.paragon_software.dictionary_manager;

//if add new language here, add in gradle AppBuilder plugin too
public enum Language
{
  Afrikaans("Afrikaans", "afri", "af"),
  Arabic("Arabic", "arab", "ar"),
  Argentinian("Argentinian", "arge", "es", "AR"),
  Basque("Basque", "basq", "eu"),
  Brazilian("Brazilian portuguese", "braz", "pt", "BR"),
  Bulgarian("Bulgarian", "bulg", "bg"),
  Catalan("Catalan", "ctln", "ca"),
  Chinese("Chinese", "chin", "zh"),
  ChineseSimplified("Chinese Simplified", "chin", "zh", "CN"),
  ChineseTraditional("Chinese Traditional", "chin", "zh", "TW"),
  Croatian("Croatian", "croa", "hr"),
  Czech("Czech", "czec", "cs"),
  Danish("Danish", "dani", "da"),
  Dutch("Dutch", "dutc", "nl"),
  English("English", "engl", "en"),
  BritishEnglish("British English", "bren", "en", "GB"),
  AmericanEnglish("American English", "amen", "en", "US"),
  Estonian("Estonian", "esto", "et"),
  Farsi("Farsi", "fars", "fa"),
  Finnish("Finnish", "finn", "fi"),
  French("French", "fren", "fr"),
  German("German", "germ", "de"),
  Greek("Greek", "gree", "el"),
  Hebrew("Hebrew", "hebr", "he"),
  Hindi("Hindi", "hind", "hi"),
  Hungarian("Hungarian", "hung", "hu"),
  Icelandic("Icelandic", "icel", "is"),
  Indonesian("Indonesian", "indo", "id"),
  Irish("Irish", "iris", "ga"),
  Italian("Italian", "ital", "it"),
  Japanese("Japanese", "japa", "ja"),
  Korean("Korean", "kore", "ko"),
  Latin("Latin", "lati", "la"),
  Latvian("Latvian", "latv", "lv"),
  Lithuanian("Lithuanian", "lith", "lt"),
  Malay("Malay", "mala", "ms"),
  Norwegian("Norwegian", "norw", "no"),
  Persian("Persian", "pers", "fa"),
  Polish("Polish", "poli", "pl"),
  Portuguese("Portuguese", "port", "pt"),
  Romanian("Romanian", "roma", "ro"),
  Russian("Russian", "russ", "ru"),
  Serbian("Serbian", "serb", "sr"),
  Slovak("Slovak", "slov", "sk"),
  Slovak2("Slovak", "slvk", "sk"),
  Slovenian("Slovenian", "slvn", "sl"),
  Spanish("Spanish", "span", "es"),
  Swedish("Swedish", "swed", "sv"),
  Thai("Thai", "thai", "th"),
  Turkish("Turkish", "turk", "tr"),
  Ukrainian("Ukrainian", "ukra", "uk"),
  Urdu("Urdu", "urdu", "ur"),
  Uzbek("Uzbek", "uzbe", "uz"),
  Vietnamese("Vietnamese", "viet", "vi"),
  WorldEnglish("World English", "woen", "en", "WE");

  public final String name;
  public final String shortName;
  public final String isoCode;
  public final String countryCode;
  public final int    code;

  Language(String name, String shortName, String isoCode )
  {
    this(name, shortName, isoCode, null);
  }

  Language(String name, String shortName, String isoCode, String countryCode )
  {
    this.name = name;
    this.shortName = shortName;
    this.isoCode = isoCode;
    this.countryCode = countryCode;
    this.code = getLangCodeFromShortForm(shortName);
  }

  public final String getFullForm()
  {
    return name;
  }

  public static int getLangCodeFromShortForm( String shortForm )
  {
    int b1 = shortForm.codePointAt(3);
    int b2 = shortForm.codePointAt(2);
    int b3 = shortForm.codePointAt(1);
    int b4 = shortForm.codePointAt(0);
    return ( b1 << 24 ) | ( b2 << 16 ) | ( b3 << 8 ) | b4;
  }

  public static Language fromCode(int langCode)
  {
    for (Language l : Language.values())
    {
      if (l.code == langCode)
      {
        return l;
      }
    }
    return English;
  }
}
