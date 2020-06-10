package com.paragon_software.dictionary_manager_factory;

import com.google.gson.annotations.SerializedName;

class Components
{
  @SerializedName( "dict" )
  private Dict   dict;
  @SerializedName( "demo_dict" )
  private Dict   demoDict;
  @SerializedName( "sound_1" )
  private Sound  sound1;
  @SerializedName( "sound_2" )
  private Sound  sound2;
  @SerializedName( "sound_3" )
  private Sound  sound3;
  @SerializedName( "sound_4" )
  private Sound  sound4;
  @SerializedName( "sound_5" )
  private Sound  sound5;
  @SerializedName( "sound_6" )
  private Sound  sound6;
  @SerializedName( "demo_sound_1" )
  private Sound  demoSound1;
  @SerializedName( "demo_sound_2" )
  private Sound  demoSound2;
  @SerializedName( "demo_sound_3" )
  private Sound  demoSound3;
  @SerializedName( "demo_sound_4" )
  private Sound  demoSound4;
  @SerializedName( "demo_sound_5" )
  private Sound  demoSound5;
  @SerializedName( "demo_sound_6" )
  private Sound  demoSound6;
  @SerializedName( "morpho_1" )
  private Morpho morpho1;
  @SerializedName( "morpho_2" )
  private Morpho morpho2;
  @SerializedName( "pict" )
  private Pict pict;
  @SerializedName( "demo_pict" )
  private Pict demoPict;

  Dict getDict()
  {
    return dict;
  }

  Dict getDemoDict()
  {
    return demoDict;
  }

  Sound getSound1()
{
  return sound1;
}

  Sound getSound2()
  {
    return sound2;
  }

  Sound getSound3()
  {
    return sound3;
  }

  Sound getSound4()
  {
    return sound4;
  }

  Sound getSound5()
  {
    return sound5;
  }

  Sound getSound6()
  {
    return sound6;
  }

  Sound getDemoSound1()
  {
    return demoSound1;
  }

  Sound getDemoSound2()
  {
    return demoSound2;
  }

  Sound getDemoSound3()
  {
    return demoSound3;
  }

  Sound getDemoSound4()
  {
    return demoSound4;
  }
  Sound getDemoSound5()
  {
    return demoSound5;
  }

  Sound getDemoSound6()
  {
    return demoSound6;
  }
  Morpho getMorpho1()
  {
    return morpho1;
  }

  Morpho getMorpho2()
  {
    return morpho2;
  }

  Pict getPict()
  {
    return pict;
  }

  Pict getDemoPict()
  {
    return demoPict;
  }
}
