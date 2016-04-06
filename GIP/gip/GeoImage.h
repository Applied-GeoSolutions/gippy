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

#ifndef GIP_GEOIMAGE_H
#define GIP_GEOIMAGE_H

#include <gip/GeoResource.h>
#include <gip/GeoRaster.h>
#include <stdint.h>
#include <gip/Utils.h>

namespace gip {
    //using std::string;
    //using std::vector;

    // Forward declaration
    class GeoRaster;

    //! GeoImage class
    /*!
        The GeoImage is a collection of GeoRaster objects
    */
    class GeoImage : public GeoResource {
    public:
        //! \name Constructors/Destructor
        //! Default constructor
        explicit GeoImage() : GeoResource() {}
        //! Open file constructor
        explicit GeoImage(std::string filename, bool update=false)
            : GeoResource(filename, update) {
            LoadBands();
        }
        //! Open file from vector of individual files
        explicit GeoImage(std::vector<std::string> filenames);
        //! Constructor for creating new file
        explicit GeoImage(std::string filename, int xsz, int ysz, int bsz, DataType dt=DataType("Uint8")) :
            GeoResource(xsz, ysz, bsz, dt, filename) {
            LoadBands();
        }
        //! Constructor for creating new file with same properties (xsize, ysize, metadata) as existing file
        explicit GeoImage(std::string filename, const GeoImage& image, DataType dt, int bsz) :
            GeoResource(image.XSize(), image.YSize(), bsz, dt, filename) {
            //if (datatype == GDT_Unknown) datatype = image->Type();
            //CopyMeta(image);
            SetCoordinateSystem(image);
            LoadBands();
        }
        //! Constructor for creating new file with same properties (xsize, ysize, bsize) as existing file
        explicit GeoImage(std::string filename, const GeoImage& image, DataType dt) :
            GeoResource(image.XSize(), image.YSize(), image.NumBands(), dt, filename) {
            //if (datatype == GDT_Unknown) datatype = image->Type();
            //CopyMeta(image);
            SetCoordinateSystem(image);
            LoadBands();
        }
        //! Constructor for creating new file with given properties (xsize, ysize, bsize,datatype) as existing file
        explicit GeoImage(std::string filename, const GeoImage& image) :
            GeoResource(image.XSize(), image.YSize(), image.NumBands(), image.Type(), filename) {
            //if (datatype == GDT_Unknown) datatype = image->Type();
            //CopyMeta(image);
            SetCoordinateSystem(image);
            LoadBands();
        }

        // Factory functions to support keywords in python bindings
        /*static GeoImage Open(string filename, bool update=true) {
            return GeoImage(filename, update);
        }*/

        //static GeoImage New(string filename, const GeoImage& template=GeoImage(), int xsz=0, int ysz=0, int bands=1, DataType dt=GDT_Byte)
        //! Constructor to create new file based on input vector extents
        /*explicit GeoImage(string filename, string vector, float xres, float yres, DataType datatype=GDT_Byte, int bsz=1) {
            OGRDataSource *poDS = OGRSFDriverRegistrar::Open(vector.c_str());
            OGRLayer *poLayer = poDS->GetLayer(0);
            OGREnvelope extent;
            poLayer->GetExtent(&extent, true);
            int xsize = (int)(0.5 + (extent.MaxX - extent.MinX) / xres);
            int ysize = (int)(0.5 + (extent.MaxY - extent.MinY) / yres);
            GeoResource::CreateNew(xsize, ysize, bsz, datatype, filename);
            double affine[6];
            affine[0] = extent.MinX;
            affine[1] = xres;
            affine[2] = 0;
            affine[3] = extent.MaxY;
            affine[4] = 0;
            affine[5] = -yres;
            _GDALDataset->SetGeoTransform(affine);
            char* wkt = NULL;
            poLayer->GetSpatialRef()->exportToWkt(&wkt);
            _GDALDataset->SetProjection(wkt);
            OGRDataSource::DestroyDataSource( poDS );
        }*/
        //! Copy constructor - copies GeoResource and all bands
        GeoImage(const GeoImage& image);
        //! Assignment Operator
        GeoImage& operator=(const GeoImage& image) ;
        //! Destructor
        ~GeoImage() { _RasterBands.clear(); }

        //! \name File Information
        //! Return list of filename for each band (could be duplicated)
        std::vector<std::string> Filenames() const {
            std::vector<std::string> fnames;
            for (unsigned int i=0;i<_RasterBands.size();i++) {
                fnames.push_back(_RasterBands[i].Filename());
            }
            return fnames;
        }

        DataType Type() const { return _RasterBands[0].Type(); }
        //! Return information on image as string
        std::string Info(bool=true, bool=false) const;

        //! \name Bands and colors
        //! Number of bands
        unsigned int NumBands() const { return _RasterBands.size(); }
        //! Get datatype of image (TODO - check all raster bands, return 'largest')
        //! Get vector of band names
        std::vector<std::string> BandNames() const { return _BandNames; }
        //! Set a band name
        void SetBandName(std::string desc, int bandnum) {
            try {
                // Test if color already exists
                (*this)[desc];
                throw std::out_of_range ("Band " + desc + " already exists in GeoImage!");
            } catch(...) {
                _BandNames[bandnum-1] = desc;
                _RasterBands[bandnum-1]._GDALRasterBand->SetDescription(desc.c_str());
            }            
        }
        //! Set all band names with vector size equal to # bands
        void SetBandNames(std::vector<std::string> names) {
	       if (names.size() != NumBands())
            	throw std::out_of_range("Band list size must be equal to # of bands");
            for (int i=0; i< (names.size() + 1); i++) {
                try {
                    SetBandName(names[i], i+1);
                } catch(...) {
                    // TODO - print to stderr ? or log?
                    std::cout << "Band " + names[i] + " already exists" << std::endl;
                }
            }
        }
        //! Check if this band exists
        bool BandExists(std::string desc) const {
            try {
                (*this)[desc];
                return true;
            } catch(...) {
                return false;
            } 
        }   
        //! Check if ALL these bands exist
        bool BandsExist(std::vector<std::string> desc) const {
            for (std::vector<std::string>::const_iterator i=desc.begin(); i!=desc.end(); i++) {
                if (!BandExists(*i)) return false;
            }
            return true;            
        }

        //! Get raster band (0-based index)
        GeoRaster& operator[](unsigned int index) { 
            // Call const version
            return const_cast<GeoRaster&>(static_cast<const GeoImage&>(*this)[index]);
        }
        //! Get raster band, const version
        const GeoRaster& operator[](unsigned int index) const;
        //! Get raster band by description
        GeoRaster& operator[](std::string desc) {
            // Call const version
            return const_cast<GeoRaster&>(static_cast<const GeoImage&>(*this)[desc]);
        }
        //! Get raster band by description, const version
        const GeoRaster& operator[](std::string desc) const;

        //! Adds a band (as last band)
        GeoImage& AddBand(GeoRaster band);
        //! Remove band
        GeoImage& RemoveBand(unsigned int bandnum);
        //! Prune bands to only provided names
        GeoImage& PruneBands(std::vector<std::string> = {"red", "green", "blue"});

        //! \name Multiple band convenience functions
        //! Set gain for all bands
        void SetGain(float gain) { for (unsigned int i=0;i<_RasterBands.size();i++) _RasterBands[i].SetGain(gain); }
        //! Set gain for all bands
        void SetOffset(float offset) { for (unsigned int i=0;i<_RasterBands.size();i++) _RasterBands[i].SetOffset(offset); }
        //! Set NoData for all bands
        void SetNoData(double val) { for (unsigned int i=0;i<_RasterBands.size();i++) _RasterBands[i].SetNoData(val); }

        //! \name Processing functions
        template<class T> GeoImage& Process();
        //! Process band into new file (copy and apply processing functions)
        template<class T> GeoImage Process(std::string, DataType = 0);

        //! Adds a mask band (1 for valid) to every band in image
        GeoImage& AddMask(const GeoRaster& band) {
            for (unsigned int i=0;i<_RasterBands.size();i++) _RasterBands[i].AddMask(band);
            return *this;
        }
        //! Clear all masks
        void ClearMasks() { for (unsigned int i=0;i<_RasterBands.size();i++) _RasterBands[i].ClearMasks(); }
        //! Apply a mask directly to a file
        GeoImage& ApplyMask(CImg<uint8_t> mask, iRect chunk=iRect()) {
            for (unsigned int i=0;i<_RasterBands.size();i++) _RasterBands[i].ApplyMask(mask, chunk);
            return *this;
        }

        // hmm, what's this do?
        //const GeoImage& ComputeStats() const;

        //! Add overviews
        GeoResource& AddOverviews() {
            int panOverviewList[3] = { 2, 4, 8 };
            _GDALDataset->BuildOverviews( "NEAREST", 3, panOverviewList, 0, NULL, GDALDummyProgress, NULL );
            return *this; 
        }

        //! \name File I/O
        //! Read raw chunk, across all bands
        template<class T> CImg<T> ReadRaw(iRect chunk=iRect()) const {
            CImgList<T> images;
            typename std::vector< GeoRaster >::const_iterator iBand;
            for (iBand=_RasterBands.begin();iBand!=_RasterBands.end();iBand++) {
                images.insert( iBand->ReadRaw<T>(chunk) );
            }
            return images.get_append('v','p');
        }

        //! Read chunk, across all bands
        template<class T> CImg<T> Read(iRect chunk=iRect()) const {
            CImgList<T> images;
            typename std::vector< GeoRaster >::const_iterator iBand;
            for (iBand=_RasterBands.begin();iBand!=_RasterBands.end();iBand++) {
                images.insert( iBand->Read<T>(chunk) );
            }
            return images.get_append('v','p');
        }

        //! Write cube across all bands
        template<class T> GeoImage& Write(const CImg<T> img, iRect chunk=iRect()) {
            typename std::vector< GeoRaster >::iterator iBand;
            int i(0);
            for (iBand=_RasterBands.begin();iBand!=_RasterBands.end();iBand++) {
                iBand->Write(img.get_channel(i++), chunk);
            }
            return *this;
        }

        // Generate Masks: NoData, Data, Saturation
        //! NoData mask.  1's where it is nodata
        CImg<uint8_t> NoDataMask(std::vector<std::string> bands, iRect chunk=iRect()) const {
            std::vector<int> ibands = Descriptions2Indices(bands);
            CImg<unsigned char> mask;
            for (std::vector<int>::const_iterator i=ibands.begin(); i!=ibands.end(); i++) {
                if (i==ibands.begin()) 
                    mask = CImg<unsigned char>(_RasterBands[*i].NoDataMask(chunk));
                else
                    mask|=_RasterBands[*i].NoDataMask(chunk);
            }
            return mask;
        }

        // NoData mask (all bands)
        CImg<uint8_t> NoDataMask(iRect chunk=iRect()) const {
            return NoDataMask({}, chunk);
        }

        //! Data mask. 1's where valid data
        CImg<unsigned char> DataMask(std::vector<std::string> bands, iRect chunk=iRect()) const {
            return NoDataMask(bands, chunk)^=1;
        }

        CImg<unsigned char> DataMask(iRect chunk=iRect()) const {
            return DataMask({}, chunk);
        }

        //! Saturation mask (all bands).  1's where it is saturated
        CImg<unsigned char> SaturationMask(std::vector<std::string> bands, iRect chunk=iRect()) const {
            std::vector<int> ibands = Descriptions2Indices(bands);
            CImg<unsigned char> mask;
            for (std::vector<int>::const_iterator i=ibands.begin(); i!=ibands.end(); i++) {
                if (i==ibands.begin()) 
                    mask = CImg<unsigned char>(_RasterBands[*i].SaturationMask(chunk));
                else
                    mask|=_RasterBands[*i].SaturationMask(chunk);
            }
            return mask;
        }

        CImg<unsigned char> SaturationMask(iRect chunk=iRect()) const {
            return SaturationMask({}, chunk);
        }

        //! Whiteness (created from red, green, blue)
        CImg<float> Whiteness(iRect chunk=iRect()) const {
            if (!BandsExist({"red", "green", "blue"}))
                throw std::out_of_range("Need RGB bands to calculate whiteness");
            CImg<float> red = operator[]("red").ReadRaw<float>(chunk);
            CImg<float> green = operator[]("green").ReadRaw<float>(chunk);
            CImg<float> blue = operator[]("blue").ReadRaw<float>(chunk);
            CImg<float> white(red.width(),red.height());
            float mu;
            cimg_forXY(white,x,y) {
                mu = (red(x,y) + green(x,y) + blue(x,y))/3;
                white(x,y) = (abs(red(x,y)-mu) + abs(green(x,y)-mu) + abs(blue(x,y)-mu))/mu;
            }
            // Saturation?  If pixel saturated make Whiteness 0 ?
            return white;
        }

    protected:
        //! Vector of raster bands
        std::vector< GeoRaster > _RasterBands;
        //! Vector of raster band names
        std::vector< std::string > _BandNames;

        //! Loads Raster Bands of this GDALDataset into _RasterBands vector
        void LoadBands();

        // Convert vector of band descriptions to band indices
        std::vector<int> Descriptions2Indices(std::vector<std::string> bands) const;

    private:
        int BandIndex(std::string name) const {
            name = to_lower(name);
            std::string bname;
            for (unsigned int i=0; i<_BandNames.size(); i++) {
                bname = _BandNames[i];
                if (name == to_lower(bname)) return i;
            }
            throw std::out_of_range("No band " + name);
        }     

    }; // class GeoImage

    // GeoImage template function definitions
    //! Process in-place
    template<class T> GeoImage& GeoImage::Process() {
        // Create chunks
        ChunkSet chunks(XSize(), YSize());
        for (unsigned int i=0; i<NumBands(); i++) {
            for (unsigned int iChunk=0; iChunk<chunks.Size(); iChunk++) {
                (*this)[i].Write((*this)[i].Read<T>(chunks[iChunk]),chunks[iChunk]);
            }
            // clear functions after processing
            (*this)[i].ClearFunctions();
        }
        return *this;
    }

    // Copy input file into new output file
    template<class T> GeoImage GeoImage::Process(std::string filename, DataType dt) {
        // TODO: if not supplied base output datatype on units?
        if (dt.Int() == 0) dt = this->Type();
        GeoImage imgout(filename, *this, dt);
        for (unsigned int i=0; i<imgout.NumBands(); i++) {
            imgout[i].CopyMeta((*this)[i]);
            (*this)[i].Process<T>(imgout[i]);
        }
	imgout.SetBandNames(_BandNames);
        return imgout;
    }

} // namespace gip

#endif
