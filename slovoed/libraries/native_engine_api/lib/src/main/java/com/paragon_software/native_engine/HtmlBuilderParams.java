package com.paragon_software.native_engine;

public class HtmlBuilderParams
{
  private float scale;
  private float horizontalPadding;

  private boolean hidePhonetics;
  private boolean hideExamples;
  private boolean hideImages;
  private boolean hideIdioms;
  private boolean hidePhrase;
  private String[] hiddenSoundIcons;
  private boolean removeBodyMargin;

  private HtmlBuilderParams( Builder builder )
  {
    scale = builder.scale;
    horizontalPadding = builder.horizontalPadding;
    hidePhonetics = builder.hidePhonetics;
    hideExamples = builder.hideExamples;
    hideImages = builder.hideImages;
    hideIdioms = builder.hideIdioms;
    hidePhrase = builder.hidePhrase;
    hiddenSoundIcons = builder.hiddenSoundIcons;
    removeBodyMargin = builder.removeBodyMargin;
  }

  public float getScale()
  {
    return scale;
  }

  public float getHorizontalPadding()
  {
    return horizontalPadding;
  }

  public boolean isHidePhonetics()
  {
    return hidePhonetics;
  }

  public boolean isHideExamples()
  {
    return hideExamples;
  }

  public boolean isHideImages()
  {
    return hideImages;
  }

  public boolean isHideIdioms()
  {
    return hideIdioms;
  }

  public boolean isHidePhrase()
  {
    return hidePhrase;
  }

  public String[] getHiddenSoundIcons()
  {
    return hiddenSoundIcons;
  }

  public boolean isRemoveBodyMargin()
  {
    return removeBodyMargin;
  }

  public static class Builder
  {
    private float scale             = 1.0f;
    private float horizontalPadding = 0f;

    private boolean hidePhonetics = false;
    private boolean hideExamples  = false;
    private boolean hideImages    = false;
    private boolean hideIdioms    = false;
    private boolean hidePhrase    = false;
    private String[] hiddenSoundIcons = new String[]{};
    private boolean removeBodyMargin = false;

    public Builder()
    {
    }

    public Builder setScale( float _scale )
    {
      scale = _scale;
      return this;
    }

    public Builder setHorizontalPadding( float _horizontalPadding )
    {
      horizontalPadding = _horizontalPadding;
      return this;
    }

    public Builder setHidePhonetics( boolean hide )
    {
      hidePhonetics = hide;
      return this;
    }

    public Builder setHideExamples( boolean hide )
    {
      hideExamples = hide;
      return this;
    }

    public Builder setHideImages( boolean hide )
    {
      hideImages = hide;
      return this;
    }

    public Builder setHideIdioms( boolean hide )
    {
      hideIdioms = hide;
      return this;
    }

    public Builder setHidePhrase( boolean hide )
    {
      hidePhrase = hide;
      return this;
    }


    public Builder setHiddenSoundIcons(String[] soundIcons )
    {
      hiddenSoundIcons = soundIcons;
      return this;
    }

    public Builder setRemoveBodyMargin(boolean remove) {
      this.removeBodyMargin = remove;
      return this;
    }

    public HtmlBuilderParams create()
    {
      return new HtmlBuilderParams(this);
    }
  }
}
