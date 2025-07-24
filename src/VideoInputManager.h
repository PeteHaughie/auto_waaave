#pragma once

#include "ofMain.h"

class VideoInputManager {
public:
  VideoInputManager();
  void setup(int width, int height);
  void refreshInputDevices();
  void incrementDeviceID();
  std::shared_ptr<ofVideoGrabber> getInput();
  std::vector<int>
  getSuitableDeviceIDs(const std::vector<ofVideoDevice> &devices);
  int getCurrentDeviceID() const;
  int getCurrentDeviceIndex() const;
  bool isInputDeviceConnected() const;
  void update();
  void close();

  // Polling
  void setInputPollInterval(uint64_t interval);
  uint64_t getInputPollInterval() const;

#ifdef __linux__
  size_t lastVideoDeviceCount = 0;
#endif

private:
  std::shared_ptr<ofVideoGrabber> input;
  std::vector<int> suitableDeviceIDs;
  std::vector<int> lastSuitableDeviceIDs;
  int currentDeviceID = -1;
  int currentDeviceIndex = 0;
  bool inputDeviceConnected = false;
  uint64_t lastInputPollTime = 0;
  uint64_t inputPollInterval = 1000; // ms
  bool needsGrabberRefresh = false;
  bool deviceChangeDetected = false;
};