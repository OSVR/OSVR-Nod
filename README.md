# OSVR-Nod
> Maintained at <https://github.com/OSVR/OSVR-Nod>
>
> For details, see <http://osvr.github.io>

This is a plugin for OSVR that provides access to gesture data for Nod Ring

It is Windows-only, and requires the OpenSpatialServiceController .h & .cpp files to build. See <https://github.com/openspatial/openspatial-windows> to download the framework for Windows and <https://dev.nod.com/docs/windows/developer-guide/> for details on how to use it.

If you generate the project with CMake

Then, just put the `.dll` file from this plugin in the same directory of your OSVR server as the other plugins - usually something like `osvr-plugins-0` - and use the config file provided here as an example.


## Licenses
This plugin: Licensed under the Apache License, Version 2.0.

Note that since this builds with OpenSpatialServiceController, which uses Windows libraries, you are responsible for complying with the terms of the corresponding licenses.
