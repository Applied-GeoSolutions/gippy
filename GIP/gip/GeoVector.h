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

#ifndef GIP_GEOVECTOR_H
#define GIP_GEOVECTOR_H

#include <string>

#include <gdal/ogrsf_frmts.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include <gip/geometry.h>
#include <gip/GeoVectorResource.h>
#include <gip/GeoFeature.h>

namespace gip {

    class GeoVector : public GeoVectorResource {
    public:

        //! \name Constructors/Destructors
        //! Default constructor
        GeoVector() 
            : GeoVectorResource() {}
        //! Open existing layer from source
        GeoVector(std::string filename, std::string layer="")
            : GeoVectorResource(filename, layer) {
            _Layer->ResetReading();
            OGRFeature* feature;
            while( (feature = _Layer->GetNextFeature()) != NULL) {
                std::cout << "feature" << feature->GetFID() << std::endl;
                boost::shared_ptr<OGRFeature> f; 
                f.reset(feature, OGRFeature::DestroyFeature);
                _Features.push_back(f);
                std::cout << "feature use_count " << f.use_count() << std::endl;
            }
        }
        //! Create new file vector layer
        //GeoVector(std::string filename, OGRwkbGeometryType dtype);

        //! Copy constructor
        GeoVector(const GeoVector& vector)
            : GeoVectorResource(vector) {
            _Features = vector._Features;
        }
        //! Assignment operator
        GeoVector& operator=(const GeoVector& vector) {
            if (this == &vector) return *this;
            GeoVectorResource::operator=(vector);
            _Features = vector._Features;
            return *this;
        }
        //! Destructor
        ~GeoVector() {
            if (Options::Verbose() > 3)
                std::cout << "~GeoVector destruct" << std::endl;
        }

        // Features
        //! Get feature (0-based index)
        GeoFeature operator[](int index) { return GeoFeature(*this, _Features[index]); }
        //! Get feature, const version
        const GeoFeature operator[](int index) const { return GeoFeature(*this, _Features[index]); }

        //! Combine into single geometry - Should be freed with OGRGeometryFactory::destroyGeometry() after use.
        /*OGRGeometry* Union() const {
            OGRGeometry* site = OGRGeometryFactory::createGeometry( wkbMultiPolygon );
            return site;
        }*/

    protected:

        // OGRFeature
        //std::vector<GeoFeature> _Features;
        std::vector< boost::shared_ptr<OGRFeature> > _Features;


    }; // class GeoVector

} // namespace gip

#endif
