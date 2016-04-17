/*##############################################################################
#    GIPPY: Geospatial Image Processing library for Python
#
#    AUTHOR: Matthew Hanson
#    EMAIL:  matt.a.hanson@gmail.com
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

#include <cstdio>
#include <gip/GeoResource.h>
#include <gip/Utils.h>


namespace gip {
    using std::string;
    using std::vector;

    // Options given initial values here
    string Options::_DefaultFormat("GTiff");
    float Options::_ChunkSize(128.0);
    int Options::_Verbose(1);
    int Options::_NumCores(2);
    string Options::_WorkDir("/tmp/");  

    // Constructors
    GeoResource::GeoResource(string filename, bool update, bool temp)
        : _Filename(filename), _temp(temp) {

        // read/write permissions
        GDALAccess access = update ? GA_Update : GA_ReadOnly;
        if (access == GA_ReadOnly)
            CPLSetConfigOption("GDAL_PAM_ENABLED","NO");
        else CPLSetConfigOption("GDAL_PAM_ENABLED",NULL);

        // open dataset
        GDALDataset* ds = (GDALDataset*)GDALOpen(_Filename.c_str(), access);
        // Check if Update access not supported
        if (ds == NULL) // && CPLGetLastErrorNo() == 6)
            ds = (GDALDataset*)GDALOpen(_Filename.c_str(), GA_ReadOnly);
        if (ds == NULL) {
            throw std::runtime_error(to_string(CPLGetLastErrorNo()) + ": " + string(CPLGetLastErrorMsg()));
        }
        _GDALDataset.reset(ds);

        if (Options::Verbose() > 4)
            std::cout << Basename() << ": GeoResource Open (use_count = " << _GDALDataset.use_count() << ")" << std::endl;
    }

    //! Create new file
    GeoResource::GeoResource(int xsz, int ysz, int bsz, DataType dt, string filename, bool temp)
        : _Filename(filename), _temp(temp) {

        // format, driver, and file extension
        string format = Options::DefaultFormat();
        //if (format == "GTiff") options["COMPRESS"] = "LZW";
        GDALDriver *driver = GetGDALDriverManager()->GetDriverByName(format.c_str());
        // TODO check for null driver and create method
        // check if extension (case insensitive) is already in filename
        string ext = driver->GetMetadataItem(GDAL_DMD_EXTENSION);
        string curext = Extension();
        if ((to_lower(ext) != to_lower(curext)) && ext != "") {
            _Filename = _Filename + '.' + ext;
        }

        // add options
        char **papszOptions = NULL;
        /*if (options.size()) {
            for (dictionary::const_iterator imap=options.begin(); imap!=options.end(); imap++)
                papszOptions = CSLSetNameValue(papszOptions,imap->first.c_str(),imap->second.c_str());
        }*/

        // create file
        //BOOST_LOG_TRIVIAL(info) << Basename() << ": create new file " << xsz << " x " << ysz << " x " << bsz << std::endl;
        if (Options::Verbose() > 4)
            std::cout << Basename() << ": create new file " << xsz << " x " << ysz << " x " << bsz << std::endl;
        _GDALDataset.reset( driver->Create(_Filename.c_str(), xsz,ysz,bsz, dt.gdal(), papszOptions) );
        if (_GDALDataset.get() == NULL) {
            std::cout << "Error creating " << _Filename << CPLGetLastErrorMsg() << std::endl;
        }
    }

    GeoResource::GeoResource(const GeoResource& resource)
        : _Filename(resource._Filename), _GDALDataset(resource._GDALDataset), _temp(resource._temp) {}

    GeoResource& GeoResource::operator=(const GeoResource& resource) {
        if (this == &resource) return *this;
        _Filename = resource._Filename;
        _GDALDataset = resource._GDALDataset;
        _temp = resource._temp;
        return *this;
    }

    GeoResource::~GeoResource() {
        // flush GDALDataset if last open pointer
        if (_GDALDataset.unique()) {
            _GDALDataset->FlushCache();
            //BOOST_LOG_TRIVIAL(trace) << Basename() << ": ~GeoResource (use_count = " << _GDALDataset.use_count() << ")" << std::endl;
            if (Options::Verbose() > 4) std::cout << Basename() << ": ~GeoResource (use_count = " << _GDALDataset.use_count() << ")" << std::endl;
            if (_temp) {
                std::remove(_Filename.c_str());
            }
        }
    }

    // Info
    //! Get full filename
    string GeoResource::Filename() const {
        return _Filename;
    }

    //! Return basename of filename (no path, no extension)
    string GeoResource::Basename() const {
        return gip::Basename(_Filename);
    }

    //! Get extension of filename
    string GeoResource::Extension() const {
        return gip::Extension(_Filename);
    }

    // Geospatial
    Point<double> GeoResource::GeoLoc(float xloc, float yloc) const {
        CImg<double> affine = Affine();
        Point<double> pt(affine[0] + xloc*affine[1] + yloc*affine[2], affine[3] + xloc*affine[4] + yloc*affine[5]);
        return pt;
    }

    Point<double> GeoResource::TopLeft() const { 
        return GeoLoc(0, 0); 
    }

    Point<double> GeoResource::LowerLeft() const {
        return GeoLoc(0, YSize()-1); 
    }

    Point<double> GeoResource::TopRight() const { 
        return GeoLoc(XSize()-1, 0);
    }

    Point<double> GeoResource::LowerRight() const { 
        return GeoLoc(XSize()-1, YSize()-1);
    }

    Point<double> GeoResource::MinXY() const {
        Point<double> pt1(TopLeft()), pt2(LowerRight());
        double MinX(std::min(pt1.x(), pt2.x()));
        double MinY(std::min(pt1.y(), pt2.y()));
        return Point<double>(MinX, MinY);           
    }

    Point<double> GeoResource::MaxXY() const { 
        Point<double> pt1(TopLeft()), pt2(LowerRight());
        double MaxX(std::max(pt1.x(), pt2.x()));
        double MaxY(std::max(pt1.y(), pt2.y()));
        return Point<double>(MaxX, MaxY);
    }

    Point<double> GeoResource::Resolution() const {
        CImg<double> affine = Affine();
        return Point<double>(affine[1], affine[5]);
    }

    GeoResource& GeoResource::SetCoordinateSystem(const GeoResource& res) {
        SetSRS(res.SRS());
        SetAffine(res.Affine());
        return *this;
    }

    ChunkSet GeoResource::Chunks(unsigned int padding, unsigned int numchunks) const {
        return ChunkSet(XSize(), YSize(), padding, numchunks);
    }

    // Metadata
    string GeoResource::Meta(string key) const {
        const char* item = _GDALDataset->GetMetadataItem(key.c_str());
        return (item == NULL) ? "": item;
    }

    GeoResource& GeoResource::SetMeta(string key, string item) {
        _GDALDataset->SetMetadataItem(key.c_str(), item.c_str());
        return *this;
    }

    GeoResource& GeoResource::SetMeta(std::map<string, string> items) {
        for (dictionary::const_iterator i=items.begin(); i!=items.end(); i++) {
            SetMeta(i->first, i->second);
        }
        return *this;
    }

    GeoResource& GeoResource::CopyMeta(const GeoResource& resource) {
        _GDALDataset->SetMetadata(resource._GDALDataset->GetMetadata());
        return *this;
    }


    // Get metadata group
    vector<string> GeoResource::MetaGroup(string group, string filter) const {
        char** meta= _GDALDataset->GetMetadata(group.c_str());
        int num = CSLCount(meta);
        vector<string> items;
        for (int i=0;i<num; i++) {
                if (filter != "") {
                        string md = string(meta[i]);
                        string::size_type pos = md.find(filter);
                        if (pos != string::npos) items.push_back(md.substr(pos+filter.length()));
                } else items.push_back( meta[i] );
        }
        return items;
    }

} // namespace gip
