#pragma once
extern "C"
{
  void renderSettingsOverlay();
  void showStartLifeScreen();
  void teardownSettingsOverlay();
  void resetLastSettingsTheme();  // Reset theme tracker for preset changes
}