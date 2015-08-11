/** @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Nod Labs.
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
#include <osvr/PluginKit/GestureInterfaceC.h>

/// @todo uncomment remaining include to use those interfaces
//#include <osvr/PluginKit/ButtonInterfaceC.h>
//#include <osvr/PluginKit/TrackerInterfaceC.h>

// Generated JSON header file
#include "com_osvr_Nod_json.h"

// Library/third-party includes
#include "OpenSpatialServiceController.h"

// Standard includes
#include <iostream>
#include <memory>
#include <ctime>

// Anonymous namespace to avoid symbol collision
namespace {

// typedef std::shared_ptr<OpenSpatialServiceController> ControllerPtr;

OSVR_MessageType gestureMessage;

class NodDevice : public OpenSpatialDelegate {
  public:
    NodDevice(OSVR_PluginRegContext ctx) {
        /// Create the initialization options
        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

        osvrDeviceGestureConfigure(opts, &m_gesture);
        // @todo if you will be using more interfaces,
        // you'll need to configure them as well such
        // as above

        /// Create the sync device token with the options
        m_dev.initSync(ctx, "Nod", opts);

        /// register gesture names with OSVR
        initializeGestureMap();

        /// Send JSON descriptor
        // @todo you will need to update json descriptor
        // if you will be using button/tracker interfaces
        m_dev.sendJsonDescriptor(com_osvr_Nod_json);

        /// Register update callback
        m_dev.registerUpdateCallback(this);
    }

    OSVR_ReturnCode update() { return OSVR_RETURN_SUCCESS; }

    void buttonEventFired(ButtonEvent event) {
        /// @todo add proper handling for event using the Button interface API
    }

    void pointerEventFired(PointerEvent event) {
        /// @todo add proper handling for event
    }

    void gestureEventFired(GestureEvent event) {
        printf("\nGesture Event Fired. Gesture Type: %d from id: %d",
               event.gestureType, event.sender);

        OSVR_TimeValue timestamp;

        osvrTimeValueGetNow(&timestamp);

        OSVR_GestureState state = OSVR_GESTURE_COMPLETE;

        osvrDeviceGestureReportData(m_gesture, m_gestureMap[event.gestureType],
                                    state, 0, &timestamp);
    }

    void pose6DEventFired(Pose6DEvent event) {
        /// @todo add proper handling for event using the Tracker interface API
    }

    void gameControlEventFired(GameControlEvent event) {
        /// @todo add proper handling for event
    }

    void motion6DEventFired(Motion6DEvent event) {
        /// @todo add proper handling for event
    }

    // register Nod's gesture IDs with corresponding strings
    // required part because OSVR has a pre-set list of gestures
    // that may conflict with Nod's ID
    void initializeGestureMap() {

        OSVR_GestureID id;
        osvrDeviceGestureGetID(m_gesture, "SWIPE_DOWN", &id);
        m_gestureMap.insert(std::pair<int, OSVR_GestureID>(SWIPE_DOWN, id));
        osvrDeviceGestureGetID(m_gesture, "SWIPE_LEFT", &id);
        m_gestureMap.insert(std::pair<int, OSVR_GestureID>(SWIPE_LEFT, id));
        osvrDeviceGestureGetID(m_gesture, "SWIPE_RIGHT", &id);
        m_gestureMap.insert(std::pair<int, OSVR_GestureID>(SWIPE_RIGHT, id));
        osvrDeviceGestureGetID(m_gesture, "SWIPE_UP", &id);
        m_gestureMap.insert(std::pair<int, OSVR_GestureID>(SWIPE_UP, id));
        osvrDeviceGestureGetID(m_gesture, "CW", &id);
        m_gestureMap.insert(std::pair<int, OSVR_GestureID>(CW, id));
        osvrDeviceGestureGetID(m_gesture, "CCW", &id);
        m_gestureMap.insert(std::pair<int, OSVR_GestureID>(CCW, id));
        osvrDeviceGestureGetID(m_gesture, "SLIDER_LEFT", &id);
        m_gestureMap.insert(std::pair<int, OSVR_GestureID>(SLIDER_LEFT, id));
        osvrDeviceGestureGetID(m_gesture, "SLIDER_RIGHT", &id);
        m_gestureMap.insert(std::pair<int, OSVR_GestureID>(SLIDER_RIGHT, id));
    }

  private:
    osvr::pluginkit::DeviceToken m_dev;
    OSVR_GestureDeviceInterface m_gesture;
    // @todo add more interface variable like gesture one above
    // if you will be using those interfaces
    // OSVR_TrackerDeviceInterface m_tracker;
    // OSVR_ButtonDeviceInterface m_button;

    // map for Nod's gesture ID to OSVR gesture ID
    std::map<int, OSVR_GestureID> m_gestureMap;
};

class HardwareDetection {
  public:
    HardwareDetection() : m_found(false) {}

    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {

        std::cout << "PLUGIN: Got a hardware detection request" << std::endl;

        // if the device has been discovered we don't need to go thru hardware
        // detection again
        if (!m_found) {

            OpenSpatialServiceController *ring =
                new OpenSpatialServiceController;

            /// this will wait until the ring is setup
            /// you can increase/decrease the number of tries/time to wait
            int i = 0;
            while (!ring->setup && i < 20) {
                Sleep(500);
                i++;
            }

            if (!ring->setup) {
                std::cout << "PLUGIN: We have NOT detected Nod Ring device"
                          << std::endl;
                return OSVR_RETURN_FAILURE;
            }

            std::cout << "PLUGIN: We have detected Nod Ring! " << std::endl;

            m_found = true;
            /// Create our device object

            NodDevice *nodDev = new NodDevice(ctx);
            ring->setDelegate(nodDev);

            for (auto &ringName : ring->names) {
                ring->controlService(SUBSCRIBE_TO_GESTURE, ringName);
            }

            osvr::pluginkit::registerObjectForDeletion(ctx, nodDev);
        }

        return OSVR_RETURN_SUCCESS;
    }

    bool m_found;
};
} // namespace

OSVR_PLUGIN(com_osvr_Gesture) {

    /// don't modify this part

    osvrDeviceRegisterMessageType(ctx, "GestureMessage", &gestureMessage);

    osvr::pluginkit::PluginContext context(ctx);

    /// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}