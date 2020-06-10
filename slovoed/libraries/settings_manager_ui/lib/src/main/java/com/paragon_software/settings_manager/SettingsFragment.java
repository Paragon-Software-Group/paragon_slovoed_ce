package com.paragon_software.settings_manager;

import androidx.fragment.app.Fragment;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.SwitchCompat;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.settings_manager_ui.R;

public class SettingsFragment extends Fragment implements OnControllerApplicationSettingsChangeListener,
                                                          OnControllerErrorListener
{
  private static final int ENTRY_LIST_FONT_SEEK_BAR_PROGRESS_MAX_VALUE = 1000;
  private static final float SEEK_BAR_PROGRESS_TO_ENTRY_LIST_FONT_FACTOR =
      (ApplicationSettings.getMaxFontSize() - ApplicationSettings.getMinFontSize()) / ENTRY_LIST_FONT_SEEK_BAR_PROGRESS_MAX_VALUE;

  private SettingsControllerAPI mController;

  private SwitchCompat mShowKeyboardForSearchSwitch;
  private SwitchCompat mUsePinchToZoomSwitch;

  private SeekBar  mEntryListFontSizeSeekBar;
  private TextView mEntryListFontSizeExampleLabel;

  private SwitchCompat mSendStatisticsSwitch;
  private SwitchCompat mReceiveNewsSwitch;

  @Nullable
  @Override
  public View onCreateView( @NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                            @Nullable Bundle savedInstanceState )
  {
    View view = inflater.inflate(R.layout.fragment_settings, container,false);
    initViews(view);
    return view;
  }

  @Override
  public void onResume()
  {
    super.onResume();
    setViewEnableState(getView(), false);
    try
    {
      initController();
      setViewEnableState(getView(), true);
      updateViews();
      initViewListeners();
    }
    catch ( ManagerInitException | WrongTypeException exception )
    {
      showMessage(exception.getMessage());
    }
  }

  @Override
  public void onPause()
  {
    mController.unregisterNotifier(this);
    SettingsManagerAPI settingsManager = SettingsManagerHolder.getManager();
    if ( settingsManager != null )
    {
      settingsManager.freeController(SettingsControllerType.DEFAULT_CONTROLLER);
    }
    super.onPause();
  }

  private void initViews( View rootView )
  {
    initBehaviorSectionViews(rootView);
    initEntryListFontSizeSeekBar(rootView);
    initPrivacySectionViews(rootView);
  }

  private void initBehaviorSectionViews( View rootView )
  {
    mShowKeyboardForSearchSwitch = rootView.findViewById(R.id.show_keyboard_for_search_switch);
    mUsePinchToZoomSwitch = rootView.findViewById(R.id.use_pinch_to_zoom_switch);
  }

  /**
   * Initialize seekbar for changing entry list font size setting value.
   * @param rootView root view containing the seekbar
   */
  private void initEntryListFontSizeSeekBar( View rootView )
  {
    TextView exampleLabelSpace = rootView.findViewById(R.id.settings_entry_list_font_size_example_label_space_view);
    exampleLabelSpace.setTextSize(TypedValue.COMPLEX_UNIT_SP, ApplicationSettings.getMaxFontSize());
    mEntryListFontSizeExampleLabel = rootView.findViewById(R.id.settings_entry_list_font_size_example_label);
    mEntryListFontSizeSeekBar = rootView.findViewById(R.id.settings_entry_list_font_size_seek_bar);
    mEntryListFontSizeSeekBar.setMax(ENTRY_LIST_FONT_SEEK_BAR_PROGRESS_MAX_VALUE);
  }

  private void initPrivacySectionViews( View rootView )
  {
    mSendStatisticsSwitch = rootView.findViewById(R.id.send_statistics_switch);
    mReceiveNewsSwitch = rootView.findViewById(R.id.receive_news_switch);
  }

  private void initController() throws ManagerInitException, WrongTypeException
  {
    SettingsManagerAPI settingsManager = SettingsManagerHolder.getManager();
    if ( settingsManager != null )
    {
      mController = settingsManager.getController(SettingsControllerType.DEFAULT_CONTROLLER);
      mController.registerNotifier(this);
    }
  }

  private void updateViews()
  {
    ApplicationSettings applicationSettings = mController.getApplicationSettings();
    mShowKeyboardForSearchSwitch.setChecked(applicationSettings.isShowKeyboardForSearchEnabled());
    mUsePinchToZoomSwitch.setChecked(applicationSettings.isPinchToZoomEnabled());
    mEntryListFontSizeSeekBar.setProgress((int) ((applicationSettings.getEntryListFontSize() -
        ApplicationSettings.getMinFontSize()) / SEEK_BAR_PROGRESS_TO_ENTRY_LIST_FONT_FACTOR));
    mEntryListFontSizeExampleLabel.setTextSize(TypedValue.COMPLEX_UNIT_SP,
                                               applicationSettings.getEntryListFontSize());
    mSendStatisticsSwitch.setChecked(applicationSettings.isStatisticsEnabled());
    mReceiveNewsSwitch.setChecked(applicationSettings.isNewsEnabled());
  }

  private void initViewListeners()
  {
    mShowKeyboardForSearchSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener()
    {
      @Override
      public void onCheckedChanged( CompoundButton compoundButton, boolean isChecked )
      {
        ApplicationSettings newSettings = mController.getApplicationSettings();
        newSettings.setShowKeyboardForSearchEnabled(isChecked);
        mController.saveNewApplicationSettings(newSettings);
      }
    });
    mUsePinchToZoomSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener()
    {
      @Override
      public void onCheckedChanged( CompoundButton compoundButton, boolean isChecked )
      {
        ApplicationSettings newSettings = mController.getApplicationSettings();
        newSettings.setPinchToZoomEnabled(isChecked);
        mController.saveNewApplicationSettings(newSettings);
      }
    });
    mEntryListFontSizeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener()
    {
      @Override
      public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser )
      {
        mEntryListFontSizeExampleLabel.setTextSize(TypedValue.COMPLEX_UNIT_SP,ApplicationSettings.getMinFontSize() +
            progress * SEEK_BAR_PROGRESS_TO_ENTRY_LIST_FONT_FACTOR);
      }
      @Override
      public void onStartTrackingTouch( SeekBar seekBar )
      {
      }
      @Override
      public void onStopTrackingTouch( SeekBar seekBar )
      {
        ApplicationSettings newSettings = mController.getApplicationSettings();
        newSettings.setEntryListFontSize(ApplicationSettings.getMinFontSize() +
                                             seekBar.getProgress() * SEEK_BAR_PROGRESS_TO_ENTRY_LIST_FONT_FACTOR);
        mController.saveNewApplicationSettings(newSettings);
      }
    });
    mSendStatisticsSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener()
    {
      @Override
      public void onCheckedChanged( CompoundButton compoundButton, boolean isChecked )
      {
        ApplicationSettings newSettings = mController.getApplicationSettings();
        newSettings.setStatisticsEnabled(isChecked);
        mController.saveNewApplicationSettings(newSettings);
      }
    });
    mReceiveNewsSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener()
    {
      @Override
      public void onCheckedChanged( CompoundButton compoundButton, boolean isChecked )
      {
        ApplicationSettings newSettings = mController.getApplicationSettings();
        newSettings.setNewsEnabled(isChecked);
        mController.saveNewApplicationSettings(newSettings);
      }
    });
  }

  private void setViewEnableState( View view, boolean enable ) {
    if (view instanceof ViewGroup)
    {
      for ( int i = 0 ; i < ((ViewGroup) view).getChildCount() ; i++ )
      {
        setViewEnableState(((ViewGroup) view).getChildAt(i), enable);
      }
    }
    view.setEnabled(enable);
  }

  private void showMessage( String message )
  {
    Toast.makeText(getActivity(), message, Toast.LENGTH_SHORT).show();
  }

  @Override
  public void onControllerError( Exception exception )
  {
    showMessage(exception.getMessage());
  }

  @Override
  public void onSettingsChanged()
  {
    updateViews();
  }
}
