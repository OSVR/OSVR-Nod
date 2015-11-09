/** @file
    @brief Implementation

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
#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>

// Library/third-party includes
// - none

// Standard includes
#include <iostream>

#define ANALOG
#define BUTTON
#define ORIENTATION
#define DUAL

void TriggerCallback(void * /*userdata*/, const OSVR_TimeValue * /*timestamp*/,
                      const OSVR_AnalogReport *report) {
    std::cout << "Got analog data report" << report->sensor << ": " << report->state << std::endl;
}

void TriggerCallback2(void * /*userdata*/, const OSVR_TimeValue * /*timestamp*/,
    const OSVR_AnalogReport *report) {
    std::cout << "Got analog data report 2 " << report->sensor << ": " << report->state << std::endl;
}

void ButtonCallback(void * /*userdata*/, const OSVR_TimeValue * /*timestamp*/,
    const OSVR_ButtonReport *report) {
    int state = report->state;
    std::cout << "Got button" << report->sensor << " report: " << state << std::endl;
}

void ButtonCallback2(void * /*userdata*/, const OSVR_TimeValue * /*timestamp*/,
    const OSVR_ButtonReport *report) {
    int state = report->state;
    std::cout << "Got button 2 " << report->sensor << " report: " << state << std::endl;
}

void OrientationCallback(void * /*userdata*/, const OSVR_TimeValue * /*timestamp*/,
    const OSVR_PoseReport *report) {
    std::cout << "Got rotation report: " << report->pose.rotation.data[0] << " " << report->pose.rotation.data[1] << " " << report->pose.rotation.data[2] << " " << report->pose.rotation.data[3] << std::endl;
}

void OrientationCallback2(void * /*userdata*/, const OSVR_TimeValue * /*timestamp*/,
    const OSVR_PoseReport *report) {
    std::cout << "Got rotation report 2: " << report->pose.rotation.data[0] << " " << report->pose.rotation.data[1] << " " << report->pose.rotation.data[2] << " " << report->pose.rotation.data[3] << std::endl;
}

int main() {
    osvr::clientkit::ClientContext context(
        "com.osvr.exampleclients.AnalogCallback");

    // This is just one of the paths: specifically, the Hydra's left
    // controller's analog trigger. More are in the docs and/or listed on
    // startup
#ifdef ANALOG
   osvr::clientkit::Interface analogTrigger =
       context.getInterface("/NodOSVRPlugin/0/analog/");
   analogTrigger.registerCallback(&TriggerCallback, NULL);
#ifdef DUAL
   osvr::clientkit::Interface analogTrigger2 =
       context.getInterface("/NodOSVRPlugin/1/analog/");
   analogTrigger2.registerCallback(&TriggerCallback2, NULL);
#endif
#endif
#ifdef BUTTON
    osvr::clientkit::Interface button =
    context.getInterface("/NodOSVRPlugin/0/button/");
    button.registerCallback(&ButtonCallback, NULL);
#ifdef DUAL
    osvr::clientkit::Interface button2 =
        context.getInterface("/NodOSVRPlugin/1/button/");
    button2.registerCallback(&ButtonCallback2, NULL);
#endif
#endif
#ifdef ORIENTATION
    osvr::clientkit::Interface orientation =
        context.getInterface("/NodOSVRPlugin/1/tracker/");
    orientation.registerCallback(&OrientationCallback, NULL);
#ifdef DUAL
    osvr::clientkit::Interface orientation2 =
        context.getInterface("/NodOSVRPlugin/1/tracker/");
    orientation2.registerCallback(&OrientationCallback2, NULL);
#endif
#endif
    // Pretend that this is your application's mainloop.
    while (true) {
        context.update();
    }

    std::cout << "Library shut down, exiting." << std::endl;
    return 0;
}
