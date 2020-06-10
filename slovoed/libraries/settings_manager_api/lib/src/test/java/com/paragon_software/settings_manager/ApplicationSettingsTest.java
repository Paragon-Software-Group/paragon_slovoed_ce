package com.paragon_software.settings_manager;

import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

public class ApplicationSettingsTest
{
  private static final float DELTA = 0.01f;

  private ApplicationSettings applicationSettings;

  @Before
  public void setUp()
  {
    applicationSettings = new ApplicationSettings();
  }

  @Test
  public void testSetGetEntryListFontSize()
  {
    assertEquals(applicationSettings.getEntryListFontSize(), ApplicationSettings.getDefaultFontSize(), DELTA);
    applicationSettings.setEntryListFontSize(ApplicationSettings.getMinFontSize() - 1);
    assertEquals(applicationSettings.getEntryListFontSize(), ApplicationSettings.getMinFontSize(), DELTA);
    applicationSettings.setEntryListFontSize(ApplicationSettings.getMaxFontSize() + 1);
    assertEquals(applicationSettings.getEntryListFontSize(), ApplicationSettings.getMaxFontSize(), DELTA);
    applicationSettings.setEntryListFontSize(ApplicationSettings.getDefaultFontSize());
    assertEquals(applicationSettings.getEntryListFontSize(), ApplicationSettings.getDefaultFontSize(), DELTA);
  }

  @Test
  public void testSetGetDefaultArticleScale() {
    assertEquals(ApplicationSettings.DEFAULT_ARTICLE_SCALE, ApplicationSettings.getDefaultArticleScale(), DELTA);
    assertEquals(ApplicationSettings.MIN_ARTICLE_SCALE, ApplicationSettings.getMinArticleScale(), DELTA);
    assertEquals(ApplicationSettings.MAX_ARTICLE_SCALE, ApplicationSettings.getMaxArticleScale(), DELTA);

    assertEquals(ApplicationSettings.DEFAULT_ARTICLE_SCALE, applicationSettings.getArticleScale(), DELTA);
    applicationSettings.setArticleScale(applicationSettings.getArticleScale() + 0.1f);
    assertEquals(ApplicationSettings.DEFAULT_ARTICLE_SCALE + 0.1f, applicationSettings.getArticleScale(), DELTA);
    applicationSettings.setArticleScale(ApplicationSettings.getMinArticleScale() - 0.1f);
    assertEquals(ApplicationSettings.MIN_ARTICLE_SCALE, applicationSettings.getArticleScale(), DELTA);
    applicationSettings.setArticleScale(ApplicationSettings.getMaxArticleScale() + 0.1f);
    assertEquals(ApplicationSettings.MAX_ARTICLE_SCALE, applicationSettings.getArticleScale(), DELTA);
  }

  @Test
  public void testReceiveWotdAndNews() {
    assertEquals(ApplicationSettings.DEFAULT_RECEIVE_NEWS, ApplicationSettings.getDefaultReceivedNews());
    assertEquals(ApplicationSettings.DEFAULT_RECEIVE_NEWS, applicationSettings.isNewsEnabled());
    applicationSettings.setNewsEnabled(false);
    assertFalse(applicationSettings.isNewsEnabled());
    applicationSettings.setNewsEnabled(true);
    assertTrue(applicationSettings.isNewsEnabled());

    assertEquals(ApplicationSettings.DEFAULT_RECEIVE_WOTD, ApplicationSettings.getDefaultReceiveWotD());
    assertEquals(ApplicationSettings.DEFAULT_RECEIVE_WOTD, applicationSettings.isWotDEnabled());
    applicationSettings.setWotDEnabled(false);
    assertFalse(applicationSettings.isWotDEnabled());
    applicationSettings.setWotDEnabled(true);
    assertTrue(applicationSettings.isWotDEnabled());
  }

  @Test
  public void testSetGetDefaultTabletColumnWidth() {
    assertEquals(ApplicationSettings.DEFAULT_TABLET_COLUMN_WIDTH, ApplicationSettings.getDefaultTabletColumnWidth(), DELTA);
    assertEquals(ApplicationSettings.MIN_TABLET_COLUMN_WIDTH, ApplicationSettings.getMinTabletColumnWidth(), DELTA);
    assertEquals(ApplicationSettings.MAX_TABLET_COLUMN_WIDTH, ApplicationSettings.getMaxTabletColumnWidth(), DELTA);

    assertEquals(ApplicationSettings.DEFAULT_TABLET_COLUMN_WIDTH, applicationSettings.getTabletColumnWidth(), DELTA);
    applicationSettings.setTabletColumnWidth(applicationSettings.getTabletColumnWidth() + 0.1f);
    assertEquals(ApplicationSettings.DEFAULT_TABLET_COLUMN_WIDTH + 0.1f, applicationSettings.getTabletColumnWidth(), DELTA);
    applicationSettings.setTabletColumnWidth(ApplicationSettings.getMinTabletColumnWidth() - 0.1f);
    assertEquals(ApplicationSettings.MIN_TABLET_COLUMN_WIDTH, applicationSettings.getTabletColumnWidth(), DELTA);
    applicationSettings.setTabletColumnWidth(ApplicationSettings.getMaxTabletColumnWidth() + 0.1f);
    assertEquals(ApplicationSettings.MAX_TABLET_COLUMN_WIDTH, applicationSettings.getTabletColumnWidth(), DELTA);
  }

  @Test
  public void testShowHighlightingEnabled() {
    assertEquals(ApplicationSettings.DEFAULT_SHOW_HIGHLIGHTING, applicationSettings.isShowHighlightingEnabled());

    applicationSettings.setShowHighlightingEnabled(true);
    assertTrue(applicationSettings.isShowHighlightingEnabled());
    applicationSettings.setShowHighlightingEnabled(false);
    assertFalse(applicationSettings.isShowHighlightingEnabled());
  }

  @Test
  public void testHideSoundIcon() {
    assertFalse(applicationSettings.isHideBritishSoundIcon());
    assertFalse(applicationSettings.isHideAmericanSoundIcon());

    applicationSettings.setHideBritishSoundIcon(true);
    assertTrue(applicationSettings.isHideBritishSoundIcon());
    applicationSettings.setHideBritishSoundIcon(false);
    assertFalse(applicationSettings.isHideBritishSoundIcon());

    applicationSettings.setHideAmericanSoundIcon(true);
    assertTrue(applicationSettings.isHideAmericanSoundIcon());
    applicationSettings.setHideAmericanSoundIcon(false);
    assertFalse(applicationSettings.isHideAmericanSoundIcon());
  }

  @Test
  public void testMyViewSettings() {
    assertTrue(applicationSettings.getMyView().isMyViewEnabled());
    assertFalse(applicationSettings.getMyView().isHideExamples());
    assertFalse(applicationSettings.getMyView().isHideIdioms());
    assertFalse(applicationSettings.getMyView().isHidePhrasalVerbs());
    assertFalse(applicationSettings.getMyView().isHidePictures());
    assertFalse(applicationSettings.getMyView().isHidePronunciations());

    applicationSettings.getMyView().setMyViewEnabledState(false);
    assertFalse(applicationSettings.getMyView().isMyViewEnabled());
    applicationSettings.getMyView().setMyViewEnabledState(true);
    assertTrue(applicationSettings.getMyView().isMyViewEnabled());

    applicationSettings.getMyView().setHideExamples(true);
    assertTrue(applicationSettings.getMyView().isHideExamples());
    applicationSettings.getMyView().setHideExamples(false);
    assertFalse(applicationSettings.getMyView().isHideExamples());

    applicationSettings.getMyView().setHideIdioms(true);
    assertTrue(applicationSettings.getMyView().isHideIdioms());
    applicationSettings.getMyView().setHideIdioms(false);
    assertFalse(applicationSettings.getMyView().isHideIdioms());

    applicationSettings.getMyView().setHidePhrasalVerbs(true);
    assertTrue(applicationSettings.getMyView().isHidePhrasalVerbs());
    applicationSettings.getMyView().setHidePhrasalVerbs(false);
    assertFalse(applicationSettings.getMyView().isHidePhrasalVerbs());

    applicationSettings.getMyView().setHidePictures(true);
    assertTrue(applicationSettings.getMyView().isHidePictures());
    applicationSettings.getMyView().setHidePictures(false);
    assertFalse(applicationSettings.getMyView().isHidePictures());

    applicationSettings.getMyView().setHidePronunciations(true);
    assertTrue(applicationSettings.getMyView().isHidePronunciations());
    applicationSettings.getMyView().setHidePronunciations(false);
    assertFalse(applicationSettings.getMyView().isHidePronunciations());
  }

  @Test
  public void testApplicationSettingsEquals()
  {
    assertEquals(applicationSettings, applicationSettings);
    assertNotNull(applicationSettings);
    assertNotEquals(applicationSettings, 1);

    ApplicationSettings equalSettings = new ApplicationSettings(applicationSettings);
    assertEquals(applicationSettings, equalSettings);
    assertEquals(applicationSettings.hashCode(), equalSettings.hashCode());

    applicationSettings.setShowKeyboardForSearchEnabled(!applicationSettings.isShowKeyboardForSearchEnabled());
    assertNotEquals(applicationSettings, equalSettings);
    assertNotEquals(applicationSettings.hashCode(), equalSettings.hashCode());
    equalSettings.setShowKeyboardForSearchEnabled(!equalSettings.isShowKeyboardForSearchEnabled());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setPinchToZoomEnabled(!applicationSettings.isPinchToZoomEnabled());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setPinchToZoomEnabled(!equalSettings.isPinchToZoomEnabled());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setEntryListFontSize(applicationSettings.getEntryListFontSize() + 1);
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setEntryListFontSize(equalSettings.getEntryListFontSize() + 1);
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setArticleScale(applicationSettings.getArticleScale() + 0.1f);
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setArticleScale(equalSettings.getArticleScale() + 0.1f);
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setTabletColumnWidth(applicationSettings.getTabletColumnWidth() + 0.1f);
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setTabletColumnWidth(equalSettings.getTabletColumnWidth() + 0.1f);
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setStatisticsEnabled(!applicationSettings.isStatisticsEnabled());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setStatisticsEnabled(!equalSettings.isStatisticsEnabled());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setNewsEnabled(!applicationSettings.isNewsEnabled());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setNewsEnabled(!equalSettings.isNewsEnabled());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setWotDEnabled(!applicationSettings.isWotDEnabled());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setWotDEnabled(!equalSettings.isWotDEnabled());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setShowHighlightingEnabled(!applicationSettings.isShowHighlightingEnabled());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setShowHighlightingEnabled(!equalSettings.isShowHighlightingEnabled());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setHideBritishSoundIcon(!applicationSettings.isHideBritishSoundIcon());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setHideBritishSoundIcon(!equalSettings.isHideBritishSoundIcon());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setHideAmericanSoundIcon(!applicationSettings.isHideAmericanSoundIcon());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.setHideAmericanSoundIcon(!equalSettings.isHideAmericanSoundIcon());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.setHideAmericanSoundIcon(!applicationSettings.isHideAmericanSoundIcon());
    assertNotEquals(applicationSettings.getHideSoundIcons(), equalSettings.getHideSoundIcons());
    equalSettings.setHideAmericanSoundIcon(!equalSettings.isHideAmericanSoundIcon());
    assertEquals(applicationSettings.getHideSoundIcons(), equalSettings.getHideSoundIcons());

    applicationSettings.getMyView().setMyViewEnabledState(!applicationSettings.getMyView().isMyViewEnabled());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.getMyView().setMyViewEnabledState(!equalSettings.getMyView().isMyViewEnabled());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.getMyView().setHidePronunciations(!applicationSettings.getMyView().isHidePronunciations());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.getMyView().setHidePronunciations(!equalSettings.getMyView().isHidePronunciations());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.getMyView().setHideExamples(!applicationSettings.getMyView().isHideExamples());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.getMyView().setHideExamples(!equalSettings.getMyView().isHideExamples());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.getMyView().setHidePictures(!applicationSettings.getMyView().isHidePictures());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.getMyView().setHidePictures(!equalSettings.getMyView().isHidePictures());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.getMyView().setHideIdioms(!applicationSettings.getMyView().isHideIdioms());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.getMyView().setHideIdioms(!equalSettings.getMyView().isHideIdioms());
    assertEquals(applicationSettings, equalSettings);

    applicationSettings.getMyView().setHidePhrasalVerbs(!applicationSettings.getMyView().isHidePhrasalVerbs());
    assertNotEquals(applicationSettings, equalSettings);
    equalSettings.getMyView().setHidePhrasalVerbs(!equalSettings.getMyView().isHidePhrasalVerbs());
    assertEquals(applicationSettings, equalSettings);
  }
}