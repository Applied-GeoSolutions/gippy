/*##############################################################################
#    GIPPY: Geospatial Image Processing library for Python
#
#    Copyright (C) 2014 Matthew A Hanson
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see <http://www.gnu.org/licenses/>
##############################################################################*/

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <iostream>
#include <vector>
#include <gip/Utils.h>
//#include <gdal/ogr_srs_api.h>
#include <gdal/ogr_spatialref.h>

namespace gip {
    using std::vector;

    //! (2D) Point Class
    template<typename T=int> class Point {
    public:
        //! Default Constructor
        Point() : _x(0), _y(0) {}
        //! Constructor takes in point coordinate
        Point(T x, T y) : _x(x), _y(y) {}
        //! Constructor takes in comma delimited string
        //! Destructor
        ~Point() {}

        Point<T> operator-(const Point<T>& p) { return Point<T>(_x-p._x, _y-p._y); }
        Point<T> operator+(const Point<T>& p) { return Point<T>(_x+p._x, _y+p._y); }

        bool operator==(const Point<T>& p) const {
            if (_x == p._x && _y == p._y)
                return true;
            else return false;
        }
        bool operator!=(const Point<T>& p) const {
            return operator==(p) ? false : true;
        }

        //! Get x coordinate
        T x() const { return _x; }
        //! Get y coordinate
        T y() const { return _y; }
        // output operator
        friend std::ostream& operator<<(std::ostream& stream,const Point& p) {
            return stream << "(" << p._x << "," << p._y << ")";
        }
    protected:
        T _x;
        T _y;
    };

    //! (2D) Rect class
    template<typename T=int> class Rect {
    public:
        //! Default Constructor
        Rect() : _p0(0,0), _p1(-1,-1) {}
        //! Constructor takes in top left coordinate and width/height
        Rect(T x, T y, T width, T height) 
            : _p0(x,y), _p1(x+width-1,y+height-1) {
            // Validate, x0 and y0 should always be the  min values
            /*if (_width < 0) {
                _width = abs(m_width);
                _x = _x - _width + 1;
            }
            if (_height < 0) {
                _height = abs(_height);
                _y = _y - _height + 1;
            }*/
        }
        Rect(Point<T> p0, Point<T> p1)
            : _p0(p0), _p1(p1) {
        }

        //! Destructor
        ~Rect() {};

        Point<T> p0() const { return _p0; }
        Point<T> p1() const { return _p1; }

        //Point<T> min_corner() const { return Point<T>(_x,_y); }
        //Point<T> max_corner() const { return Point<T>(_x+_width,_y+height); }

        //! Validity of rect
        T valid() const { return (area() == 0) ? false : true; }
        //! Area of the Rect
        T area() const { return abs(width()*height()); }
        //! Width of Rect
        T width() const { return _p1.x()-_p0.x()+1; }
        //! Height of Rect
        T height() const { return _p1.y()-_p0.y()+1; }
        //! Left x coordinate
        T x0() const { return _p0.x(); }
        //! Top y coordinate
        T y0() const { return _p0.y(); }
        //! Right x coordinate
        T x1() const { return _p1.x(); }
        //! Bottom y coordinate
        T y1() const { return _p1.y(); }

        bool operator==(const Rect<T>& rect) const {
            if (_p0 == rect._p0 && _p1 == rect._p1)
                return true;
            return false;
        }
        bool operator!=(const Rect<T>& rect) const {
            return !operator==(rect);
        }

        //! Determines if ROI is valid (not valid if height or width is 0 or less)
        //bool valid() const { if (width() <= 0 || height() <=0) return false; else return true; }

        //! Shift Rect by (x,y,z)
        /*Rect& Shift(T x, T y) {
            _x += x;
            _y += y;
            return *this;
        }
        //! Shift Rect by (x,y) and return new Rect
        Rect get_Shift(T x, T y) const {
            return Rect(*this).Shift(x,y);
        }*/

        //! Transform between coordinate systems
        Rect& Transform(OGRSpatialReference src, OGRSpatialReference dst) {
            if (src.IsSame(&dst)) return *this;
            OGRCoordinateTransformation* trans = OGRCreateCoordinateTransformation(&src, &dst);
            double x, y;
            x = _p0.x();
            y = _p0.y();
            trans->Transform(1, &x, &y);
            _p0 = Point<T>(x, y);
            x = _p1.x();
            y = _p1.y();
            trans->Transform(1, &x, &y);
            _p1 = Point<T>(x, y);
            delete trans;
            return *this;
        }

        Rect& Pad(int pad) {
            _p0 = _p0 - Point<T>(pad,pad);
            _p1 = _p1 + Point<T>(pad,pad);
            return *this;
        }
        Rect get_Pad(int pad) const {
            return Rect<T>(*this).Pad(pad);
        }
        //! Intersects Rect with argument Rect
        Rect& Intersect(const Rect& rect) {
            _p0 = Point<T>( std::max(_p0.x(), rect.x0()), std::max(_p0.y(), rect.y0()) );
            _p1 = Point<T>( std::min(_p1.x(), rect.x1()), std::min(_p1.y(), rect.y1()) );
            return *this;
        }
        //! Returns intersection of two Rects
        Rect get_Intersect(const Rect& rect) const {
            return Rect<T>(*this).Intersect(rect);
        }
        // Calculates untion of Rect with argument Rect
        Rect& Union(const Rect& rect) {
            _p0 = Point<T>( std::min(_p0.x(), rect.x0()), std::min(_p0.y(), rect.y0()) );
            _p1 = Point<T>( std::max(_p1.x(), rect.x1()), std::max(_p1.y(), rect.y1()) );
            return *this;            
        }
        //! Returns outer bounding box of two rects
        Rect get_Union(const Rect& rect) const {
            return Rect<T>(*this).Union(rect);
        }
        friend std::ostream& operator<<(std::ostream& stream,const Rect& r) {
            return stream << r._p0 << "-" << r._p1;
        }
    private:
        // top-left
        Point<T> _p0;
        // bottom-right
        Point<T> _p1;
    };


    //! Collection of Rects representing a chunked up region (i.e. image)
    class ChunkSet {
    public:
        //! Default constructor
        ChunkSet()
            : _xsize(0), _ysize(0) {}

        //! Constructor taking in image size
        ChunkSet(unsigned int xsize, unsigned int ysize, unsigned int numchunks=0)
            : _xsize(xsize), _ysize(ysize) {
            ChunkUp(numchunks);
        }

        //! Copy constructor
        ChunkSet(const ChunkSet& chunks)
            : _xsize(chunks._xsize), _ysize(chunks._ysize) {
            ChunkUp(chunks.Size());
        }

        //! Assignment copy
        ChunkSet& operator=(const ChunkSet& chunks) {
            if (this == & chunks) return *this;
            _xsize = chunks._xsize;
            _ysize = chunks._ysize;
            ChunkUp(chunks.Size());
        }

        //! Get width of region
        unsigned int XSize() const { return _xsize; }

        //! Get height of region
        unsigned int YSize() const { return _ysize; }

        //! Determine if region is valid
        bool Valid() const {
            return (((_xsize * _ysize) == 0) || (Size() == 0)) ? false : true;
        }

        //! Get number of chunks
        unsigned int Size() const { return _Chunks.size(); }

        //! Get a chunk without padding
        Rect<int>& operator[](int index) { return _Chunks[index]; }
        //! Get a chunk, const version
        const Rect<int>& operator[](int index) const { return _Chunks[index]; }

        //! Get a chunk for reading with optional padding
        const Rect<int> Chunk(int index, int padding=0) const {
            if (padding == 0)
                return _Chunks[index];
            else {
                return _Chunks[index].get_Pad(padding).Intersect(Rect<int>(0,0,XSize(),YSize()));
            }
        }

        //! Get a rect that has been "de-padded" - returns rect suitable to use for cropping
        const Rect<int> DePad(int index, int padding=0) const {
            Rect<int> chunk = _Chunks[index];
            if (padding == 0) return chunk;
            Rect<int> pchunk = Chunk(index, padding);
            Point<int> p0(chunk.p0()-pchunk.p0());
            Point<int> p1 = p0 + Point<int>(chunk.width()-1,chunk.height()-1);
            return Rect<int>(p0, p1);
        }     

    private:
        //! Function to chunk up region
        vector< Rect<int> > ChunkUp(unsigned int numchunks=0) {
            unsigned int rows;

            if (numchunks == 0) {
                rows = floor( ( Options::ChunkSize() *1024*1024) / sizeof(double) / XSize() );
                rows = rows > YSize() ? YSize() : rows;
                numchunks = ceil( YSize()/(float)rows );
            } else {
                rows = int(YSize() / numchunks);
            }

            _Chunks.clear();
            Rect<int> chunk;
            /*if (Options::Verbose() > 3) {
                std::cout << Basename() << ": chunking into " << numchunks << " chunks (" 
                    << Options::ChunkSize() << " MB max each)" << std::endl;
            }*/
            for (unsigned int i=0; i<numchunks; i++) {
                chunk = Rect<int>(0, rows*i, XSize(), std::min(rows*(i+1),YSize())-(rows*i) );
                _Chunks.push_back(chunk);
                //if (Options::Verbose() > 3) std::cout << "  Chunk " << i << ": " << chunk << std::endl;
            }
            return _Chunks;            
        }

        //! Width (columns) of region
        unsigned int _xsize;
        //! Height (rows) of region
        unsigned int _ysize;

        //! Coordinates of the chunks
        vector< Rect<int> > _Chunks;
    };

} // namespace GIP


#endif
