#include "VideoInputManager.h"

VideoInputManager::VideoInputManager() {}

#ifdef __linux__
#include <filesystem>
namespace fs = std::filesystem;

size_t getVideoDeviceCount() {
  size_t count = 0;
  for (const auto &entry : fs::directory_iterator("/dev")) {
    std::string path = entry.path().string();
    if (path.find("/dev/video") == 0 && isdigit(path.back())) {
      count++;
    }
  }
  return count;
}
#endif

void VideoInputManager::setup(int width, int height) {
  input = std::make_shared<ofVideoGrabber>();
  auto devices = input->listDevices();
  suitableDeviceIDs = getSuitableDeviceIDs(devices);
  currentDeviceIndex = 0;
  if (!suitableDeviceIDs.empty()) {
    currentDeviceID = suitableDeviceIDs[currentDeviceIndex];
    input->setDeviceID(currentDeviceID);
    input->setDesiredFrameRate(30);
    input->setup(width, height);
    inputDeviceConnected = true;
  } else {
    inputDeviceConnected = false;
    ofLogError() << "No suitable capture device found!";
  }
}

void VideoInputManager::refreshInputDevices() {
  ofLogNotice() << "Refreshing video input devices...";
  input.reset();
  input = std::make_shared<ofVideoGrabber>();
  auto devices = input->listDevices();
  suitableDeviceIDs = getSuitableDeviceIDs(devices);
  if (!suitableDeviceIDs.empty()) {
    if (currentDeviceIndex >= suitableDeviceIDs.size()) {
      currentDeviceIndex = 0;
    }
    currentDeviceID = suitableDeviceIDs[currentDeviceIndex];
    input->setDeviceID(currentDeviceID);
    input->setup(ofGetWidth(), ofGetHeight());
    inputDeviceConnected = true;
    ofLogNotice() << "Auto-selected video device: " << currentDeviceID;
  } else {
    inputDeviceConnected = false;
    ofLogError() << "No suitable capture device found after refresh!";
  }
}

void VideoInputManager::incrementDeviceID() {
  refreshInputDevices();
  auto devices = input->listDevices();
  suitableDeviceIDs = getSuitableDeviceIDs(devices);
  if (!suitableDeviceIDs.empty()) {
    currentDeviceIndex = (currentDeviceIndex + 1) % suitableDeviceIDs.size();
    currentDeviceID = suitableDeviceIDs[currentDeviceIndex];
    ofLog() << "Device ID set to: " << currentDeviceID;
    input->close();
    input->setDeviceID(currentDeviceID);
    input->setup(ofGetWidth(), ofGetHeight());
  } else {
    ofLogError() << "No suitable capture device found!";
  }
}

std::vector<int> VideoInputManager::getSuitableDeviceIDs(
    const std::vector<ofVideoDevice> &devices) {
  std::vector<int> ids;
  for (const auto &d : devices) {
    if (d.deviceName.find("bcm2835") == std::string::npos) {
      ids.push_back(d.id);
    }
  }
  return ids;
}

std::shared_ptr<ofVideoGrabber> VideoInputManager::getInput() { return input; }

int VideoInputManager::getCurrentDeviceID() const { return currentDeviceID; }

int VideoInputManager::getCurrentDeviceIndex() const {
  return currentDeviceIndex;
}

bool VideoInputManager::isInputDeviceConnected() const {
  return inputDeviceConnected;
}

void VideoInputManager::update() {
  if (input && input->isInitialized()) {
    input->update();
  }

  // Only poll if device is connected and initialized
  if (inputDeviceConnected && input && input->isInitialized()) {
    uint64_t now = ofGetElapsedTimeMillis();
    if (now - lastInputPollTime > inputPollInterval) {

#ifdef __linux__
      size_t currentCount = getVideoDeviceCount();
      if (currentCount != lastVideoDeviceCount) {
        deviceChangeDetected = true;
        lastVideoDeviceCount = currentCount;
        ofLogNotice() << "Linux /dev/video* count changed: " << currentCount;
      }
#endif

      auto devices = input->listDevices();
      auto currentIDs = getSuitableDeviceIDs(devices);

      if (currentIDs != lastSuitableDeviceIDs) {
        deviceChangeDetected = true;
        lastSuitableDeviceIDs = currentIDs;
        ofLogNotice() << "Video device change detected!";
      }

      // ofLog() << "Polling video input devices...";
      suitableDeviceIDs = getSuitableDeviceIDs(devices);
      // ofLog() << "Found " << suitableDeviceIDs.size() << " suitable video
      // input devices.";

      // If the current device is no longer in the list, mark as disconnected
      if (std::find(suitableDeviceIDs.begin(), suitableDeviceIDs.end(),
                    currentDeviceID) == suitableDeviceIDs.end()) {
        inputDeviceConnected = false;
        input->close();
        if (input->isInitialized()) {
          ofLogWarning() << "input still reports initialized after close()!";
        }
        ofLogError() << "Current video input device disconnected!";
      }
      lastInputPollTime = now;
    }
  }
}

void VideoInputManager::close() {
  if (input && input->isInitialized()) {
    input->close();
    inputDeviceConnected = false;
    ofLogNotice() << "Video input device closed.";
  }
}
