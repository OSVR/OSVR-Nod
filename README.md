# OSVR-Nod
> Maintained at <https://github.com/OSVR/OSVR-Nod>
>
> For details, see <http://osvr.github.io>

This is a plugin for OSVR that provides access to Nod Ring and Backspin interfaces such as analog, button, orientation, etc.

It is Windows-only, and requires the NodPlugin.h, Settings.h, OpenSpatial.dll and .lib files to build (from the OpenSpatial Windows SDK). See <https://github.com/openspatial/openspatial-windows> to download the framework for Windows and <https://dev.nod.com/docs/windows/developer-guide/> for details on how to use it. They should be copied to the source folder.

Once the project is successfully built, just put the `.dll` file from this plugin in the same directory of your OSVR server as the other plugins - usually something like `osvr-plugins-0` - and use the config file provided here as an example.

NOTE: this plugin is compatible with the Nod OpenSpatial Windows SDK v1686 and newer. Please upgrade to this version through the aformentioned github repository. If you must use an older version of the Nod OpenSpatial Windows SDK feel free to browse the revision history.

Paths exposed in the plugin:

Currently the plugin exposes a pose interface (position is currently not implemented and will be 0 for x,y,z), a button interface with 10 buttons and 3 analog interfaces

/NodOSVRPlugin/(device-number)/analog
                              /tracker
                              /button
							  
is the path to the interfaces, device-number is the id of the device you want,
starts from 0 and increases.

 

## Licenses
This plugin: Licensed under the Apache License, Version 2.0. See accompanying License file for details and https://github.com/openspatial/openspatial-windows for details about OpenSpatial License.
