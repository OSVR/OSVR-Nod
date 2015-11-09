/** @file
    @brief Nod Plugin Implementation for OSVR

    Enables Nod Data to be sent from the Nod service
    into the OSVR server and used as an osvr object

    Exposes:
    3 analog (joystick x, joystick y, trigger)
    10 buttons
    1 Pose (x, y and z are still in development but orientation works)

    these are exposed one per a nod device that this plugin detects.
    Note: currently hardware detection does not refresh, please pair your
    Nod devices before running the OSVR server.

    @date 2015

    @author
    Nod, Inc
    <http://www.nod.com/>
*/

// Copyright 2015 Nod, Inc.
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


// Anonymous namespace to avoid symbol collision
namespace {

void euler(const float& pitch, const float& roll, const float& yaw, OSVR_OrientationState* state)
{
    const float sinHalfYaw = (float)sin(yaw / 2.0f);
    const float cosHalfYaw = (float)cos(yaw / 2.0f);
    const float sinHalfPitch = (float)sin(pitch / 2.0f);
    const float cosHalfPitch = (float)cos(pitch / 2.0f);
    const float sinHalfRoll = (float)sin(roll / 2.0f);
    const float cosHalfRoll = (float)cos(roll / 2.0f);

    //W
    state->data[0] = cosHalfRoll * cosHalfPitch * cosHalfYaw
        + sinHalfRoll * sinHalfPitch * sinHalfYaw;
    //X
    state->data[1] = cosHalfRoll * sinHalfPitch * sinHalfYaw
        + cosHalfPitch * cosHalfYaw * sinHalfRoll;
    //Y
    state->data[2] = cosHalfRoll * cosHalfPitch * sinHalfYaw
        - sinHalfRoll * cosHalfYaw * sinHalfPitch;
    //Z
    state->data[3] = cosHalfYaw*sinHalfPitch*cosHalfRoll -
        sinHalfYaw*cosHalfPitch*sinHalfRoll;
}

class Backspin {
public:
    Backspin(OSVR_PluginRegContext ctx, int deviceNumber) {
        /// Create the initialization options
        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

        /// config channels
        osvrDeviceAnalogConfigure(opts, &m_analog, 3);
        osvrDeviceButtonConfigure(opts, &m_button, 10);
        osvrDeviceTrackerConfigure(opts, &m_orientation);
        /// Create the device token with the options
        m_dev.initAsync(ctx, std::to_string(deviceNumber).c_str(), opts);

        /// Send JSON descriptor
        m_dev.sendJsonDescriptor(NodOSVR);
    }
    osvr::pluginkit::DeviceToken m_dev;
    OSVR_AnalogDeviceInterface m_analog;
    OSVR_ButtonDeviceInterface m_button;
    OSVR_TrackerDeviceInterface m_orientation;
    OSVR_OrientationState lastOrientation;
    OSVR_PositionState lastPosition;
};

std::vector<Backspin*> devices;

void OpenSpatialEventFired(NodEvent ev)
{
    if (ev.type == EventType::ServiceReady)
    {
        std::cout << "[NOD PLUGIN] Service Ready" << std::endl;
        for (int i = 0; i < NodNumRings(); i++)
        {
            NodSubscribe(Modality::GameControlMode, NodGetRingName(i));
            NodSubscribe(Modality::ButtonMode, NodGetRingName(i));
            NodSubscribe(Modality::EulerMode, NodGetRingName(i));
        }
        NodSubscribe(Modality::TranslationMode, NodGetRingName(0));
    }
    if (ev.type == EventType::AnalogData)
    {
        Backspin* dev = devices.at(ev.sender);
        OSVR_AnalogState analogval[3];
        analogval[0] = ev.trigger;
        analogval[1] = ev.x;
        analogval[2] = ev.y;
        osvrDeviceAnalogSetValues(dev->m_dev, dev->m_analog, analogval, 3);
    }
    if (ev.type == EventType::Button)
    {
        Backspin* dev = devices.at(ev.sender);
        int index = ev.buttonID;
        int val = (ev.buttonState == UP) ? 0 : 1;
        osvrDeviceButtonSetValue(dev->m_dev, dev->m_button, val, index);
    }
    if (ev.type == EventType::EulerAngles)
    {
        Backspin* dev = devices.at(ev.sender);
        OSVR_OrientationState orientationval;
        euler(-ev.roll, -ev.pitch, ev.yaw, &orientationval);
        OSVR_Pose3 pose;
        pose.rotation = orientationval;
        pose.translation = dev->lastPosition;
        dev->lastOrientation = orientationval;
        osvrDeviceTrackerSendPose(dev->m_dev, dev->m_orientation, &pose, 0);
    }
    if (ev.type == EventType::Translation)
    {
        Backspin* dev = devices.at(0);
        OSVR_PositionState positionval;
        positionval.data[0] = -1*ev.xf/1000;
        positionval.data[1] = ev.yf/1000;
        positionval.data[2] = -1*ev.zf/1000;
        OSVR_Pose3 pose;
        pose.translation = positionval;
        pose.rotation = dev->lastOrientation;
        dev->lastPosition = positionval;
        osvrDeviceTrackerSendPose(dev->m_dev, dev->m_orientation, &pose, 0);
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
                    Backspin* dev = new Backspin(ctx, i);
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
