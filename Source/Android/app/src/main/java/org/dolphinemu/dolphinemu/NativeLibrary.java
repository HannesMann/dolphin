/*
 * Copyright 2013 Dolphin Emulator Project
 * Licensed under GPLv2+
 * Refer to the license.txt file included.
 */

package org.dolphinemu.dolphinemu;

import android.view.Surface;

import androidx.appcompat.app.AlertDialog;

import org.dolphinemu.dolphinemu.activities.EmulationActivity;
import org.dolphinemu.dolphinemu.utils.Log;
import org.dolphinemu.dolphinemu.utils.Rumble;

import java.lang.ref.WeakReference;

/**
 * Class which contains methods that interact
 * with the native side of the Dolphin code.
 */
public final class NativeLibrary
{
  private static WeakReference<EmulationActivity> sEmulationActivity = new WeakReference<>(null);

  /**
   * Returns the current instance of EmulationActivity.
   * There should only ever be one EmulationActivity instantiated.
   */
  public static EmulationActivity getEmulationActivity()
  {
    return sEmulationActivity.get();
  }

  /**
   * Button type for use in onTouchEvent
   */
  public static final class ButtonType
  {
    public static final int BUTTON_A = 0;
    public static final int BUTTON_B = 1;
    public static final int BUTTON_START = 2;
    public static final int BUTTON_X = 3;
    public static final int BUTTON_Y = 4;
    public static final int BUTTON_Z = 5;
    public static final int BUTTON_UP = 6;
    public static final int BUTTON_DOWN = 7;
    public static final int BUTTON_LEFT = 8;
    public static final int BUTTON_RIGHT = 9;
    public static final int STICK_MAIN = 10;
    public static final int STICK_MAIN_UP = 11;
    public static final int STICK_MAIN_DOWN = 12;
    public static final int STICK_MAIN_LEFT = 13;
    public static final int STICK_MAIN_RIGHT = 14;
    public static final int STICK_C = 15;
    public static final int STICK_C_UP = 16;
    public static final int STICK_C_DOWN = 17;
    public static final int STICK_C_LEFT = 18;
    public static final int STICK_C_RIGHT = 19;
    public static final int TRIGGER_L = 20;
    public static final int TRIGGER_R = 21;
    public static final int WIIMOTE_BUTTON_A = 100;
    public static final int WIIMOTE_BUTTON_B = 101;
    public static final int WIIMOTE_BUTTON_MINUS = 102;
    public static final int WIIMOTE_BUTTON_PLUS = 103;
    public static final int WIIMOTE_BUTTON_HOME = 104;
    public static final int WIIMOTE_BUTTON_1 = 105;
    public static final int WIIMOTE_BUTTON_2 = 106;
    public static final int WIIMOTE_UP = 107;
    public static final int WIIMOTE_DOWN = 108;
    public static final int WIIMOTE_LEFT = 109;
    public static final int WIIMOTE_RIGHT = 110;
    public static final int WIIMOTE_IR = 111;
    public static final int WIIMOTE_IR_UP = 112;
    public static final int WIIMOTE_IR_DOWN = 113;
    public static final int WIIMOTE_IR_LEFT = 114;
    public static final int WIIMOTE_IR_RIGHT = 115;
    public static final int WIIMOTE_IR_FORWARD = 116;
    public static final int WIIMOTE_IR_BACKWARD = 117;
    public static final int WIIMOTE_IR_HIDE = 118;
    public static final int WIIMOTE_SWING = 119;
    public static final int WIIMOTE_SWING_UP = 120;
    public static final int WIIMOTE_SWING_DOWN = 121;
    public static final int WIIMOTE_SWING_LEFT = 122;
    public static final int WIIMOTE_SWING_RIGHT = 123;
    public static final int WIIMOTE_SWING_FORWARD = 124;
    public static final int WIIMOTE_SWING_BACKWARD = 125;
    public static final int WIIMOTE_TILT = 126;
    public static final int WIIMOTE_TILT_FORWARD = 127;
    public static final int WIIMOTE_TILT_BACKWARD = 128;
    public static final int WIIMOTE_TILT_LEFT = 129;
    public static final int WIIMOTE_TILT_RIGHT = 130;
    public static final int WIIMOTE_TILT_MODIFIER = 131;
    public static final int WIIMOTE_SHAKE_X = 132;
    public static final int WIIMOTE_SHAKE_Y = 133;
    public static final int WIIMOTE_SHAKE_Z = 134;
    public static final int NUNCHUK_BUTTON_C = 200;
    public static final int NUNCHUK_BUTTON_Z = 201;
    public static final int NUNCHUK_STICK = 202;
    public static final int NUNCHUK_STICK_UP = 203;
    public static final int NUNCHUK_STICK_DOWN = 204;
    public static final int NUNCHUK_STICK_LEFT = 205;
    public static final int NUNCHUK_STICK_RIGHT = 206;
    public static final int NUNCHUK_SWING = 207;
    public static final int NUNCHUK_SWING_UP = 208;
    public static final int NUNCHUK_SWING_DOWN = 209;
    public static final int NUNCHUK_SWING_LEFT = 210;
    public static final int NUNCHUK_SWING_RIGHT = 221;
    public static final int NUNCHUK_SWING_FORWARD = 212;
    public static final int NUNCHUK_SWING_BACKWARD = 213;
    public static final int NUNCHUK_TILT = 214;
    public static final int NUNCHUK_TILT_FORWARD = 215;
    public static final int NUNCHUK_TILT_BACKWARD = 216;
    public static final int NUNCHUK_TILT_LEFT = 217;
    public static final int NUNCHUK_TILT_RIGHT = 218;
    public static final int NUNCHUK_TILT_MODIFIER = 219;
    public static final int NUNCHUK_SHAKE_X = 220;
    public static final int NUNCHUK_SHAKE_Y = 221;
    public static final int NUNCHUK_SHAKE_Z = 222;
    public static final int CLASSIC_BUTTON_A = 300;
    public static final int CLASSIC_BUTTON_B = 301;
    public static final int CLASSIC_BUTTON_X = 302;
    public static final int CLASSIC_BUTTON_Y = 303;
    public static final int CLASSIC_BUTTON_MINUS = 304;
    public static final int CLASSIC_BUTTON_PLUS = 305;
    public static final int CLASSIC_BUTTON_HOME = 306;
    public static final int CLASSIC_BUTTON_ZL = 307;
    public static final int CLASSIC_BUTTON_ZR = 308;
    public static final int CLASSIC_DPAD_UP = 309;
    public static final int CLASSIC_DPAD_DOWN = 310;
    public static final int CLASSIC_DPAD_LEFT = 311;
    public static final int CLASSIC_DPAD_RIGHT = 312;
    public static final int CLASSIC_STICK_LEFT = 313;
    public static final int CLASSIC_STICK_LEFT_UP = 314;
    public static final int CLASSIC_STICK_LEFT_DOWN = 315;
    public static final int CLASSIC_STICK_LEFT_LEFT = 316;
    public static final int CLASSIC_STICK_LEFT_RIGHT = 317;
    public static final int CLASSIC_STICK_RIGHT = 318;
    public static final int CLASSIC_STICK_RIGHT_UP = 319;
    public static final int CLASSIC_STICK_RIGHT_DOWN = 320;
    public static final int CLASSIC_STICK_RIGHT_LEFT = 321;
    public static final int CLASSIC_STICK_RIGHT_RIGHT = 322;
    public static final int CLASSIC_TRIGGER_L = 323;
    public static final int CLASSIC_TRIGGER_R = 324;
    public static final int GUITAR_BUTTON_MINUS = 400;
    public static final int GUITAR_BUTTON_PLUS = 401;
    public static final int GUITAR_FRET_GREEN = 402;
    public static final int GUITAR_FRET_RED = 403;
    public static final int GUITAR_FRET_YELLOW = 404;
    public static final int GUITAR_FRET_BLUE = 405;
    public static final int GUITAR_FRET_ORANGE = 406;
    public static final int GUITAR_STRUM_UP = 407;
    public static final int GUITAR_STRUM_DOWN = 408;
    public static final int GUITAR_STICK = 409;
    public static final int GUITAR_STICK_UP = 410;
    public static final int GUITAR_STICK_DOWN = 411;
    public static final int GUITAR_STICK_LEFT = 412;
    public static final int GUITAR_STICK_RIGHT = 413;
    public static final int GUITAR_WHAMMY_BAR = 414;
    public static final int DRUMS_BUTTON_MINUS = 500;
    public static final int DRUMS_BUTTON_PLUS = 501;
    public static final int DRUMS_PAD_RED = 502;
    public static final int DRUMS_PAD_YELLOW = 503;
    public static final int DRUMS_PAD_BLUE = 504;
    public static final int DRUMS_PAD_GREEN = 505;
    public static final int DRUMS_PAD_ORANGE = 506;
    public static final int DRUMS_PAD_BASS = 507;
    public static final int DRUMS_STICK = 508;
    public static final int DRUMS_STICK_UP = 509;
    public static final int DRUMS_STICK_DOWN = 510;
    public static final int DRUMS_STICK_LEFT = 511;
    public static final int DRUMS_STICK_RIGHT = 512;
    public static final int TURNTABLE_BUTTON_GREEN_LEFT = 600;
    public static final int TURNTABLE_BUTTON_RED_LEFT = 601;
    public static final int TURNTABLE_BUTTON_BLUE_LEFT = 602;
    public static final int TURNTABLE_BUTTON_GREEN_RIGHT = 603;
    public static final int TURNTABLE_BUTTON_RED_RIGHT = 604;
    public static final int TURNTABLE_BUTTON_BLUE_RIGHT = 605;
    public static final int TURNTABLE_BUTTON_MINUS = 606;
    public static final int TURNTABLE_BUTTON_PLUS = 607;
    public static final int TURNTABLE_BUTTON_HOME = 608;
    public static final int TURNTABLE_BUTTON_EUPHORIA = 609;
    public static final int TURNTABLE_TABLE_LEFT = 610;
    public static final int TURNTABLE_TABLE_LEFT_LEFT = 611;
    public static final int TURNTABLE_TABLE_LEFT_RIGHT = 612;
    public static final int TURNTABLE_TABLE_RIGHT = 613;
    public static final int TURNTABLE_TABLE_RIGHT_LEFT = 614;
    public static final int TURNTABLE_TABLE_RIGHT_RIGHT = 615;
    public static final int TURNTABLE_STICK = 616;
    public static final int TURNTABLE_STICK_UP = 617;
    public static final int TURNTABLE_STICK_DOWN = 618;
    public static final int TURNTABLE_STICK_LEFT = 619;
    public static final int TURNTABLE_STICK_RIGHT = 620;
    public static final int TURNTABLE_EFFECT_DIAL = 621;
    public static final int TURNTABLE_CROSSFADE = 622;
    public static final int TURNTABLE_CROSSFADE_LEFT = 623;
    public static final int TURNTABLE_CROSSFADE_RIGHT = 624;
    public static final int WIIMOTE_ACCEL_LEFT = 625;
    public static final int WIIMOTE_ACCEL_RIGHT = 626;
    public static final int WIIMOTE_ACCEL_FORWARD = 627;
    public static final int WIIMOTE_ACCEL_BACKWARD = 628;
    public static final int WIIMOTE_ACCEL_UP = 629;
    public static final int WIIMOTE_ACCEL_DOWN = 630;
    public static final int WIIMOTE_GYRO_PITCH_UP = 631;
    public static final int WIIMOTE_GYRO_PITCH_DOWN = 632;
    public static final int WIIMOTE_GYRO_ROLL_LEFT = 633;
    public static final int WIIMOTE_GYRO_ROLL_RIGHT = 634;
    public static final int WIIMOTE_GYRO_YAW_LEFT = 635;
    public static final int WIIMOTE_GYRO_YAW_RIGHT = 636;
  }

  /**
   * Button states
   */
  public static final class ButtonState
  {
    public static final int RELEASED = 0;
    public static final int PRESSED = 1;
  }

  private NativeLibrary()
  {
    // Disallows instantiation.
  }

  /**
   * Default touchscreen device
   */
  public static final String TouchScreenDevice = "Touchscreen";

  /**
   * Handles button press events for a gamepad.
   *
   * @param Device The input descriptor of the gamepad.
   * @param Button Key code identifying which button was pressed.
   * @param Action Mask identifying which action is happening (button pressed down, or button released).
   * @return If we handled the button press.
   */
  public static native boolean onGamePadEvent(String Device, int Button, int Action);

  /**
   * Handles gamepad movement events.
   *
   * @param Device The device ID of the gamepad.
   * @param Axis   The axis ID
   * @param Value  The value of the axis represented by the given ID.
   */
  public static native void onGamePadMoveEvent(String Device, int Axis, float Value);

  /**
   * Rumble sent from native. Currently only supports phone rumble.
   *
   * @param padID Ignored for now. Future use would be to pass rumble to a connected controller
   * @param state Ignored for now since phone rumble can't just be 'turned' on/off
   */
  public static void rumble(int padID, double state)
  {
    final EmulationActivity emulationActivity = sEmulationActivity.get();
    if (emulationActivity == null)
    {
      Log.warning("[NativeLibrary] EmulationActivity is null");
      return;
    }

    Rumble.checkRumble(padID, state);
  }

  public static native void SetMotionSensorsEnabled(boolean accelerometerEnabled,
          boolean gyroscopeEnabled);

  // Angle is in radians and should be non-negative
  public static native double GetInputRadiusAtAngle(int emu_pad_id, int stick, double angle);

  public static native void NewGameIniFile();

  public static native void LoadGameIniFile(String gameId);

  public static native void SaveGameIniFile(String gameId);

  public static native String GetUserSetting(String gameID, String Section, String Key);

  public static native void SetUserSetting(String gameID, String Section, String Key, String Value);

  public static native void SetProfileSetting(String profile, String Section, String Key,
          String Value);

  public static native void InitGameIni(String gameID);

  /**
   * Gets a value from a key in the given ini-based config file.
   *
   * @param configFile The ini-based config file to get the value from.
   * @param Section    The section key that the actual key is in.
   * @param Key        The key to get the value from.
   * @param Default    The value to return in the event the given key doesn't exist.
   * @return the value stored at the key, or a default value if it doesn't exist.
   */
  public static native String GetConfig(String configFile, String Section, String Key,
          String Default);

  /**
   * Sets a value to a key in the given ini config file.
   *
   * @param configFile The ini-based config file to add the value to.
   * @param Section    The section key for the ini key
   * @param Key        The actual ini key to set.
   * @param Value      The string to set the ini key to.
   */
  public static native void SetConfig(String configFile, String Section, String Key, String Value);

  /**
   * Gets the Dolphin version string.
   *
   * @return the Dolphin version string.
   */
  public static native String GetVersionString();

  public static native String GetGitRevision();

  /**
   * Saves a screen capture of the game
   */
  public static native void SaveScreenShot();

  /**
   * Saves a game state to the slot number.
   *
   * @param slot The slot location to save state to.
   * @param wait If false, returns as early as possible.
   *             If true, returns once the savestate has been written to disk.
   */
  public static native void SaveState(int slot, boolean wait);

  /**
   * Saves a game state to the specified path.
   *
   * @param path The path to save state to.
   * @param wait If false, returns as early as possible.
   *             If true, returns once the savestate has been written to disk.
   */
  public static native void SaveStateAs(String path, boolean wait);

  /**
   * Loads a game state from the slot number.
   *
   * @param slot The slot location to load state from.
   */
  public static native void LoadState(int slot);

  /**
   * Loads a game state from the specified path.
   *
   * @param path The path to load state from.
   */
  public static native void LoadStateAs(String path);

  /**
   * Sets the current working user directory
   * If not set, it auto-detects a location
   */
  public static native void SetUserDirectory(String directory);

  /**
   * Returns the current working user directory
   */
  public static native String GetUserDirectory();

  public static native int DefaultCPUCore();

  public static native void ReloadConfig();

  /**
   * Initializes the native parts of the app.
   *
   * Should be called at app start before running any other native code
   * (other than the native methods in DirectoryInitialization).
   */
  public static native void Initialize();

  /**
   * Tells analytics that Dolphin has been started.
   *
   * Since users typically don't explicitly close Android apps, it's appropriate to
   * call this not only when the app starts but also when the user returns to the app
   * after not using it for a significant amount of time.
   */
  public static native void ReportStartToAnalytics();

  /**
   * Begins emulation.
   */
  public static native void Run(String[] path);

  /**
   * Begins emulation from the specified savestate.
   */
  public static native void Run(String[] path, String savestatePath, boolean deleteSavestate);

  public static native void ChangeDisc(String path);

  // Surface Handling
  public static native void SurfaceChanged(Surface surf);

  public static native void SurfaceDestroyed();

  /**
   * Unpauses emulation from a paused state.
   */
  public static native void UnPauseEmulation();

  /**
   * Pauses emulation.
   */
  public static native void PauseEmulation();

  /**
   * Stops emulation.
   */
  public static native void StopEmulation();

  public static native void WaitUntilDoneBooting();

  /**
   * Returns true if emulation is running (or is paused).
   */
  public static native boolean IsRunning();

  /**
   * Enables or disables CPU block profiling
   *
   * @param enable
   */
  public static native void SetProfiling(boolean enable);

  /**
   * Writes out the block profile results
   */
  public static native void WriteProfileResults();

  /**
   * Native EGL functions not exposed by Java bindings
   **/
  public static native void eglBindAPI(int api);

  /**
   * Provides a way to refresh the connections on Wiimotes
   */
  public static native void RefreshWiimotes();

  public static native void ReloadWiimoteConfig();

  public static native boolean InstallWAD(String file);

  public static native String FormatSize(long bytes, int decimals);

  private static boolean alertResult = false;

  public static boolean displayAlertMsg(final String caption, final String text,
          final boolean yesNo)
  {
    Log.error("[NativeLibrary] Alert: " + text);
    final EmulationActivity emulationActivity = sEmulationActivity.get();
    boolean result = false;
    if (emulationActivity == null)
    {
      Log.warning("[NativeLibrary] EmulationActivity is null, can't do panic alert.");
    }
    else
    {
      // Create object used for waiting.
      final Object lock = new Object();
      AlertDialog.Builder builder = new AlertDialog.Builder(emulationActivity,
              R.style.DolphinDialogBase)
              .setTitle(caption)
              .setMessage(text);

      // If not yes/no dialog just have one button that dismisses modal,
      // otherwise have a yes and no button that sets alertResult accordingly.
      if (!yesNo)
      {
        builder
                .setCancelable(false)
                .setPositiveButton("OK", (dialog, whichButton) ->
                {
                  dialog.dismiss();
                  synchronized (lock)
                  {
                    lock.notify();
                  }
                });
      }
      else
      {
        alertResult = false;

        builder
                .setPositiveButton("Yes", (dialog, whichButton) ->
                {
                  alertResult = true;
                  dialog.dismiss();
                  synchronized (lock)
                  {
                    lock.notify();
                  }
                })
                .setNegativeButton("No", (dialog, whichButton) ->
                {
                  alertResult = false;
                  dialog.dismiss();
                  synchronized (lock)
                  {
                    lock.notify();
                  }
                });
      }

      // Show the AlertDialog on the main thread.
      emulationActivity.runOnUiThread(builder::show);

      // Wait for the lock to notify that it is complete.
      synchronized (lock)
      {
        try
        {
          lock.wait();
        }
        catch (Exception e)
        {
        }
      }

      if (yesNo)
        result = alertResult;
    }
    return result;
  }

  public static void setEmulationActivity(EmulationActivity emulationActivity)
  {
    Log.verbose("[NativeLibrary] Registering EmulationActivity.");
    sEmulationActivity = new WeakReference<>(emulationActivity);
  }

  public static void clearEmulationActivity()
  {
    Log.verbose("[NativeLibrary] Unregistering EmulationActivity.");

    sEmulationActivity.clear();
  }

  public static void updateTouchPointer()
  {
    final EmulationActivity emulationActivity = sEmulationActivity.get();
    if (emulationActivity == null)
    {
      Log.warning("[NativeLibrary] EmulationActivity is null.");
    }
    else
    {
      emulationActivity.runOnUiThread(emulationActivity::initInputPointer);
    }
  }

  public static native float GetGameAspectRatio();
}
