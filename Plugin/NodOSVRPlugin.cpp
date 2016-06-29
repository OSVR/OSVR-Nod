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
#define _USE_MATH_DEFINES
#include <math.h>
#include <map>

// Global device count for path incrementation
int deviceCount = 0;

// Anonymous namespace to avoid symbol collision
namespace {

std::vector<float> euler(const float& pitch, const float& roll, const float& yaw)
{
    const float sinHalfYaw = (float)sin(yaw / 2.0f);
    const float cosHalfYaw = (float)cos(yaw / 2.0f);
    const float sinHalfPitch = (float)sin(pitch / 2.0f);
    const float cosHalfPitch = (float)cos(pitch / 2.0f);
    const float sinHalfRoll = (float)sin(roll / 2.0f);
    const float cosHalfRoll = (float)cos(roll / 2.0f);

    std::vector<float> eulerOutput;
    eulerOutput.push_back(cosHalfYaw*sinHalfPitch*cosHalfRoll -
        sinHalfYaw*cosHalfPitch*sinHalfRoll);
    eulerOutput.push_back(cosHalfRoll * cosHalfPitch * sinHalfYaw
        - sinHalfRoll * cosHalfYaw * sinHalfPitch);
    eulerOutput.push_back(cosHalfRoll * sinHalfPitch * sinHalfYaw
        + cosHalfPitch * cosHalfYaw * sinHalfRoll);
    eulerOutput.push_back(-1.0 * cosHalfRoll * cosHalfPitch * cosHalfYaw
        + sinHalfRoll * sinHalfPitch * sinHalfYaw);

    return eulerOutput;
}

class Backspin {
public:
    Backspin(OSVR_PluginRegContext ctx) {
        /// Create the initialization options
        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

        /// config channels
        osvrDeviceAnalogConfigure(opts, &m_analog, 3);
        osvrDeviceButtonConfigure(opts, &m_button, 10);
        osvrDeviceTrackerConfigure(opts, &m_pose);
        /// Create the device token with the options
        m_dev.initAsync(ctx, std::to_string(deviceCount).c_str(), opts);
        deviceCount++;
        /// Send JSON descriptor
        m_dev.sendJsonDescriptor(NodOSVR);;
    }
    osvr::pluginkit::DeviceToken m_dev;
    OSVR_AnalogDeviceInterface m_analog;
    OSVR_ButtonDeviceInterface m_button;
    OSVR_TrackerDeviceInterface m_pose;

    OSVR_OrientationState lastOrientation = { 0, 0, 0, 0 };
    OSVR_PositionState lastPosition = { 0, 0, 0 };
};

std::map<std::string, Backspin*> devices;

void OpenSpatialEventFired(NodEvent ev)
{
    std::string deviceName;
    if (strcmp(ev.sender, "right") == 0)
    {
        deviceName = "right";
    }
    else if (strcmp(ev.sender, "left") == 0)
    {
        deviceName = "left";
    }
    else
    {
        deviceName = "head";
    }

    if (ev.type == EventType::ServiceReady)
    {
        std::cout << "[NOD PLUGIN] Service ready for device \"" << deviceName << "\"." << std::endl;
        for (int i = 0; i < NodNumDevices(); i++)
        {
            NodSubscribe(Modality::GameControlMode, NodGetDeviceName(i));
            NodSubscribe(Modality::ButtonMode, NodGetDeviceName(i));
            NodSubscribe(Modality::EulerMode, NodGetDeviceName(i));
        }
    }
    Backspin* dev = devices[deviceName];
    if (dev) {
        float x, y, z, w;
        OSVR_PoseState pose_state;
        switch (ev.type) {
            case EventType::AnalogData: 
                {
                    OSVR_AnalogState analogval[3];
                    analogval[0] = ev.trigger;
                    analogval[1] = ev.x;
                    analogval[2] = ev.y;
                    osvrDeviceAnalogSetValues(dev->m_dev, dev->m_analog, analogval, 3);
                }	
                break;
            case EventType::Button:
                {
                    int index = ev.buttonID;
                    int val = (ev.buttonState == UP) ? 0 : 1;
                    osvrDeviceButtonSetValue(dev->m_dev, dev->m_button, val, index);
                }
                break;
            case EventType::EulerAngles:
                {
                    OSVR_OrientationState pose_orientation;
                    std::vector<float> eulerProc;
                    eulerProc = euler(-ev.pitch, -ev.roll, ev.yaw);
                    pose_orientation.data[0] = eulerProc[0];
                    pose_orientation.data[1] = eulerProc[1];
                    pose_orientation.data[2] = eulerProc[2];
                    pose_orientation.data[3] = eulerProc[3];
                    osvrPose3SetIdentity(&pose_state);
                    pose_state.rotation = pose_orientation;
                    pose_state.translation = dev->lastPosition;
                    dev->lastOrientation = pose_orientation;
                    osvrDeviceTrackerSendPose(dev->m_dev, dev->m_pose, &pose_state, 0);
                }
                break;
            case EventType::Translation:
                {
                    OSVR_PositionState pose_translation;
                    x = -1.0 * ev.xf / 1000.0;
                    y = ev.yf / 1000.0;
                    z = -1.0 * ev.zf / 1000.0;
                    pose_translation.data[0] = x;
                    pose_translation.data[1] = z;
                    pose_translation.data[2] = -y;
                    pose_state.translation = pose_translation;
                    pose_state.rotation = dev->lastOrientation;
                    dev->lastPosition = pose_translation;
                    osvrDeviceTrackerSendPose(dev->m_dev, dev->m_pose, &pose_state, 0);
                }
                break;
        }
    }
}

class HardwareDetection {
  public:
    HardwareDetection() : m_found(false) { NodInitialize(OpenSpatialEventFired); }
    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {
        std::cout << "[NOD PLUGIN] Got a hardware detection request." << std::endl;
        if (!m_found)
        {
            if (NodNumDevices() > 0)
            {
                std::cout << "[NOD PLUGIN] Found a device." << std::endl;
                for (int i = 0; i < NodNumDevices(); i++)
                {
                    std::string deviceName = NodGetDeviceName(i);
                    std::cout << "[NOD PLUGIN] Device name: " << NodGetDeviceName(i) << std::endl;
                    Backspin* dev = new Backspin(ctx);
                    devices[deviceName] = dev;
                    osvr::pluginkit::registerObjectForDeletion(
                        ctx, dev);
                    m_found = true;
                }
            }
            else
            {
                std::cout << "[NOD PLUGIN]: No devices found." << std::endl;
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

OSVR_PLUGIN(NodOSVRPlugin) {
    osvr::pluginkit::PluginContext context(ctx);

    /// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}