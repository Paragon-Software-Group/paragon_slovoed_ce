package com.paragon_software.settings_manager;

import static com.paragon_software.settings_manager.SettingsControllerTest.DELTA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_AppSettingsChangeListener
      implements OnControllerApplicationSettingsChangeListener
{
  private BaseSettingsController settingsController;
  private int mMaxNumberOfCalls;
  private int mNumberOfCalls;

  private Boolean expectedShowKeyboardForSearch = null;

  private Boolean expectedUsePinchToZoom = null;

  private Float expectedEntryListFontSize = null;

  private Boolean expectedSendStatistics = null;

  private Boolean expectedReceiveNews = null;

  public MockForController_AppSettingsChangeListener(BaseSettingsController settingsController
          , int maxNumberOfCalls)
  {
    this.settingsController = settingsController;
    mMaxNumberOfCalls = maxNumberOfCalls;
  }

  public int getNumberOfCalls()
  {
    return mNumberOfCalls;
  }

  public void setExpectedShowKeyboardForSearch( boolean showKeyboardForSearch )
  {
    expectedShowKeyboardForSearch = showKeyboardForSearch;
  }

  public void setExpectedUsePinchToZoom( boolean usePinchToZoom )
  {
    expectedUsePinchToZoom = usePinchToZoom;
  }

  public void setExpectedEntryListFontSize( float entryListFontSize )
  {
    expectedEntryListFontSize = entryListFontSize;
  }

  public void setExpectedSendStatistics( boolean sendStatistics )
  {
    expectedSendStatistics = sendStatistics;
  }

  public void setExpectedReceiveNews( boolean receiveNews )
  {
    expectedReceiveNews = receiveNews;
  }

  @Override
  public void onSettingsChanged()
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    if ( expectedShowKeyboardForSearch != null )
      assertEquals(expectedShowKeyboardForSearch, settingsController.getApplicationSettings().isShowKeyboardForSearchEnabled());
    if ( expectedUsePinchToZoom != null )
      assertEquals(expectedUsePinchToZoom, settingsController.getApplicationSettings().isPinchToZoomEnabled());
    if ( expectedEntryListFontSize != null )
      assertEquals(expectedEntryListFontSize, settingsController.getApplicationSettings().getEntryListFontSize(), DELTA);
    if ( expectedSendStatistics != null )
      assertEquals(expectedSendStatistics, settingsController.getApplicationSettings().isStatisticsEnabled());
    if ( expectedReceiveNews != null )
      assertEquals(expectedReceiveNews, settingsController.getApplicationSettings().isNewsEnabled());
  }
}