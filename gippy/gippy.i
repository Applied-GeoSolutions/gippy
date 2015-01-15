/*##############################################################################
#    GIPPY: Geospatial Image Processing library for Python
#
#    Copyright (C) 2015 Applied Geosolutions
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
##############################################################################*/
%module gippy
%feature("autodoc", "1");
%{
    #define SWIG_FILE_WITH_INIT
    #include <gdal/gdal_priv.h>

    // Additional functions used by the SWIG interface but not used directly by users

    void gip_gdalinit() { 
       GDALAllRegister();
       CPLPushErrorHandler(CPLQuietErrorHandler);
    }
%}

// Register file formats with GDAL
void gip_gdalinit();

%include "core.i"