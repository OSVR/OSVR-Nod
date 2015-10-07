/** @file
    @brief Comprehensive example: Implementation of a dummy Hardware Detect
   Callback that creates a dummy device when it is "detected"

    @date 2014

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2014 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/AnalogInterfaceC.h>
#include <osvr/PluginKit/ButtonInterfaceC.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>

// Generated JSON header file
#include "NodOSVR.h"
#include "NodPlugin.h"

#pragma comment(lib, "OpenSpatialDll.lib")

// Library/third-party includes
// - none

// Standard includes
#include <iostream>

// Global device count for path incrementation
int deviceCount = 0;

// Anonymous namespace to avoid symbol collision
namespace {

class Backspin {
  public:
    Backspin(OSVR_PluginRegContext ctx) {
        /// Create the initialization options
        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

        /// config channels
        osvrDeviceAnalogConfigure(opts, &m_analog, 3);
        osvrDeviceButtonConfigure(opts, &m_button, 10);
        osvrDeviceTrackerConfigure(opts, &m_orientation);
        /// Create the device token with the options
        m_dev.initAsync(ctx, std::to_string(deviceCount).c_str(), opts);

        /// Send JSON descriptor
        m_dev.sendJsonDescriptor(NodOSVR);

        /// Register update callback
        m_dev.registerUpdateCallback(this);
        
        // initialize orientation
        m_orientationval.data[0] = 0.0;
        m_orientationval.data[1] = 0.0;
        m_orientationval.data[2] = 0.0;
        m_orientationval.data[3] = 0.0;
    }
    int flip = 1;
    OSVR_ReturnCode update() {

        /// send trigger, joystick x and joystick y
        osvrDeviceAnalogSetValues(m_dev, m_analog, m_analogval, 3);

        // Don't send the first value of the button (plugin was sending 0 when it started up)
        if (buttonPressed)
        {
            //send all buttons (proto1 10 buttons)
            osvrDeviceButtonSetValues(m_dev, m_button, m_buttons, 10);
        }

        // send orientation quaternion and 0,0,0 for position (could just send orientation, but
        // pose is expected by most programs
        OSVR_Pose3 pose;
        pose.rotation = m_orientationval;
        pose.translation = { 0,0,0 };
        osvrDeviceTrackerSendPose(m_dev, m_orientation, &pose, 0);

        return OSVR_RETURN_SUCCESS;
    }

    OSVR_AnalogState m_analogval[3] = { 255, 128, 128 };
    OSVR_ButtonState m_buttons[10] = { 0,0,0,0,0,0,0,0,0,0 };
    OSVR_OrientationState m_orientationval;
    bool buttonPressed = false;
  private:
      osvr::pluginkit::DeviceToken m_dev;
      OSVR_AnalogDeviceInterface m_analog;
      OSVR_ButtonDeviceInterface m_button;
      OSVR_TrackerDeviceInterface m_orientation;
};

std::vector<Backspin*> devices;

void euler(const float& pitch, const float& roll, const float& yaw, double* mData)
{
    const float sinHalfYaw = (float)sin(yaw / 2.0f);
    const float cosHalfYaw = (float)cos(yaw / 2.0f);
    const float sinHalfPitch = (float)sin(pitch / 2.0f);
    const float cosHalfPitch = (float)cos(pitch / 2.0f);
    const float sinHalfRoll = (float)sin(roll / 2.0f);
    const float cosHalfRoll = (float)cos(roll / 2.0f);

    mData[0] = -cosHalfRoll * sinHalfPitch * sinHalfYaw
        + cosHalfPitch * cosHalfYaw * sinHalfRoll;
    mData[1] = cosHalfRoll * cosHalfYaw * sinHalfPitch
        + sinHalfRoll * cosHalfPitch * sinHalfYaw;
    mData[2] = cosHalfRoll * cosHalfPitch * sinHalfYaw
        - sinHalfRoll * cosHalfYaw * sinHalfPitch;
    mData[3] = cosHalfRoll * cosHalfPitch * cosHalfYaw
        + sinHalfRoll * sinHalfPitch * sinHalfYaw;
}

void OpenSpatialEventFired(NodEvent ev)
{
    if (ev.type == EventType::ServiceReady)
    {
        std::cout << "[NOD PLUGIN] Service Ready" << std::endl;
        NodSubscribe(Modality::GameControlMode, "nod2-004008");
        NodSubscribe(Modality::ButtonMode, "nod2-004008");
        NodSubscribe(Modality::EulerMode, "nod2-004008");
    }
    if (ev.type == EventType::AnalogData)
    {
        Backspin* dev = devices.at(0);
        dev->m_analogval[0] = ev.trigger;
        dev->m_analogval[1] = ev.x;
        dev->m_analogval[2] = ev.y;
    }
    if (ev.type == EventType::Button)
    {
        Backspin* dev = devices.at(0);
        int index = ev.buttonID;
        dev->buttonPressed = true;
        dev->m_buttons[index] = (ev.buttonState == UP) ? 0 : 1;
    }
    if (ev.type == EventType::EulerAngles)
    {
        Backspin* dev = devices.at(0);
        double val[4];
        euler(ev.pitch, ev.roll, ev.yaw, val);
        dev->m_orientationval.data[0] = val[0];
        dev->m_orientationval.data[1] = val[1];
        dev->m_orientationval.data[2] = val[2];
        dev->m_orientationval.data[3] = val[3];
    }
}

class HardwareDetection {
  public:
    HardwareDetection() : m_found(false) { NodInitialize(OpenSpatialEventFired); }
    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {
        std::cout << "[NOD PLUGIN] Got a hardware detection request" << std::endl;
        if (!m_found)
        {
            if (NodNumRings() > 0)
            {
                std::cout << "[NOD PLUGIN] found device" << std::endl;
                for (int i = 0; i < NodNumRings(); i++)
                {
                    Backspin* dev = new Backspin(ctx);
                    devices.push_back(dev);
                    osvr::pluginkit::registerObjectForDeletion(
                        ctx, dev);
                    m_found = true;
                }
            }
            else
            {
                std::cout << "[NOD PLUGIN]: no devices found" << std::endl;
            }
        }
        return OSVR_RETURN_SUCCESS;
    }

  private:
    /// @brief Have we found our device yet? (this limits the plugin to one
    /// instance)
    bool m_found;
};
} // namespace

OSVR_PLUGIN(com_osvr_example_selfcontained) {
    osvr::pluginkit::PluginContext context(ctx);

    /// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}
