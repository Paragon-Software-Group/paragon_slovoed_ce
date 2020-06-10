package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.google.gson.annotations.SerializedName;
import com.paragon_software.theme_manager.ApplicationTheme;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Class to work with application settings parameters.
 */
public class ApplicationSettings implements Serializable
{
  // WARNING!!! Don't change @SerializedName values. They already
  // was used in persistent storage of real users devices.
  // (it has so strange names because of ProGuard)
  static float DEFAULT_FONT_SIZE = 16;
  static float MIN_FONT_SIZE = 0.5f * DEFAULT_FONT_SIZE;
  static float MAX_FONT_SIZE = 1.5f * DEFAULT_FONT_SIZE;
  public static float getDefaultFontSize()
  {
    return DEFAULT_FONT_SIZE;
  }
  public static float getMinFontSize()
  {
    return MIN_FONT_SIZE;
  }
  public static float getMaxFontSize()
  {
    return MAX_FONT_SIZE;
  }
  @SerializedName("m")
  @Nullable
  private Float entryListFontSize;


  static float DEFAULT_ARTICLE_SCALE = 1.0f;
  static float MIN_ARTICLE_SCALE = 0.5f * DEFAULT_ARTICLE_SCALE;
  static float MAX_ARTICLE_SCALE = 1.5f * DEFAULT_ARTICLE_SCALE;
  public static float getDefaultArticleScale()
  {
    return DEFAULT_ARTICLE_SCALE;
  }
  public static float getMinArticleScale()
  {
    return MIN_ARTICLE_SCALE;
  }
  public static float getMaxArticleScale()
  {
    return MAX_ARTICLE_SCALE;
  }
  @SerializedName("n")
  @Nullable
  private Float articleScale;

  static boolean DEFAULT_SHOW_KEYBOARD_FOR_SEARCH = true;
  static boolean DEFAULT_SHOW_HIGHLIGHTING = true;
  static final boolean DEFAULT_KEY_MATCH_FONT = true;
  static boolean DEFAULT_USE_PINCH_TO_ZOOM = true;
  @SerializedName("o")
  @Nullable
  private Boolean showKeyboardForSearch;
  @SerializedName("p")
  @Nullable
  private Boolean showHighlighting;
  @SerializedName("q")
  @Nullable
  private Boolean usePinchToZoom;
  @SerializedName("key_match_font")
  @Nullable
  private Boolean useKeyMatchFont;

  static boolean DEFAULT_SEND_STATISTICS = true;
  static boolean DEFAULT_RECEIVE_NEWS = true;
  public static boolean getDefaultReceivedNews()
  {
    return DEFAULT_RECEIVE_NEWS;
  }
  static boolean DEFAULT_RECEIVE_WOTD = true;
  public static boolean getDefaultReceiveWotD()
  {
    return DEFAULT_RECEIVE_WOTD;
  }
  @SerializedName("r")
  @Nullable
  private Boolean sendStatistics;
  @SerializedName("s")
  @Nullable
  private Boolean receiveNews;
  @SerializedName("t")
  @Nullable
  private Boolean receiveWotD;

  static float DEFAULT_TABLET_COLUMN_WIDTH = 0.40f;
  static float MIN_TABLET_COLUMN_WIDTH = 0.28f;
  static float MAX_TABLET_COLUMN_WIDTH = 0.666f;
  public static float getDefaultTabletColumnWidth()
  {
    return DEFAULT_TABLET_COLUMN_WIDTH;
  }
  public static float getMinTabletColumnWidth()
  {
    return MIN_TABLET_COLUMN_WIDTH;
  }
  public static float getMaxTabletColumnWidth()
  {
    return MAX_TABLET_COLUMN_WIDTH;
  }
  @SerializedName("tabletColumnWidth")
  @Nullable
  private Float tabletColumnWidth;

  @SerializedName("u")
  @NonNull
  private MyView myView = new MyView();

  @SerializedName("v")
  @NonNull
  private List<String> hideSoundIcons = new ArrayList<>();

  @SerializedName("isSystemThemeEnable")
  @Nullable
  private Boolean isSystemThemeEnable;
  private static boolean DEFAULT_SYSTEM_THEME_ENABLE = false;
  @SerializedName("theme")
  @Nullable
  private ApplicationTheme theme;
  private static ApplicationTheme DEFAULT_APPLICATION_THEME = ApplicationTheme.Light;

  /**
   * Default protected constructor.
   */
  ApplicationSettings()
  {
  }

  public ApplicationSettings( @NonNull ApplicationSettings toClone )
  {
    showKeyboardForSearch = toClone.showKeyboardForSearch;
    useKeyMatchFont = toClone.useKeyMatchFont;
    showHighlighting = toClone.showHighlighting;
    usePinchToZoom = toClone.usePinchToZoom;
    entryListFontSize = toClone.entryListFontSize;
    articleScale = toClone.articleScale;

    sendStatistics = toClone.sendStatistics;
    receiveNews = toClone.receiveNews;
    receiveWotD = toClone.receiveWotD;

    tabletColumnWidth = toClone.tabletColumnWidth;

    myView = new MyView(toClone.myView);

    hideSoundIcons = new ArrayList<>(toClone.hideSoundIcons);

    isSystemThemeEnable = toClone.isSystemThemeEnable;
    theme = toClone.theme;
  }

  public boolean isShowKeyboardForSearchEnabled()
  {
    return showKeyboardForSearch == null ? DEFAULT_SHOW_KEYBOARD_FOR_SEARCH : showKeyboardForSearch;
  }
  public boolean isMatchFontEnabled()
  {
    return useKeyMatchFont == null ? DEFAULT_KEY_MATCH_FONT : useKeyMatchFont;
  }

  public void setMatchFontEnabled(boolean useKeyMatchFont)
  {
    this.useKeyMatchFont = useKeyMatchFont;
  }
  void setShowKeyboardForSearchEnabled( boolean enabled )
  {
    this.showKeyboardForSearch = enabled;
  }

  public boolean isShowHighlightingEnabled()
  {
    return showHighlighting == null ? DEFAULT_SHOW_HIGHLIGHTING : showHighlighting;
  }

  void setShowHighlightingEnabled( boolean enabled )
  {
    this.showHighlighting = enabled;
  }

  public boolean isPinchToZoomEnabled()
  {
    return usePinchToZoom == null ? DEFAULT_USE_PINCH_TO_ZOOM : usePinchToZoom;
  }

  void setPinchToZoomEnabled( boolean enabled )
  {
    this.usePinchToZoom = enabled;
  }

  /**
   * Get entry list font size setting value.
   * @return entry list font size setting value
   */
  public float getEntryListFontSize()
  {
    return entryListFontSize == null ? DEFAULT_FONT_SIZE : entryListFontSize;
  }

  /**
   * Set entry list font size setting value. If new value is more than maximum font value (less than minimum font
   * value) then entry list font size setting will be set to maximum font value (minimum font value).
   * @param entryListFontSize new value for entry list font size setting value
   */
  void setEntryListFontSize( float entryListFontSize )
  {
    this.entryListFontSize = entryListFontSize < MIN_FONT_SIZE ? MIN_FONT_SIZE :
                             (entryListFontSize > MAX_FONT_SIZE ? MAX_FONT_SIZE : entryListFontSize);
  }

  public float getArticleScale()
  {
    return articleScale == null ? DEFAULT_ARTICLE_SCALE : articleScale;
  }

  public void setArticleScale( float articleScale )
  {
    this.articleScale = articleScale < MIN_ARTICLE_SCALE ? MIN_ARTICLE_SCALE :
                             (articleScale > MAX_ARTICLE_SCALE ? MAX_ARTICLE_SCALE : articleScale);
  }


  public boolean isStatisticsEnabled()
  {
    return sendStatistics == null ? DEFAULT_SEND_STATISTICS : sendStatistics;
  }

  void setStatisticsEnabled( boolean enabled )
  {
    this.sendStatistics = enabled;
  }

  public boolean isNewsEnabled()
  {
    return receiveNews == null ? DEFAULT_RECEIVE_NEWS : receiveNews;
  }

  public void setNewsEnabled( boolean enabled )
  {
    this.receiveNews = enabled;
  }

  public boolean isWotDEnabled()
  {
    return receiveWotD == null ? DEFAULT_RECEIVE_WOTD : receiveWotD;
  }

  void setWotDEnabled( boolean enabled )
  {
    this.receiveWotD = enabled;
  }

  /**
   * This width set for the first column in tablets and can be changed.
   * @return stored value of column width
   */
  public float getTabletColumnWidth()
  {
    return tabletColumnWidth == null ? DEFAULT_TABLET_COLUMN_WIDTH : tabletColumnWidth;
  }

  /**
   * This width set for the first column in tablets.
   * @param tabletColumnWidth is column width value, which will be saved
   */
  public void setTabletColumnWidth( float tabletColumnWidth )
  {
    this.tabletColumnWidth = tabletColumnWidth < MIN_TABLET_COLUMN_WIDTH ? MIN_TABLET_COLUMN_WIDTH :
            (tabletColumnWidth > MAX_TABLET_COLUMN_WIDTH ? MAX_TABLET_COLUMN_WIDTH : tabletColumnWidth);
  }

  @NonNull
  public MyView getMyView()
  {
    return myView;
  }

  void setHideBritishSoundIcon(boolean isHide) {
    if (isHide) {
      hideSoundIcons.add("enUK");
    } else {
      hideSoundIcons.remove("enUK");
    }
  }

  public boolean isHideBritishSoundIcon() {
    return hideSoundIcons.contains("enUK");
  }

  void setHideAmericanSoundIcon(boolean isHide) {
    if (isHide) {
      hideSoundIcons.add("enUS");
    } else {
      hideSoundIcons.remove("enUS");
    }
  }

  public boolean isHideAmericanSoundIcon() {
    return hideSoundIcons.contains("enUS");
  }

  @NonNull
  public List<String> getHideSoundIcons() {
    List<String> clone = new ArrayList<>();
    clone.addAll(hideSoundIcons);
    return clone;
  }

  public boolean isSystemThemeEnable()
  {
    return isSystemThemeEnable == null ? DEFAULT_SYSTEM_THEME_ENABLE : isSystemThemeEnable;
  }

  void setSystemThemeEnable(@Nullable Boolean isSystemThemeEnable )
  {
    this.isSystemThemeEnable = isSystemThemeEnable;
  }

  @NonNull
  public ApplicationTheme getTheme()
  {
    return theme == null ? DEFAULT_APPLICATION_THEME : theme;
  }

  void setTheme(@NonNull ApplicationTheme theme)
  {
    this.theme = theme;
  }

  @Override
  public boolean equals( Object obj )
  {
    if ( this == obj )
    {
      return true;
    }
    if ( obj == null || getClass() != obj.getClass() )
    {
      return false;
    }
    ApplicationSettings that = (ApplicationSettings) obj;
    return this.toString().equals(that.toString());
  }

  @Override
  public int hashCode()
  {
    return toString().hashCode();
  }

  @Override
  public String toString()
  {
    return "ApplicationSettings{" + "showKeyboardForSearch=" + showKeyboardForSearch
        + ", useKeyMatchFont=" + useKeyMatchFont
        + ", showHighlighting=" + showHighlighting + ", usePinchToZoom=" + usePinchToZoom
        + ", entryListFontSize=" + entryListFontSize + ", articleScale=" + articleScale
        + ", sendStatistics=" + sendStatistics + ", receiveNews=" + receiveNews 
        + ", myView=" + myView + ", hideSoundIcons=" + Arrays.toString(hideSoundIcons.toArray())
        + ", sendStatistics=" + sendStatistics + ", receiveNews=" + receiveNews
        + ", receiveWotD=" + receiveWotD + ", tabletColumnWidth=" + tabletColumnWidth
        + ", isThemeEnable=" + isSystemThemeEnable + "theme=" + theme + '}';
  }

  public class MyView
  {
    @SerializedName("b")
    @Nullable
    private Boolean myViewEnabled;

    @SerializedName("c")
    @Nullable
    private Boolean hidePronunciations; // switch blocks with 'phonetics' thematic
    @SerializedName("d")
    @Nullable
    private Boolean hideExamples;
    @SerializedName("e")
    @Nullable
    private Boolean hidePictures; // switch blocks with 'images' thematic
    @SerializedName("f")
    @Nullable
    private Boolean hideIdioms;
    @SerializedName("g")
    @Nullable
    private Boolean hidePhrasalVerbs; // switch blocks with 'phrase' thematic

    MyView()
    {
    }

    MyView( @NonNull MyView toClone )
    {
      myViewEnabled = toClone.myViewEnabled;

      hidePronunciations = toClone.hidePronunciations;
      hideExamples = toClone.hideExamples;
      hidePictures = toClone.hidePictures;
      hideIdioms = toClone.hideIdioms;
      hidePhrasalVerbs = toClone.hidePhrasalVerbs;
    }

    public boolean isMyViewEnabled()
    {
      return myViewEnabled == null ? true : myViewEnabled;
    }

    public void setMyViewEnabledState( boolean enabled )
    {
      this.myViewEnabled = enabled;
    }


    public boolean isHidePronunciations()
    {
      return hidePronunciations == null ? false : hidePronunciations;
    }

    void setHidePronunciations( boolean hidePronunciations )
    {
      this.hidePronunciations = hidePronunciations;
    }

    public boolean isHideExamples()
    {
      return hideExamples == null ? false : hideExamples;
    }

    void setHideExamples( boolean hideExamples )
    {
      this.hideExamples = hideExamples;
    }

    public boolean isHidePictures()
    {
      return hidePictures == null ? false : hidePictures;
    }

    void setHidePictures( boolean hidePictures )
    {
      this.hidePictures = hidePictures;
    }

    public boolean isHideIdioms()
    {
      return hideIdioms == null ? false : hideIdioms;
    }

    void setHideIdioms( boolean hideIdioms )
    {
      this.hideIdioms = hideIdioms;
    }

    public boolean isHidePhrasalVerbs()
    {
      return hidePhrasalVerbs == null ? false : hidePhrasalVerbs;
    }

    void setHidePhrasalVerbs( boolean hidePhrasalVerbs )
    {
      this.hidePhrasalVerbs = hidePhrasalVerbs;
    }

    @Override
    public String toString()
    {
      return "MyView{" + "myViewEnabled=" + myViewEnabled + ", hidePronunciations="
          + hidePronunciations + ", hideExamples=" + hideExamples + ", hidePictures=" + hidePictures
          + ", hideIdioms=" + hideIdioms + ", hidePhrasalVerbs=" + hidePhrasalVerbs + '}';
    }
  }
}
