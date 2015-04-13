/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Source File of the Transportation Mode Algebra

May, 2010 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations of
doing triangulation for polygon with and without holes.
The original implementation is from Atul Narkhede and Dinesh Manocha

[TOC]

1 Overview

2 Defines and includes

*/

#include "Triangulate.h"

using namespace Copied_from_Algebra_TransportationMode;
namespace Copied_from_Algebra_TransportationMode
{
//////////////////////////////////////////////////////////////////////////////
/////////// another implementation of triangulation /////////////////////////
/////////// 2011.7 from code project/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

Dimension GlobDim::mDim = DIM_NONE;

/*
initialize the polygon, input points

*/
  /*
  void HPolygon::Init( const char name[])
  {
      int     n, ni;
      double  x, y;

      FILE    *f = fopen( name, "rt");

      if (!f)
          THROW_FILE( "can not open file", name);

      printf( "Reading file: %s\n", name );

      fscanf( f, "%d", &n);
      mtabSize.resize( n);

      for ( int i=0; i<n; ++i){
          fscanf( f, "%d", &ni);
          mtabSize[i] = ni;

          for ( int j=0; j<ni; ++j)
          {
              fscanf( f, "%lg %lg", &x, &y);
              mtabPnt.insert( mtabPnt.end(), Vect2D( x, y) );
          }
      }

      fclose( f);
  }
  */
/*
new initialization function, read the input from vectors instead of files 

*/
void HPolygon::Init2(int ncontours, int cntr[], vector<double>& vertices_x,
                   vector<double>& vertices_y)
{

    int n = ncontours;
    mtabSize.resize(n);

    int point_id = 1;

    int count = 0;
    for ( int i = 0; i< n; i++){
        int ni = cntr[i];
        mtabSize[i] = ni;
//        cout<<"ni "<<ni<<endl;
/*        double first_x, first_y;
        int first_p_id;*/
        double first_x = 0.0;
        double first_y = 0.0;
        int first_p_id = 0;

        for ( int j = 0; j < ni; j++){
           double x = vertices_x[j + count];
           double y = vertices_y[j + count];
//           cout<<"x "<<x<<" y "<<y<<endl;
           mtabPnt.insert( mtabPnt.end(), Vect2D( x, y) );
           if(j == 0){
              first_x = x;
              first_y = y;
              first_p_id = point_id;
           }
           p_id_list.push_back(point_id);
           point_id++;
        }

        mtabSize[i]++;
        mtabPnt.insert( mtabPnt.end(), Vect2D( first_x, first_y) );
        count += ni;
        p_id_list.push_back(first_p_id);//no ++ !!!

//        count += ni;
    }

}

void HPolygon::Triangulate()
{
//    printf( "Triangulation started\n" );

    HGrid   grid;

    grid.Init( mtabPnt, mtabSize);
    grid.Generate();

    vector<HTri>::iterator  itri;
    IterGCell   itr;

    for ( itr = grid.CellBegin(); itr != grid.CellEnd(); ++itr)
    {
        itri = mtabCell.insert( mtabCell.end(), HTri() );
        (*itri).rIndex( 0) = (*(*itr)->Node( 0))->Index();
        (*itri).rIndex( 1) = (*(*itr)->Node( 1))->Index();
        (*itri).rIndex( 2) = (*(*itr)->Node( 2))->Index();
    }

}

/*
new form of triangulation

*/
  /*
  int HPolygon::Triangulation2(int ncontours, int cntr[], 
                              vector<double>& vertices_x,
                    vector<double>& vertices_y)
  {

  //try
  //  {
  //      HPolygon    poly;

  //      if ( argc != 3 ){
  //        printf("usage: hgrd [input file with points]
  //              [output tecplot file]\n");
  //        return 1;
  //      }
  //      poly.Init( argv[1]);
  //      poly.Triangulate();
  //      poly.WriteTEC( argv[2]);
  //  }
  //  catch ( Except *pe)
  //  {
  //      ASSERT( pe);
  //      TRACE_EXCEPTION( *pe);
  //      TRACE_TO_STDERR( *pe);
  //      delete pe;
  //  }

  //     for(unsigned int i = 0;i < vertices_x.size();i++){
  //       cout<<vertices_x[i]<<" "<<vertices_y[i]<<endl;
  //     }

  //    cout<<mtabPnt.max_size()<<endl; ////////268,435,455
      Init2(ncontours, cntr, vertices_x,vertices_y);
      Triangulate();
      int triangle_no = OutPut();

      return triangle_no;
  }
  */
  /*
  output the result into files 

  */
  /*
  void HPolygon::WriteTEC( const char name[])
  {
      printf( "Writing TECPLOT file: %s\n", name );

      FILE *f = fopen( name, "wt");

      fprintf( f, "TITLE = \"polygon\"\n");
      fprintf( f, "VARIABLES = \"X\", \"Y\"\n");
      fprintf( f, "ZONE T=\"TRIANGLES\", ");
      fprintf( f, "N=%2ld, ", (long int)mtabPnt.size() );
      fprintf( f, "E=%2ld, F=FEPOINT, ET=TRIANGLE C=BLACK\n ", 
              (long int)mtabCell.size() );

      size_t      i;
      for ( i=0; i<mtabPnt.size(); ++i)
          fprintf( f, "%lg %lg\n", mtabPnt[i].X(), mtabPnt[i].Y() );


      for ( i=0; i<mtabCell.size(); ++i)
          fprintf( f, "%d %d %d\n", 1+mtabCell[i].Index(0), 
                  1+mtabCell[i].Index(1), 1+mtabCell[i].Index(2) );

      fclose( f);
  }
  */
  /*
  int HPolygon::OutPut()
  {
      
  //     for (int i = 0; i < mtabPnt.size(); ++i)
  //         printf( "%lg %lg\n", mtabPnt[i].X(), mtabPnt[i].Y() );
  // 
  // 
  //     for (int i = 0; i< mtabCell.size(); ++i)
  //         printf( "%d %d %d\n", 1+mtabCell[i].Index(0), 
  //                  1+mtabCell[i].Index(1), 1+mtabCell[i].Index(2) );

  //    cout<<mtabPnt.size()<<" "<<mtabCell.size()<<endl;

      return mtabCell.size();
  }

  */

MGFloat Angle( const Vect2D &nd, const Vect2D &nd1, const Vect2D &nd2)
{
    static Vect2D   v1, v2;
    static MGFloat  dvect, dsin, dcos;

    //v1 = nd1 - nd;
    //v2 = nd2 - nd;
    v1 = (nd1 - nd).module();
    v2 = (nd2 - nd).module();

    dsin = v1.X()*v2.Y() - v2.X()*v1.Y();
    dcos = v1.X()*v2.X() + v1.Y()*v2.Y();
    if ( fabs( dsin) < ZERO && fabs( dcos) < ZERO)
        return M_PI_2;
    dvect = atan2( dsin, dcos);
    return dvect;
}

/*
check vector crossing 

*/
bool CheckCrossing( const Vect2D& v1, const Vect2D& v2, const Vect2D& v3, 
                    const Vect2D& v4)
{
    Vect2D  vv, vv1, vv2;
    MGFloat t1, t2, t;
    MGFloat h1, h2;

    
    vv = v2 - v1;
    vv1 = v3 - v1;
    vv2 = v4 - v1;
    
    ASSERT( fabs( vv.module() ) > ZERO );

    t1 = vv * vv1 / vv.module();
    t2 = vv * vv2 / vv.module();
    
    h1 = (vv.X()*vv1.Y() - vv.Y()*vv1.X()) / vv.module();
    h2 = (vv.X()*vv2.Y() - vv.Y()*vv2.X()) / vv.module();

    if ( fabs( h2 - h1) < ZERO)
        return false;

    t = t1 - (t2 - t1)/(h2 - h1) * h1;
    
    if ( t > 0.0 && t < vv.module() && h1 * h2 < ZERO )
        return true;
    else
        return false;
}


bool AreNeigbours( const IterGCell& ic1, const IterGCell& ic2)
{
    char    sbuf[256];
    bool    b1, b2;

//  TRACE2( "itrcl = %d %d", ic1, *ic1);
//  (*ic1)->DumpTri();
//  TRACE2( "itrcl = %d %d", ic2, *ic2);
//  (*ic2)->DumpTri();

    b1 = b2 = false;
    for ( int i=0; i<NUM_TRI; ++i)
    {
//      if ( (*ic1)->Cell(i) != NULL)
            if ( (*ic1)->Cell(i) == ic2 )
                b1 = true;

//      if ( (*ic2)->Cell(i) != NULL)
            if ( (*ic2)->Cell(i) == ic1 )
                b2 = true;
    }

    sprintf( sbuf, " b1 = %d b2 = %d", static_cast<int>(b1), 
             static_cast<int>(b2) );
    TM_TRACE( sbuf);
    if ( !b1 || !b2)
        THROW_INTERNAL( "Not neighbours found");

    return true;
}


/* 
class HGrdTri 
main class for triangulation, grid 

*/

HGrdTri::HGrdTri() : mbIsOutside( false)    
{
    for ( MGInt i=0; i<NUM_TRI; i++)
    {
//        mlstNod[i] = NULL;
//        mlstCell[i] = NULL;

          mlstNod[i] = (IterGPnt)NULL;
          mlstCell[i] = (IterGCell)NULL;
    }
}

/*
check whether two grids are visible 

*/
bool HGrdTri::IsVisible( const IterGCell& icl, const Vect2D& vct)
{
    static Vect2D   vfac, v1, v2;
    static MGFloat  d1, d2;
//  static char     sbuf[1024];

    if ( Cell(0) == icl )
    {
        v1 = *( (*Node(0)) ) - *( (*Node(2)) );
        v2 = *( (*Node(1)) ) - *( (*Node(2)) );
        vfac = vct - *( (*Node(2)) );
    }
    else if ( Cell(1) == icl )
    {
        v1 = *( (*Node(1)) ) - *( (*Node(0)) );
        v2 = *( (*Node(2)) ) - *( (*Node(0)) );
        vfac = vct - *( (*Node(0)) );
    }
    else if ( Cell(2) == icl )
    {
        v1 = *( (*Node(2)) ) - *( (*Node(1)) );
        v2 = *( (*Node(0)) ) - *( (*Node(1)) );
        vfac = vct - *( (*Node(1)) );
    }
    else
    {
        ASSERT( 0);
    }
    d1 = v1.X()*vfac.Y() - v1.Y()*vfac.X();
    d2 = v2.X()*vfac.Y() - v2.Y()*vfac.X();

    d1 /= vfac.module();
    d2 /= vfac.module();
    
    if ( (d2 > ZERO && d1 < -ZERO) )
        return false;
    else
        return true;
}

bool HGrdTri::IsVisibleDump( const IterGCell& icl, const Vect2D& vct)
{
//  static Vect2D   vfac, v1, v2;
//  static MGFloat  d1, d2;
//  static char     sbuf[1024];
    Vect2D  vfac, v1, v2;
    MGFloat d1, d2;

    THROW_INTERNAL("Should not be used !!!");

    if ( Cell(0) == icl )
    {
        v1 = *( (*Node(0)) ) - *( (*Node(2)) );
        v2 = *( (*Node(1)) ) - *( (*Node(2)) );
        vfac = vct - *( (*Node(2)) );
    }
    else if ( Cell(1) == icl )
    {
        v1 = *( (*Node(1)) ) - *( (*Node(0)) );
        v2 = *( (*Node(2)) ) - *( (*Node(0)) );
        vfac = vct - *( (*Node(0)) );
    }
    else if ( Cell(2) == icl )
    {
        v1 = *( (*Node(2)) ) - *( (*Node(1)) );
        v2 = *( (*Node(0)) ) - *( (*Node(1)) );
        vfac = vct - *( (*Node(1)) );
    }
    else
    {
        ASSERT( 0);
    }
    d1 = v1.X()*vfac.Y() - v1.Y()*vfac.X();
    d2 = v2.X()*vfac.Y() - v2.Y()*vfac.X();
    
    if ( d1 * d2 > ZERO )
    {
        TM_TRACE2( "v1 = (%lg %lg)", v1.X(), v1.Y() );
        TM_TRACE2( "v2 = (%lg %lg)", v2.X(), v2.Y() );
        TM_TRACE2( "vf = (%lg %lg)", vfac.X(), vfac.Y() );
        TM_TRACE2( "d1 = %lg d1 = %lg", d1, d2 );

        return false;
    }
    else
        return true;
}

/*
for a grid, set its neighbors 

*/

void HGrdTri::SetNeighbour( const IterGCell& itrcl)
{
    static HGrdTri  *ptr;
    
//    ASSERT( itrcl != NULL);
    ASSERT( itrcl != (IterGCell)NULL);
    ptr = (*itrcl);
    ASSERT( ptr);
    
    if ( ( ptr->Node(1) == Node(0) && ptr->Node(0) == Node(1) ) ||
         ( ptr->Node(0) == Node(0) && ptr->Node(2) == Node(1) ) ||
         ( ptr->Node(2) == Node(0) && ptr->Node(1) == Node(1) ) )
    {
        rCell(0) = itrcl;
    }
    else
    if ( ( ptr->Node(1) == Node(1) && ptr->Node(0) == Node(2) ) ||
         ( ptr->Node(0) == Node(1) && ptr->Node(2) == Node(2) ) ||
         ( ptr->Node(2) == Node(1) && ptr->Node(1) == Node(2) ) )
    {
        rCell(1) = itrcl;
    }
    else
    if ( ( ptr->Node(1) == Node(2) && ptr->Node(0) == Node(0) ) ||
         ( ptr->Node(0) == Node(2) && ptr->Node(2) == Node(0) ) ||
         ( ptr->Node(2) == Node(2) && ptr->Node(1) == Node(0) ) )
    {
        rCell(2) = itrcl;
    }
}


void HGrdTri::NullifyThis( HGrdTri *pcl)
{
    for ( MGInt i=0; i<NUM_TRI; i++)
    {
//         if ( mlstCell[i] != NULL)
//             if ( (*mlstCell[i]) == pcl)
//                 mlstCell[i] = NULL;

        if ( mlstCell[i] != (IterGCell)NULL)
            if ( (*mlstCell[i]) == pcl)
                mlstCell[i] = (IterGCell)NULL;

    }
}


void HGrdTri::NullifyThis( HFroSeg *pseg)
{
    if ( ( pseg->PntLf() == Node(0) && pseg->PntLf() == Node(1) ) ||
         ( pseg->PntRt() == Node(0) && pseg->PntRt() == Node(1) ) )
    {
//        rCell(0) = NULL;
        rCell(0) = (IterGCell)NULL;
    }
    else
    if ( ( pseg->PntLf() == Node(1) && pseg->PntLf() == Node(2) ) ||
         ( pseg->PntRt() == Node(1) && pseg->PntRt() == Node(2) ) )
    {
//        rCell(1) = NULL;
        rCell(1) = (IterGCell)NULL;
    }
    else
    if ( ( pseg->PntLf() == Node(2) && pseg->PntLf() == Node(0) ) ||
         ( pseg->PntRt() == Node(2) && pseg->PntRt() == Node(0) ) )
    {
//        rCell(2) = NULL;
        rCell(2) = (IterGCell)NULL;
    }
}


void HGrdTri::InvalidateNeighb()
{
    for ( MGInt i=0; i<NUM_TRI; i++)
    {
//        if ( mlstCell[i] != NULL)
        if ( mlstCell[i] != (IterGCell)NULL)
            (*mlstCell[i])->NullifyThis( this);
    }
}



bool HGrdTri::IsInside( const Vect2D& vct)
{
// ::TODO:: new and faster algorithm should be introduced
    MGFloat alf;

    alf  = ::Angle( vct, *(*Node(0)), *(*Node(1)) );
    alf += ::Angle( vct, *(*Node(1)), *(*Node(2)) );
    alf += ::Angle( vct, *(*Node(2)), *(*Node(0)) );

    if ( fabs(alf) < M_PI )
        return false;
    else 
        return true;
}

/*
set the center point of a triangle

*/

bool HGrdTri::SetCircCenter()
{
    static MGFloat  x1, y1, x2, y2, x3, y3;
    static MGFloat  xr, yr, d;

    x1 = (*Node(0))->X();
    y1 = (*Node(0))->Y();
    x2 = (*Node(1))->X();
    y2 = (*Node(1))->Y();
    x3 = (*Node(2))->X();
    y3 = (*Node(2))->Y();

    d = y3*(x2 - x1) + y2*(x1 - x3) + y1*(x3 - x2);
    if ( fabs( d) < ZERO)
    {
      //guenther:
        xr = 0.5*(min(x3,min(x1,x2)) + max(x3,min(x1,x2)));
        yr = 0.5*(min(y3,min(y1,y2)) + max(y3,min(y1,y2)));
        mCircCenter =  Vect2D( xr, yr);
        return false;

        DumpTri();
//        TM_TRACE1( "d = %lg", d);
        printf("%.6f\n", d);
        printf("(%.7f %.7f)\n", x1, y1);
        printf("(%.7f %.7f)\n", x2, y2);
        printf("(%.7f %.7f)\n", x3, y3);

//        cout<<"SetCircCenter() "<<"error "<<endl;
        return true;
        // THROW_INTERNAL( "Problem inside SetCircCenter() !!!");
    }

    xr = x1*x1*(y3-y2) + x2*x2*(y1-y3) + x3*x3*(y2-y1) + 
         y1*y1*(y3-y2) + y2*y2*(y1-y3) + y3*y3*(y2-y1);
    xr *= -0.5/d;

    yr = x1*x1*(x3-x2) + x2*x2*(x1-x3) + x3*x3*(x2-x1) + 
         y1*y1*(x3-x2) + y2*y2*(x1-x3) + y3*y3*(x2-x1);
    yr *= 0.5/d;

    mCircCenter =  Vect2D( xr, yr);

    return false;
}




Vect2D HGrdTri::Center()
{
    return ( *(*Node(0)) + *(*Node(1)) + *(*Node(2)) )/3.0;
}



IterGCell HGrdTri::NextCell( const Vect2D& vct)
{
    static Vect2D   v1, v2;
    
//    if ( Cell(0) != NULL)
    if ( Cell(0) != (IterGCell)NULL){
        v1 = *(*Node(1)) - *(*Node(0));
        v1 = Vect2D( -v1.Y(), v1.X() );
        v2 = ( *(*Node(1)) + *(*Node(0)) )/2.0;
        v2 = vct - v2;
        if ( v1 * v2 < 0.0 )
            return Cell(0);
    }

//    if ( Cell(1) != NULL)
    if ( Cell(1) !=  (IterGCell)NULL){
        v1 = *(*Node(2)) - *(*Node(1));
        v1 = Vect2D( -v1.Y(), v1.X() );
        v2 = ( *(*Node(2)) + *(*Node(1)) )/2.0;
        v2 = vct - v2;
        if ( v1 * v2 < 0.0 )
            return Cell(1);
    }

//    if ( Cell(2) != NULL)
    if ( Cell(2) != (IterGCell)NULL){
        v1 = *(*Node(0)) - *(*Node(2));
        v1 = Vect2D( -v1.Y(), v1.X() );
        v2 = ( *(*Node(0)) + *(*Node(2)) )/2.0;
        v2 = vct - v2;
        if ( v1 * v2 < 0.0 )
            return Cell(2);
    }


    return (IterGCell)NULL;
}

/*
iterate to next cell to access  

*/
IterGCell  HGrdTri::NextCell( HFroSeg *pseg, const IterGCell& iclprv)
{
    IterGCell   itrnb, itr1, itr2;
    Vect2D      v1, v2, v3, v4;

    v1 = *(*(pseg->PntLf()));
    v2 = *(*(pseg->PntRt()));

//    if ( iclprv == NULL)
    if ( iclprv == (IterGCell)NULL){

        if ( Node(0) == pseg->PntLf() )
        {
            v3 = *(*Node(1));
            v4 = *(*Node(2));
            itrnb = Cell(1);
            if ( Node(1) == pseg->PntRt() || Node(2) == pseg->PntRt() )
//                return NULL;
            return (IterGCell)NULL;
        }
        else if ( Node(1) == pseg->PntLf() )
        {
            v3 = *(*Node(2));
            v4 = *(*Node(0));
            itrnb = Cell(2);
            if ( Node(2) == pseg->PntRt() || Node(0) == pseg->PntRt() )
//                return NULL;
                return (IterGCell)NULL;
        }
        else if ( Node(2) == pseg->PntLf() )
        {
            v3 = *(*Node(0));
            v4 = *(*Node(1));
            itrnb = Cell(0);
            if ( Node(0) == pseg->PntRt() || Node(1) == pseg->PntRt() )
//                return NULL;
            return (IterGCell)NULL;
        }
        else
        {
            THROW_INTERNAL("NextCell - seg: node not found");
        }

        if ( ::CheckCrossing( v1, v2, v3, v4 ) == true)
        {
            return itrnb;
        }
        else
//            return NULL;
          return (IterGCell)NULL;
    }
    else
    {
        int k;
        for ( int i=0; i<NUM_TRI; ++i)
        {
            if ( iclprv != Cell(i) )
            {
                if ( i == NUM_TRI-1)
                    k = 0;
                else
                    k = i+1;

                v3 = *(*Node(i));
                v4 = *(*Node(k));

                if ( ::CheckCrossing( v1, v2, v3, v4 ) == true)
                    return Cell(i);
            }
        }

//        return NULL;
          return (IterGCell)NULL;
    }
}


bool HGrdTri::IsInsideCirc( const Vect2D& vct)
{
    static MGFloat  R2, r2;
    static Vect2D   vtmp, v0;

    v0 = CircCenter();
    vtmp = v0 - *( (*Node(0)) );
    R2 = vtmp * vtmp;

    vtmp = v0 - vct;
    r2 = vtmp * vtmp;

    if ( r2 < R2) 
        return true;
    
    return false;
}


/* 
class HFront 

*/

HFront::~HFront()
{
    iterator    i;
    for ( i= begin(); i != end(); i++)
        if ( *i != NULL) delete (*i);
}


MGFloat HFront::Angle( const Vect2D& vct)
{
    iterator    itr;
    MGFloat     alf;
    IterGPnt    ipnt1, ipnt2;
    
    alf = 0.0;
    for ( itr = begin(); itr != end(); itr++)
    {
        ipnt1 = (*itr)->PntLf();
        ipnt2 = (*itr)->PntRt();
        alf += ::Angle( vct, *(*ipnt1), *(*ipnt2) );
    }
    
    return alf;
}





/* 
class HGrid 

*/


HGrid::~HGrid()         
{
    CollGPnt::iterator  itrpnt;
    CollGCell::iterator itrcell;
    
    for ( itrpnt = mcolPnt.begin(); itrpnt != mcolPnt.end(); itrpnt++)
        if ( (*itrpnt) != NULL) delete (*itrpnt);

    for ( itrcell = mcolCell.begin(); itrcell != mcolCell.end(); itrcell++)
        if ( (*itrcell) != NULL) delete (*itrcell);
}


void HGrid::Init( const vector<Vect2D>& tabp, const vector<MGInt>& tabn )
{
    MGInt   i, j, nprev;
    IterFro ifro;

    HGrdPnt     *ppnt;
    IterGPnt    ip0, ipp, ipa;
    HFroSeg     *pfro;
    Vect2D      v0, v1, v2;

    map< Vect2D, IterGPnt>              mapNod;
    map< Vect2D, IterGPnt>::iterator    imap;

//  char    sbuf[512];
    double  d;

    nprev = 0;
    for ( i=0; i<(MGInt)tabn.size(); ++i)
    {
        v1 = tabp[nprev];
        v2 = tabp[nprev+tabn[i]-1];

        d = (v2-v1).module();

        if ( (v2-v1).module() < ZERO)
        {
            ifro = mcolFro.insert( mcolFro.end(), HFront() );

            imap = mapNod.find( tabp[nprev]);
            if ( imap != mapNod.end() )
            {
                ip0 = ipp = ipa = (*imap).second;
            }
            else
            {
                ppnt = MGNEW HGrdPnt( tabp[nprev]);
                ppnt->rIndex() = nprev;
                ip0 = ipp = ipa = InsertPoint( ppnt);
                mapNod.insert( map< Vect2D, IterGPnt>::value_type( *ppnt, ipa));
            }

            v0 = *(*ip0); 
        

            for ( j=1; j<tabn[i]; ++j)
            {
                v1 = *(*ipp);
                v2 = tabp[nprev+j];

                if ( (v2 - v1).module() > ZERO)
                {
                  if ( j != tabn[i]-1 || (tabp[nprev+j] - v0 ).module() > ZERO)
                    {
                        imap = mapNod.find( tabp[nprev+j]);
                        if ( imap != mapNod.end() )
                        {
                            ipa = (*imap).second;
                            ppnt = *ipa;
                        }
                        else
                        {
                            ppnt = MGNEW HGrdPnt( tabp[nprev+j]);
                            ppnt->rIndex() = nprev+j;
                            ipa = InsertPoint( ppnt);
                            mapNod.insert( map< Vect2D, 
                                           IterGPnt>::value_type( *ppnt, ipa) );
                        }


                        pfro = MGNEW HFroSeg( ipp, ipa);
                        (*ifro).insert( (*ifro).end(), pfro);
                        ipp = ipa;
                    }
                }
            }

            v1 = *(*ipp);
            v2 = *(*ip0);

            if ( (v2 - v1).module() > ZERO)
            {
                pfro = MGNEW HFroSeg( ipp, ip0);
                (*ifro).insert( (*ifro).end(), pfro);
            }
        }

        nprev += tabn[i];
    }



    IterFro     itrfro;
    IterFSeg    itrsg;
    IterGPnt    ip1, ip2;

    for ( itrfro = mcolFro.begin(); itrfro != mcolFro.end(); ++itrfro)
    {
//      TRACE1( "Front size = %d\n", (*itrfro).size() );
        itrsg = (*itrfro).begin();
        ip0 = (*itrsg)->PntLf();
        ip1 = (*itrsg)->PntRt();

        for ( ++itrsg; itrsg != (*itrfro).end(); ++itrsg)
        {
            if ( (*itrsg)->PntLf() != ip1 )
            {
                TM_TRACE( "Front not consistent !!!\n");
            }
            ip1 = (*itrsg)->PntRt();

            v1 = *(*(*itrsg)->PntLf());
            v2 = *(*(*itrsg)->PntRt());

            if ( (v2 - v1).module() < ZERO)
                TM_TRACE1( "seg length = %24.16lg\n", (v2 - v1).module() );
        }

        if ( ip0 != ip1 )
        {
            TM_TRACE( "Front not consistent (closure problem) !!!\n");
        }
    }

//  ASSERT(0);

#ifdef _DEBUG

    FILE    *f = fopen( "front.plt", "wt");

    int isize = 0;
    for ( isize = 0, itrfro = mcolFro.begin(); itrfro != mcolFro.end();
          ++itrfro, ++isize)
    {
        fprintf( f, "VARIABLES = \"X\", \"Y\"\n" );
        fprintf( f, "ZONE I=%d F=POINT\n", (*itrfro).size()+1);

        for ( itrsg = (*itrfro).begin(); itrsg != (*itrfro).end(); ++itrsg)
        {
            v1 = *(*(*itrsg)->PntLf());
            v2 = *(*(*itrsg)->PntRt());
            fprintf( f, "%lg %lg\n", v1.X(), v1.Y() );
        }
        fprintf( f, "%lg %lg\n", v2.X(), v2.Y() );
    }

    fclose( f);

#endif // _DEBUG

}



HFroSeg* HGrid::NewFace( MGInt i, IterGCell icl)
{
    HFroSeg*    psg;
    
    THROW_ALLOC( psg = MGNEW HFroSeg() );
    switch ( i)
    {
        case  0:
            psg->rPntLf() = (*icl)->Node(0);
            psg->rPntRt() = (*icl)->Node(1);
//            if ( icl != NULL ){
            if ( icl != (IterGCell)NULL ){
                psg->rCellUp() = icl;
                psg->rCellLo() = (*icl)->Cell(0);
            }
            break;

        case  1:
            psg->rPntLf() = (*icl)->Node(1);
            psg->rPntRt() = (*icl)->Node(2);
//            if ( icl != NULL)
            if ( icl != (IterGCell)NULL){
                psg->rCellUp() = icl;
                psg->rCellLo() = (*icl)->Cell(1);
            }
            break;
            
        case  2:
            psg->rPntLf() = (*icl)->Node(2);
            psg->rPntRt() = (*icl)->Node(0);
//            if ( icl != NULL)
            if ( icl != (IterGCell)NULL){
                psg->rCellUp() = icl;
                psg->rCellLo() = (*icl)->Cell(2);
            }
            break;

        default:
            if ( psg) delete psg;
            ASSERT( 0);
            return NULL;
    };
    return psg;
}

/*
check neighbor 

*/
bool HGrid::CheckNeighb( IterGCell icl, CollFSeg& lstsg, 
                         const Vect2D& vct, const IterGCell& ipvcl)
{
    HGrdTri *pthis;

    pthis = (HGrdTri*)( (*icl) );

    if ( pthis->IsInsideCirc( vct ) == true )
    {
        HGrdTri     *ptri;
        HFroSeg     *pseg;
        IterGCell   itri;
        IterGPnt    ipnt;
        bool        bVis;

        for ( int i=0; i<NUM_TRI; i++)
        {       
//            if ( (*icl)->Cell(i) != ipvcl || ipvcl == NULL)
            if ( (*icl)->Cell(i) != ipvcl || ipvcl == (IterGCell)NULL){
                pseg = NewFace( i, icl);    // this allocate memory for pseg !!!

                itri = (*icl)->Cell(i);
                bVis = false;
//                if ( itri != NULL )
                if ( itri != (IterGCell)NULL )
                    if ( (*itri)->IsOutside() == true)
//                        itri = NULL;
                      itri = (IterGCell)NULL;

//                if ( itri != NULL)
                if ( itri != (IterGCell)NULL){
                    ptri = (HGrdTri*)( (*itri) );
                    ASSERT( ptri);
                    bVis = ptri->IsVisible( icl, vct);
                }

//                if ( itri != NULL && bVis)
                if ( itri != (IterGCell)NULL && bVis){
                    if ( CheckNeighb( itri, lstsg, vct, icl ) == true )
                    {
                        delete pseg;
                    }
                    else
                    {
                        lstsg.insert( lstsg.end(), pseg );
 //                       pseg->rCellUp() = NULL;
                        pseg->rCellUp() = (IterGCell)NULL;
                    }
                }
                else
                {
                    lstsg.insert( lstsg.end(), pseg );
//                    pseg->rCellUp() = NULL;
                    pseg->rCellUp() = (IterGCell)NULL;
                }
            }
        }
                    
        ptri = (HGrdTri*)( (*icl) );
        ptri->InvalidateNeighb();

        mcolCell.erase( icl);
        delete ptri;
        return true;
    }

    return false;
}


MGInt HGrid::InsertPointIntoMesh( IterGPnt pntitr)
{
    Vect2D      vct;
    IterGCell   itrcl, itrcl2, itrcl0, itrclout;
    HGrdTri     *ptri;
    CollFSeg    *plstsg;
    IterFSeg    itrsg, itrsg2;
    HFroSeg     *pseg;

    
    static int  num = 0;
    ++num;

    vct = *(*pntitr);
    
// sprintf( sbuf, "POINT No = %d; x=%14.8lg, y=%14.8lg", num, vct.X(), vct.Y());
//  TRACE1( "%s", sbuf);
    
    itrcl = mcolCell.begin();

    do
    {
        itrcl0 = (*itrcl)->NextCell( vct);
//        if ( itrcl0 == NULL)
        if ( itrcl0 == (IterGCell)NULL)
            break;
        itrcl = itrcl0;
    }
    while ( true);
    
    
    THROW_ALLOC( plstsg = MGNEW CollFSeg );
    
   // next function creates segments bounding Delaunay cavity (stored in plstsg)
    // removes cavity triangles from mcolCell;
    // ALL ITERATORS TO THOSE CELLS ARE THEN INVALID !!!
    // iterators to those cells are set to NULL
    
//    CheckNeighb( itrcl, *plstsg, vct, NULL);
    CheckNeighb( itrcl, *plstsg, vct, (IterGCell)NULL);

    // sorting segments stored in plstsg 
    itrsg = plstsg->end();
    itrsg--;
    do
    {
        for ( itrsg2 = plstsg->begin(); itrsg2 != plstsg->end(); itrsg2++)
        {
            if ( (*itrsg)->PntLf() == (*itrsg2)->PntRt() )
            {
                pseg = (*itrsg2);
                plstsg->erase( itrsg2);
                itrsg = plstsg->insert( itrsg, pseg );
                break;
            }
        }
    }
    while ( itrsg != plstsg->begin() );

 // creating new triangles and connections between triangles in Delaunay cavity
//    itrcl0 = itrcl2 = NULL;
    itrcl0 = itrcl2 = (IterGCell)NULL;
    for ( itrsg = plstsg->begin(); itrsg != plstsg->end(); itrsg++)
    {
        THROW_ALLOC( ptri = MGNEW HGrdTri );
        
        itrclout = (*itrsg)->CellLo();
        ptri->rNode(0) = (*itrsg)->PntLf();
        ptri->rNode(1) = (*itrsg)->PntRt();
        ptri->rNode(2) = pntitr;
        ptri->rCell(0) = itrclout;
            
        if ( ptri->SetCircCenter() )
        {
/*deleted by Guenther
            FILE *f = fopen( "cavity.plt", "wt");
            ExportTECTmp( f);
            fclose( f);
            TM_TRACE1( "num = %d", num);
            TM_TRACE2( "new point = %lg %lg", vct.X(), vct.Y() );
            TM_TRACE1( "no of segs bounding cavity = %d", plstsg->size() );

            FILE *ff = fopen( "cavity_front.plt", "wt");
            fprintf( ff, "VARIABLES = \"X\", \"Y\"\n");
            fprintf( ff, "ZONE I=%d F=POINT\n", (int) (plstsg->size()+1) );

            fprintf( ff, "%lg %lg\n", (*(*plstsg->begin())->PntLf())->X(), 
                     (*(*plstsg->begin())->PntLf())->Y() );
            for ( itrsg = plstsg->begin(); itrsg != plstsg->end(); itrsg++)
                fprintf( ff, "%lg %lg\n", (*(*itrsg)->PntRt())->X(), 
                         (*(*itrsg)->PntRt())->Y() );

            fclose( ff);

            THROW_INTERNAL("Flat triangle !!!"); */
        }

        itrcl = InsertCell( ptri);

//        if ( itrclout != NULL)
        if ( itrclout != (IterGCell)NULL)
            (*itrclout)->SetNeighbour( itrcl);

//        if ( itrcl0 == NULL)
        if ( itrcl0 == (IterGCell)NULL)
            itrcl0 = itrcl;

//        if ( itrcl2 != NULL)
        if ( itrcl2 != (IterGCell)NULL){
            (*itrcl)->rCell(2)  = itrcl2;
            (*itrcl2)->rCell(1) = itrcl;
        }
        itrcl2 = itrcl;
    }
//    if ( itrcl2 != NULL && itrcl0 != NULL)
    if ( itrcl2 != (IterGCell)NULL && itrcl0 != (IterGCell)NULL)
    {
        (*itrcl0)->rCell(2) = itrcl2;
        (*itrcl2)->rCell(1) = itrcl0;
    }   

    // removing all segments stored in plstsg
    for ( itrsg = plstsg->begin(); itrsg != plstsg->end(); itrsg++)
        if ( (*itrsg) != NULL ) delete (*itrsg);

    if ( plstsg) delete plstsg;


    return num;
}





bool HGrid::PointExists( const Vect2D& vct)
{
//  Leaf<IterFacGPnt>   *pndlf;
//  IterFacGPnt         ipn;
//
//  pndlf = mPntQTree.ClosestItem( vct );
//  if ( pndlf != NULL)
//  {   
//      ipn = pndlf->Data();
//
//      if ( fabs( (*ipn)->X() - vct.X() ) < ZERO &&
//           fabs( (*ipn)->Y() - vct.Y() ) < ZERO   )
//      {
//           return true;
//      }
//  }
//
    return false;
}



// creates basic trinagulation (two triangles) and inserts all boundary points
void HGrid::InitTriangles()
{
    CollGPnt::iterator  itr;
    Vect2D      vmin, vmax, vct;
    bool        bFirst = true;
    HGrdPnt     *pnd1, *pnd2, *pnd3, *pnd4;
    HGrdTri     *ptri1, *ptri2;
    IterGPnt    ind1, ind2, ind3, ind4;
    IterGCell   itri1, itri2;

//    ind1 = ind2 = ind3 = ind4 = NULL;
    ind1 = ind2 = ind3 = ind4 = (IterGPnt)NULL;
//    itri1 = itri2 = NULL;
    itri1 = itri2 = (IterGCell)NULL;


    // finding limits
    for ( itr = mcolPnt.begin(); itr != mcolPnt.end(); itr++)
    {
        vct = *(*itr);
        if ( bFirst)
        {
            vmin = vmax = vct;
            bFirst = false;
        }
        else
        {
            if ( vct.X() > vmax.X() ) vmax.rX() = vct.X();
            if ( vct.Y() > vmax.Y() ) vmax.rY() = vct.Y();
            if ( vct.X() < vmin.X() ) vmin.rX() = vct.X();
            if ( vct.Y() < vmin.Y() ) vmin.rY() = vct.Y();
        }
    }
    
    vct = (vmax - vmin)/1.5;
    vmax += vct;
    vmin -= vct;
    
    mBox = HRect( vmin.X(), vmin.Y(), vmax.X(), vmax.Y() ); 
    
    // creating starting triangulation containing two cells and four points
    THROW_ALLOC( pnd1 = MGNEW HGrdPnt( vmin) );
    THROW_ALLOC( pnd2 = MGNEW HGrdPnt( vmax.X(), vmin.Y()) );
    THROW_ALLOC( pnd3 = MGNEW HGrdPnt( vmax) );
    THROW_ALLOC( pnd4 = MGNEW HGrdPnt( vmin.X(), vmax.Y()) );
    
    THROW_ALLOC( ptri1 = MGNEW HGrdTri() );
    THROW_ALLOC( ptri2 = MGNEW HGrdTri() );
        
    mind1 = ind1 = InsertPoint( pnd1);
    mind2 = ind2 = InsertPoint( pnd2);
    mind3 = ind3 = InsertPoint( pnd3);
    mind4 = ind4 = InsertPoint( pnd4);

    itri1 = InsertCell( ptri1);
    itri2 = InsertCell( ptri2);

    ptri1->rNode(0) = ind1;
    ptri1->rNode(1) = ind2;
    ptri1->rNode(2) = ind3;
    
    ptri2->rNode(0) = ind3;
    ptri2->rNode(1) = ind4;
    ptri2->rNode(2) = ind1;
    
    ptri1->rCell(2) = itri2;
    ptri2->rCell(2) = itri1;
    ptri1->SetCircCenter();
    ptri2->SetCircCenter();


    // inserting frontal points into mesh
    IterFro     itrfro;
    IterFSeg    itrsg;

    map<HGrdPnt*,int>           mapNod;
    map<HGrdPnt*,int>::iterator imap;

    for ( itrfro = mcolFro.begin(); itrfro != mcolFro.end(); itrfro++)
    {
        for ( itrsg = (*itrfro).begin(); itrsg != (*itrfro).end(); itrsg++)
        {
            itr = (*itrsg)->rPntRt();

            if ( ( imap = mapNod.find( *itr) ) == mapNod.end() )
            {
                InsertPointIntoMesh( itr);
                mapNod.insert( map<HGrdPnt*,int>::value_type( *itr, 0) );
            }

        }
    }           

    
#ifdef _DEBUG

    FILE    *ftmp = fopen( "initial.plt", "wt");
    ExportTECTmp( ftmp);
    fclose( ftmp);
    
#endif // _DEBUG
}




bool HGrid::IsOutside( const Vect2D& vct)
{
    IterFro     itrfro;
    IterFSeg    itrsg;
    Vect2D      v1, v2;
    double      x;

//// winding algorithm
//  MGFloat     alf;
//  for ( itrfro = mcolFro.begin(); itrfro != mcolFro.end(); itrfro++)
//  {
//      alf += (*itrfro).Angle( vct);
//  }
//  if ( fabs(alf) < M_PI )
//      return true;
//  else 
//      return false;


// ray casting algorithm
    MGInt   cross = 0;
    for ( itrfro = mcolFro.begin(); itrfro != mcolFro.end(); itrfro++)
        for ( itrsg = (*itrfro).begin(); itrsg != (*itrfro).end(); itrsg++)
        {
            v1 = *(*(*itrsg)->PntLf());
            v2 = *(*(*itrsg)->PntRt());

            if ( ( v1.Y() > vct.Y() && v2.Y() <= vct.Y() ) ||
                 ( v2.Y() > vct.Y() && v1.Y() <= vct.Y() ) )
            {

//              x = ( v1.X()*v2.Y() - v1.Y()*v2.X() ) / ( v2.Y() - v1.Y() );
                x = (v2.X() - v1.X())*(vct.Y() - v1.Y())/(v2.Y() - v1.Y())
                     + v1.X();

                if ( x > vct.X() ) 
                    ++cross;
            }

        }

    if ( (cross % 2) == 1 )
        return false;
    else 
        return true;
}


void HGrid::FlagOuterTris()
{
    IterFro     itrfro;
    IterFSeg    itrsg;
    IterGCell   itr, itrnb, itrcl;;
    Vect2D      vout, vcnt, vc1, vc2;

    CollGCell   colCell;


//  // flaging all triangles lying outside domain using N^2 algo
//  for ( itr = mcolCell.begin(); itr != mcolCell.end(); itr++)
//  {
//      if( IsOutside( (*itr)->Center() ) )
//          (*itr)->rIsOutside() = true;
//      else
//          (*itr)->rIsOutside() = false;
//  }

    Vect2D  v1, v2, vct;
//  MGInt   cross = 0;
    HGrdTri *ptri;
    MGFloat x, y1, y2;

    multimap<MGFloat, HGrdTri*>             mapCell;
    multimap<MGFloat, HGrdTri*>::iterator   imap, ifirst, ilast;

    for ( itr = mcolCell.begin(); itr != mcolCell.end(); itr++)
    {
        vct = (*itr)->Center();
        (*itr)->rCross() = 0;
        mapCell.insert( multimap<MGFloat, HGrdTri*>::value_type( vct.Y(), 
                                                                 (*itr) ) );
    }


    for ( itrfro = mcolFro.begin(); itrfro != mcolFro.end(); itrfro++)
        for ( itrsg = (*itrfro).begin(); itrsg != (*itrfro).end(); itrsg++)
        {
            v1 = *(*(*itrsg)->PntLf());
            v2 = *(*(*itrsg)->PntRt());
            if ( v1.Y() > v2.Y() )
            {
                y1 = v2.Y();
                y2 = v1.Y();
            }
            else
            {
                y1 = v1.Y();
                y2 = v2.Y();
            }

            ifirst = mapCell.lower_bound( y1 );
            ilast  = mapCell.upper_bound( y2 );

            for ( imap = ifirst; imap != ilast; ++imap)
            {
                ptri = (*imap).second;
                vct = ptri->Center();

                if ( ( v1.Y() > vct.Y() && v2.Y() <= vct.Y() ) ||
                     ( v2.Y() > vct.Y() && v1.Y() <= vct.Y() ) )
                {
                    x = (v2.X() - v1.X())*(vct.Y() - v1.Y())/(v2.Y() - v1.Y())
                        + v1.X();

                    if ( x > vct.X() ) 
                        ++(ptri->rCross());
                }
            }
        }
    
    for ( itr = mcolCell.begin(); itr != mcolCell.end(); itr++)
    {
        if ( ((*itr)->rCross() % 2) == 1 )
            (*itr)->rIsOutside() = false;
        else
            (*itr)->rIsOutside() = true;
    }


}


void HGrid::RemoveOuterTris()
{
    IterGCell   itr, itr2;
    HGrdTri     *ptri;

//    itr2 = NULL;
    itr2 = (IterGCell)NULL;
    for ( itr = mcolCell.begin(); itr != mcolCell.end(); itr++)
    {
 //       if ( itr2 != NULL)
        if ( itr2 != (IterGCell)NULL){
            ptri = *itr2;
            (*itr2)->InvalidateNeighb();
            mcolCell.erase( itr2);
            delete ptri;
            itr2 = (IterGCell)NULL;
        }
        if ( (*itr)->IsOutside() )
            itr2 = itr;
    }
//    if ( itr2 != NULL)
    if ( itr2 != (IterGCell)NULL){
        ptri = *itr2;
        (*itr2)->InvalidateNeighb();
        mcolCell.erase( itr2);
        delete ptri;
        itr2 = (IterGCell)NULL;
    }

    if ( *mind1) delete *mind1;
    if ( *mind2) delete *mind2;
    if ( *mind3) delete *mind3;
    if ( *mind4) delete *mind4;
    mcolPnt.erase( mind1);
    mcolPnt.erase( mind2);
    mcolPnt.erase( mind3);
    mcolPnt.erase( mind4);
    mind1 = (IterGPnt)NULL;
    mind2 = (IterGPnt)NULL;
    mind3 = (IterGPnt)NULL;
    mind4 = (IterGPnt)NULL;
}



bool HGrid::CheckSwapTriangles( HGrdTri *ptri1, HGrdTri *ptri2)
{
    HGrdTri tri1, tri2;

    tri1 = *ptri1;
    tri2 = *ptri2;

    SwapTriangles( &tri1, &tri2, false);

    if ( !tri1.Check() || !tri2.Check() )
        return false;

    //Guenther
    //if ( tri1.Area() < 0 || tri2.Area() < 0 )
    //    return false;

    return true;
}

/*
switch two triangles 

*/
void HGrid::SwapTriangles( HGrdTri *ptri1, HGrdTri *ptri2, bool bgo)
{
    MGInt       ifc1, ifc2;
    IterGPnt    ip1, ip2, ip3, ip4;
    IterGCell   ic1, ic2, ic3, ic4;

    IterGCell itri1, itri2;
    
//  TRACE( "--- swapping !!!");
    
    if ( ptri2->Node(1) == ptri1->Node(0) && ptri2->Node(0) == ptri1->Node(1) )
    {
        ip1 = ptri2->Node(1);
        ip2 = ptri2->Node(2);
        ip3 = ptri2->Node(0);
        ip4 = ptri1->Node(2);
        ifc1 = 0;
        ifc2 = 0;
        ic1 = ptri2->Cell(1);
        ic2 = ptri2->Cell(2);
        ic3 = ptri1->Cell(1);
        ic4 = ptri1->Cell(2);
        itri1 = ptri2->Cell(0);
        itri2 = ptri1->Cell(0);
    }
    else
    if ( ptri2->Node(0) == ptri1->Node(0) && ptri2->Node(2) == ptri1->Node(1) )
    {
        ip1 = ptri2->Node(0);
        ip2 = ptri2->Node(1);
        ip3 = ptri2->Node(2);
        ip4 = ptri1->Node(2);
        ifc1 = 0;
        ifc2 = 2;
        ic1 = ptri2->Cell(0);
        ic2 = ptri2->Cell(1);
        ic3 = ptri1->Cell(1);
        ic4 = ptri1->Cell(2);
        itri1 = ptri2->Cell(2);
        itri2 = ptri1->Cell(0);
    }
    else
    if ( ptri2->Node(2) == ptri1->Node(0) && ptri2->Node(1) == ptri1->Node(1) )
    {
        ip1 = ptri2->Node(2);
        ip2 = ptri2->Node(0);
        ip3 = ptri2->Node(1);
        ip4 = ptri1->Node(2);
        ifc1 = 0;
        ifc2 = 1;
        ic1 = ptri2->Cell(2);
        ic2 = ptri2->Cell(0);
        ic3 = ptri1->Cell(1);
        ic4 = ptri1->Cell(2);
        itri1 = ptri2->Cell(1);
        itri2 = ptri1->Cell(0);
    }
    else
    
    if ( ptri2->Node(1) == ptri1->Node(2) && ptri2->Node(0) == ptri1->Node(0) )
    {
        ip1 = ptri2->Node(1);
        ip2 = ptri2->Node(2);
        ip3 = ptri2->Node(0);
        ip4 = ptri1->Node(1);
        ifc1 = 2;
        ifc2 = 0;
        ic1 = ptri2->Cell(1);
        ic2 = ptri2->Cell(2);
        ic3 = ptri1->Cell(0);
        ic4 = ptri1->Cell(1);
        itri1 = ptri2->Cell(0);
        itri2 = ptri1->Cell(2);
    }
    else
    if ( ptri2->Node(0) == ptri1->Node(2) && ptri2->Node(2) == ptri1->Node(0) )
    {
        ip1 = ptri2->Node(0);
        ip2 = ptri2->Node(1);
        ip3 = ptri2->Node(2);
        ip4 = ptri1->Node(1);
        ifc1 = 2;
        ifc2 = 2;
        ic1 = ptri2->Cell(0);
        ic2 = ptri2->Cell(1);
        ic3 = ptri1->Cell(0);
        ic4 = ptri1->Cell(1);
        itri1 = ptri2->Cell(2);
        itri2 = ptri1->Cell(2);
    }
    else
    if ( ptri2->Node(2) == ptri1->Node(2) && ptri2->Node(1) == ptri1->Node(0) )
    {
        ip1 = ptri2->Node(2);
        ip2 = ptri2->Node(0);
        ip3 = ptri2->Node(1);
        ip4 = ptri1->Node(1);
        ifc1 = 2;
        ifc2 = 1;
        ic1 = ptri2->Cell(2);
        ic2 = ptri2->Cell(0);
        ic3 = ptri1->Cell(0);
        ic4 = ptri1->Cell(1);
        itri1 = ptri2->Cell(1);
        itri2 = ptri1->Cell(2);
    }
    else
    
    if ( ptri2->Node(1) == ptri1->Node(1) && ptri2->Node(0) == ptri1->Node(2) )
    {
        ip1 = ptri2->Node(1);
        ip2 = ptri2->Node(2);
        ip3 = ptri2->Node(0);
        ip4 = ptri1->Node(0);
        ifc1 = 1;
        ifc2 = 0;
        ic1 = ptri2->Cell(1);
        ic2 = ptri2->Cell(2);
        ic3 = ptri1->Cell(2);
        ic4 = ptri1->Cell(0);
        itri1 = ptri2->Cell(0);
        itri2 = ptri1->Cell(1);
    }
    else
    if ( ptri2->Node(0) == ptri1->Node(1) && ptri2->Node(2) == ptri1->Node(2) )
    {
        ip1 = ptri2->Node(0);
        ip2 = ptri2->Node(1);
        ip3 = ptri2->Node(2);
        ip4 = ptri1->Node(0);
        ifc1 = 1;
        ifc2 = 2;
        ic1 = ptri2->Cell(0);
        ic2 = ptri2->Cell(1);
        ic3 = ptri1->Cell(2);
        ic4 = ptri1->Cell(0);
        itri1 = ptri2->Cell(2);
        itri2 = ptri1->Cell(1);
    }
    else
    if ( ptri2->Node(2) == ptri1->Node(1) && ptri2->Node(1) == ptri1->Node(2) )
    {
        ip1 = ptri2->Node(2);
        ip2 = ptri2->Node(0);
        ip3 = ptri2->Node(1);
        ip4 = ptri1->Node(0);
        ifc1 = 1;
        ifc2 = 1;
        ic1 = ptri2->Cell(2);
        ic2 = ptri2->Cell(0);
        ic3 = ptri1->Cell(2);
        ic4 = ptri1->Cell(0);
        itri1 = ptri2->Cell(1);
        itri2 = ptri1->Cell(1);
    }

    ASSERT( itri1 != (IterGCell)NULL && itri2 != (IterGCell)NULL);

    ptri1->rNode(0) = ip2;  
    ptri1->rNode(1) = ip4;  
    ptri1->rNode(2) = ip1;  

    ptri1->rCell(0) = itri2;    
    ptri1->rCell(1) = ic4;  
    ptri1->rCell(2) = ic1;  


    ptri2->rNode(0) = ip4;  
    ptri2->rNode(1) = ip2;  
    ptri2->rNode(2) = ip3;  

    ptri2->rCell(0) = itri1;    
    ptri2->rCell(1) = ic2;  
    ptri2->rCell(2) = ic3;

    if ( bgo)
    {
        if ( ic1 != (IterGCell)NULL ){
            if ( (*ic1)->Cell(0) == itri2)
                (*ic1)->rCell(0) = itri1;
            else if ( (*ic1)->Cell(1) == itri2)
                (*ic1)->rCell(1) = itri1;
            else if ( (*ic1)->Cell(2) == itri2)
                (*ic1)->rCell(2) = itri1;
            else
                ASSERT(0);
        }

        if ( ic3 != (IterGCell)NULL){
            if ( (*ic3)->Cell(0) == itri1)
                (*ic3)->rCell(0) = itri2;
            else if ( (*ic3)->Cell(1) == itri1)
                (*ic3)->rCell(1) = itri2;
            else if ( (*ic3)->Cell(2) == itri1)
                (*ic3)->rCell(2) = itri2;
            else
                ASSERT(0);
        }
    }

}

/*
checking bounding 

*/

void HGrid::CheckBoundIntegr()
{
//  char        sbuf[256];
    IterFro     itrfro;
    IterFSeg    itrsg;
    Vect2D      v1, v2, v3, v4;
//  CollGCell   colCell;
    CollGCell   colPath;
    CollFSeg    colSeg;
    IterFSeg    iseg;
    IterGCell   itr, itrnb, itrcl;
    Vect2D      vcnt;
    bool        bReady, bFound;
//  MGInt       k, i=0;
    MGInt k;

    list<IterGCell>             lstCell;
    list<IterGCell>::iterator   itrtmp;

    multimap< HGrdPnt*, IterGCell >                     mapNod;
    pair< multimap< HGrdPnt*, IterGCell >::iterator, 
          multimap< HGrdPnt*, IterGCell >::iterator>    range[2];   
          // no of points per froseg

    multimap< HGrdPnt*, IterGCell >::iterator           itrmap1, itrmap2;


    // getting number of front segments
    int no = 0;
    for ( itrfro = mcolFro.begin(); itrfro != mcolFro.end(); itrfro++)
        for ( itrsg = (*itrfro).begin(); itrsg != (*itrfro).end(); itrsg++)
            ++no;


    for ( itr = mcolCell.begin(); itr != mcolCell.end(); itr++)
        for ( k=0; k<NUM_TRI; ++k)
          mapNod.insert( multimap<HGrdPnt*, 
                         IterGCell>::value_type(*(*itr)->Node(k), itr) );

    int iii=0;
    int nrec = 0;
    for ( itrfro = mcolFro.begin(); itrfro != mcolFro.end(); itrfro++)
        for ( itrsg = (*itrfro).begin(); itrsg != (*itrfro).end(); itrsg++)
        {
            ++iii;
            //if ( iii - 1000*(iii/1000) == 0)
            //  printf( "triang. iter = %d / %d  nrec = %d\n", iii, no, nrec);

            range[0] = mapNod.equal_range( *(*itrsg)->PntLf() );
            range[1] = mapNod.equal_range( *(*itrsg)->PntRt() );
            
            bReady = false;

            for ( itrmap1=range[0].first; itrmap1!=range[0].second; itrmap1++)
            {
                itrcl = (*itrmap1).second;
                lstCell.insert( lstCell.begin(), itrcl );
                for ( itrmap2=range[1].first; itrmap2!=range[1].second; 
                itrmap2++)
                {
                    if ( *itrcl == *(*itrmap2).second )
                    {
                        bReady = true;
                        goto out_of_loops;
                    }
                }
            }
            out_of_loops:

    
            // the segment is missing - must be recovered
            if ( ! bReady)
            {
                ++nrec;
                //printf( "triang. iter = %d / %d  nrec = %d\n", iii, no, nrec);

                v1 = *(*((*itrsg)->PntLf()));
                v2 = *(*((*itrsg)->PntRt()));


//              TRACE( "\n" );
//              sprintf( sbuf, "seg (%lg, %lg) (%lg, %lg) - lstCell.size = %d",
//              v1.X(), v1.Y(), v2.X(), v2.Y(), lstCell.size() );
//              TRACE( sbuf);

                bFound = false;
                for ( itrtmp = lstCell.begin(); itrtmp != lstCell.end(); 
                ++itrtmp)
                {
                    itr = *itrtmp;
                    if(( itrnb = (*itr)->NextCell( *itrsg, 
                       (IterGCell)NULL) ) != (IterGCell)NULL)
                    {
                        bFound = true;
                        break;
                    }

                }

                if ( bFound)
                {
                    IterGCell itr1, itr2, itro1=(IterGCell)NULL, 
                              itro2=(IterGCell)NULL;
                    stack<IterGCell>    stackCell;

//                  CheckGrid();

                    itr1 = itr;
                    itr2 = itrnb;

//                  TRACE2( "v1 = %lg %lg", v1.X(), v1.Y() );
//                  TRACE2( "v2 = %lg %lg", v2.X(), v2.Y() );

                    int niter = 0;

                    do 
                    {
                        ++niter;

//                      TRACE( "--- loop start");
//                      TRACE2( "itrcl = %d / %d", itr1, *itr1);
//                      (*itr1)->DumpTri();
//                      TRACE2( "itrcl = %d / %d", itr2, *itr2);
//                      (*itr2)->DumpTri();

           // check if swap is possible if not then try with another triangles
                        while ( /*! (*itr2)->IsVisible( itr1, v1) ||*/ 
                            ! CheckSwapTriangles( *itr1, *itr2 )
                            || ( itr1 == itro1 && itr2 == itro2 ) ) 
                          // avoid swaping the same triangles again
                        {
                            stackCell.push( itr1);
                            itr = itr2;
                            itr2 = (*itr)->NextCell( *itrsg, itr1);
                            itr1 = itr;
                            //Guenther
                            //ASSERT( itr2 != (IterGCell)NULL);
                        }

                        itro1 = itr1;
                        itro2 = itr2;


                        // pre swapping - remove itr1 and itr2 from map
                        for ( k=0; k< NUM_TRI; ++k)
                        {
                            range[0] = mapNod.equal_range( *(*itr1)->Node(k) );
                            for ( itrmap1=range[0].first; 
                                  itrmap1!=range[0].second; itrmap1++)
                            {
                                if ( (*itrmap1).second == itr1 )
                                {
                                    mapNod.erase( itrmap1);
                                    break;
                                }
                            }

                            range[0] = mapNod.equal_range( *(*itr2)->Node(k) );
                            for ( itrmap1=range[0].first; 
                                  itrmap1!=range[0].second; itrmap1++)
                            {
                                if ( (*itrmap1).second == itr2 )
                                {
                                    mapNod.erase( itrmap1);
                                    break;
                                }
                            }
                        }

                        if ( ! CheckSwapTriangles( *itr1, *itr2 ) )
                            THROW_INTERNAL( "Recovery: Swap not possible !!!");

                        SwapTriangles( *itr1, *itr2 );


                      // post swapping - insert modified itr1 and itr2 into map
                        for ( k=0; k< NUM_TRI; ++k)
                        {
                            mapNod.insert( multimap<HGrdPnt*, 
                            IterGCell>::value_type( *(*itr1)->Node(k), itr1) );
                            mapNod.insert( multimap<HGrdPnt*, 
                            IterGCell>::value_type( *(*itr2)->Node(k), itr2) );
                        }


                        if ( stackCell.empty() )
                        {
                            itrcl = (IterGCell)NULL;
                            itrnb = (IterGCell)NULL;

                            if((itr = (*itr1)->NextCell( (*itrsg), 
                                      (IterGCell)NULL )) != (IterGCell)NULL)
                            {
                                itrcl = itr1;
                                itrnb = itr;
                            }

                            if ( (itr = (*itr2)->NextCell( (*itrsg),
                                        (IterGCell)NULL )) != (IterGCell)NULL)
                            {
                                itrcl = itr2;
                                itrnb = itr;
                            }

                            itr1 = itrcl;
                            itr2 = itrnb;
                        }
                        else{
                            itr1 = stackCell.top();
                            stackCell.pop();

                            if ( stackCell.empty() )
                             itr2 = (*itr1)->NextCell(*itrsg, (IterGCell)NULL);
                            else
                                itr2 = (*itr1)->NextCell(*itrsg, 
                                                   (IterGCell)stackCell.top());
                        }


                    }
                    while ( itr1 != (IterGCell)NULL && itr2 != (IterGCell)NULL);

                    (*itrsg)->mbtmp = true;
                }
                else
                {
                    char    sbuf[1024];
                 sprintf( sbuf, "recovery problem with seg(%lg, %lg)(%lg, %lg)",
                             v1.X(), v1.Y(), v2.X(), v2.Y() );
                    TM_TRACE( sbuf);
                    TM_TRACE1( "recovwry lstCell.size = %d", lstCell.size() );
                    TM_TRACE1( "%d edges already recovered", nrec);

                    FILE *ff = fopen( "lstcell.plt", "wt");
                    for ( itrtmp = lstCell.begin(); itrtmp != lstCell.end(); 
                          ++itrtmp)
                    {
                        (*(*itrtmp))->DumpTEC( ff);
                    }
                    fclose( ff);

                    printf( "%s\n", sbuf);
                  printf( "recovery lstCell.size = %d\n", (int)lstCell.size() );

                    (*itrsg)->mbtmp = true;

                    FILE *f = fopen( "recovery.plt", "wt");
                    ExportTECTmp( f);
                    fclose( f);
//Guenther
//                    THROW_INTERNAL( "Recovery problem !!!");
                }
            }

            lstCell.erase( lstCell.begin(), lstCell.end() );
        }

//        printf( "%d edges recovered\n", nrec);
}






void HGrid::Generate()
{
    clock_t     start, tstart, finish;
    double      duration;

    try
    {
        tstart = clock();

        start = clock();

        InitTriangles();

        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
//        printf( "InitTriangles() - %2.1f seconds\n\n", duration );

        //FILE *f = fopen( "ini.plt", "wt");
        //ExportTECTmp( f);
        //fclose( f);

        TM_TRACE( "CheckBoundIntegr()");
//        printf( "CheckBoundIntegr()\n");

        start = clock();

        CheckBoundIntegr();

        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
//        printf( "CheckBoundIntegr() - %2.1f seconds\n\n", duration );

        TM_TRACE( "FlagOuterTris()");
//        printf( "FlagOuterTris()\n");
        FlagOuterTris();

        TM_TRACE( "RemoveOuterTris()");
//        printf( "RemoveOuterTris()\n");
        RemoveOuterTris();

        finish = clock();
        duration = (double)(finish - tstart) / CLOCKS_PER_SEC;
//        printf( "Generation total - %2.1f seconds\n\n", duration );

//      FILE *f = fopen( "param.plt", "wt");
//      ExportTECTmp( f);
//      fclose( f);
    }
    catch ( Except *pe)
    {
        ASSERT( pe);
        TM_TRACE_EXCEPTION( *pe);
        TM_TRACE_TO_STDERR( *pe);
        delete pe;
    }
}



void HGrid::ExportTECTmp( FILE *f)
{
    IterGPnt    pntitr;
    Vect2D      vct;

    IterGCell   cellitr, itrcl;
    MGInt       id1, id2, id3, id4;
    MGInt       ltri, ltmp;
    map<void*,MGInt,less<void*> >   tmpMap;

    TM_TRACE( "ExportTEC");


    fprintf( f, "TITLE = \"surface\"\n");
    fprintf( f, "VARIABLES = \"X\", \"Y\"\n");
    fprintf( f, "ZONE T=\"TRIANGLES\", ");
    fprintf( f, "N=%2ld, ", (long int) mcolPnt.size() );
    fprintf( f, "E=%2d, F=FEPOINT, ET=QUADRILATERAL C=BLACK\n ", 
             ltri = (MGInt)mcolCell.size() );


    ltmp = 0;
    for ( pntitr = mcolPnt.begin(); pntitr != mcolPnt.end(); pntitr++)
    {
        ltmp++;
        vct = *(*pntitr);
        fprintf( f, "%20.10lg %20.10lg\n", vct.X(), vct.Y() );
        tmpMap.insert( map<void*,MGInt,
                       less<void*> >::value_type((void*)(*pntitr), ltmp ));
    }
    
    TM_TRACE( "after Pnt");


    ltri = 0;
    for ( cellitr = mcolCell.begin(); cellitr != mcolCell.end(); cellitr++)
    {   
        ltri++;

        pntitr = (*cellitr)->Node(0);
        id1 = tmpMap[ (*pntitr)];

        pntitr = (*cellitr)->Node(1);
        id2 = tmpMap[ (*pntitr)];

        pntitr = (*cellitr)->Node(2);
        id4 = id3 = tmpMap[ (*pntitr)];
        
        fprintf( f, "%9ld %9ld %9ld %9ld\n", (long int)id1, (long int)id2, 
                (long int)id3, (long int)id4 );
    }
    

    TM_TRACE( "after Cells");
}


void HGrid::CheckGrid()
{
//  char        sbuf[256];
    bool        bFound;
    IterGCell   itr, itrnb;

//  TRACE("CheckGrid()\n");
    for ( itr = mcolCell.begin(); itr != mcolCell.end(); ++itr)
    {
//      sprintf( sbuf, "cel = %d   | %d %d %d", itr, (*itr)->Cell(0), 
//(*itr)->Cell(1), (*itr)->Cell(2) );
//      TRACE( sbuf);
        for ( int i=0; i<NUM_TRI; ++i)
        {
            bFound = true;
            itrnb = (*itr)->Cell(i);
            if ( itrnb != (IterGCell)NULL){
                ASSERT( *itrnb );
                bFound = false;
                for ( int k=0; k<NUM_TRI; ++k)
                {
                    if ( (*itrnb)->Cell(k) == itr )
                        bFound = true;
                }
            }

            if ( ! bFound)
            {
                THROW_INTERNAL( "Cells pointers are not consistent !");
            }
        }
    }
}



void HGrid::WriteFrontTEC( const char name[])
{
}



INIT_TRACE;
}



