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

#ifndef GIP_GEOIMAGES_H
#define GIP_GEOIMAGES_H

#include <gip/GeoImage.h>
#include <stdint.h>

namespace gip {
    //! GeoImages class
    /*!
        The GeoImages class is a collection of GeoImage objects, but not necessarily
        with identical properties (e.g., extent, resolution)
    */
    class GeoImages {
    public:
        //! \name Constructors/Destructor
        //! Default constructor
        explicit GeoImages() {};
        //! Create collection with GeoImage objects
        //explicit GeoImages(std::vector< GeoImage > imgs) : _GeoImages(imgs) {}
        //! Open file from vector of individual files
        explicit GeoImages(std::vector<std::string> filenames) {
            //foreach(filenames.begin(); filenames.end(); []() { _GeoImages.push_back()}; )
            for (std::vector<std::string>::const_iterator i=filenames.begin(); i!=filenames.end(); i++) {
                _GeoImages.push_back(GeoImage(*i));
            }
        }

        //! Copy constructor - copies GeoResource and all bands
        GeoImages(const GeoImages& images) : _GeoImages(images._GeoImages) {}
        //! Assignment Operator
        GeoImages& operator=(const GeoImages& images) {
            if (this == &images) return *this;
            _GeoImages = images._GeoImages;
            return *this;
        }
        //! Destructor
        ~GeoImages() { _GeoImages.clear(); }

        //! \name File Information
        std::vector<std::string> Basenames() const {
            std::vector<std::string> names;
            for (unsigned int i=0; i<_GeoImages.size(); i++)
                names.push_back(_GeoImages[i].Basename());
            return names;
        }
        //! Number of bands
        unsigned int NumImages() const { return _GeoImages.size(); }
        unsigned int size() const { return _GeoImages.size(); }

        //! Best datatype of all bands
        GDALDataType DataType() const {
            // TODO - validate all, return highest precision data type in collection
            return _GeoImages[0].DataType();
        }

        //! Return number of bands in images, or -1 if they are not all the same
        unsigned int NumBands() const {
            // TODO - validate against all...reutrn -1 if different
            return _GeoImages[0].NumBands();
        }

        //! Get image (0-based index)
        GeoImage& operator[](int num) { return _GeoImages[num]; }
        //! Get image, const version
        const GeoImage& operator[](int num) const { return _GeoImages[num]; }

        //! Return bandnum from all images as a GeoImage
        // TODO this needs to validate (same footprint, etc.)
        //GeoImage& AsGeoImage(int bandnum) const {}

        //! Return union of all extents transformed to passed in SRS
        Rect<double> Extent(OGRSpatialReference srs) const;

    protected:
        //! Vector of raster bands
        std::vector< GeoImage > _GeoImages;

    }; // class GeoImage

} // namespace gip

#endif
