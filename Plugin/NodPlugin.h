/* Copyright 2015 Nod Labs */

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the NODPLUGIN_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// NODPLUGIN_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#ifdef _WIN32
#ifdef NODPLUGIN_EXPORTS
#define NODPLUGIN_API __declspec(dllexport)
#else
#define NODPLUGIN_API __declspec(dllimport)
#endif
#else
#define NODPLUGIN_API
#endif

#include "Settings.h"
#include <vector>
#include <string>
#include <sstream>

const static int NOD_NONE = -1;

enum Modality {
    ButtonMode = 1,
    AccelMode = 2,
    EulerMode = 3,
    GameControlMode = 4,
    GestureMode = 5,
    PointerMode = 6,
    SliderMode = 7,
    TranslationMode = 9,
    GyroMode = 10
};

enum EventType {
    Button,
    Accelerometer,
    EulerAngles,
    AnalogData,
    Gestures,
    Pointer,
    Slider,
    DataMode,
    Translation,
    Gyroscope,
    DeviceInfo,
    ServiceReady,
    NONE_T = -1
};

enum UpDown {
    NONE_UD = -1,
    UP = 0,
    DOWN = 1
};

enum GestureType {
    NONE_G = -1,
    SWIPE_DOWN = 0,
    SWIPE_LEFT = 1,
    SWIPE_RIGHT = 2,
    SWIPE_UP = 3,
    CW = 4,
    CCW = 5,
};

enum SliderType {
    NONE_S = -1,
    SLIDE_R = 0,
    SLIDE_L = 1
};

extern "C" {

    struct NodUniqueID
    {
        char byte0;
        char byte1;
        char byte2;
    };

    struct FirmwareVersion
    {
        int major = NOD_NONE;
        int minor = NOD_NONE;
        int subminor = NOD_NONE;
    };

    struct NodEvent {
        EventType type = NONE_T;
        int x = NOD_NONE;
        int y = NOD_NONE;
        int z = NOD_NONE;
        float xf = NOD_NONE;
        float yf = NOD_NONE;
        float zf = NOD_NONE;
        int trigger = NOD_NONE;
        float roll = NOD_NONE;
        float pitch = NOD_NONE;
        float yaw = NOD_NONE;
        float buttonID = NOD_NONE;
        UpDown buttonState = NONE_UD;
        int batteryPercent = NOD_NONE;
        GestureType gesture = NONE_G;
        FirmwareVersion firmwareVersion;
        const char* sender = "";
        SliderType slider = NONE_S;
    };

    // Initialize Nod Client with function pointer for all callback events
    NODPLUGIN_API bool NodInitialize(void(*evFiredFn)(NodEvent));

    // Shutdown and cleanup Nod Client
    NODPLUGIN_API bool NodShutdown(void);

    // Refresh the Nod Client to discover new devices
    NODPLUGIN_API bool NodRefresh(void);
    // Retrieve the number of Devices
    NODPLUGIN_API int  NodNumDevices(void);

    // Return a char array of device names, seperated by a whitespace delimiter
    // Use NodGetDeviceNames if instead you would like a vector of strings
    const NODPLUGIN_API char* NodGetDeviceName(int ringID);

    // Subscribe a device to a given mode
    NODPLUGIN_API bool NodSubscribe(Modality mode, const char* deviceName);
    // Unsubscribe a device from a given mode
    NODPLUGIN_API bool NodUnsubscribe(Modality mode, const char* deviceName);

    // Retrieve information for a device
    NODPLUGIN_API bool NodRequestDeviceInfo(const char* deviceName);
    // Change Settings of a device
    NODPLUGIN_API bool NodChangeSetting(const char* deviceName, Settings setting, int args[], int numArgs);
} // end extern C+