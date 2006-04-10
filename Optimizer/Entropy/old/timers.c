/*----------------------------------------------------------------------- 
// Copyright (C) 2005 
// M. Spiekermann
//
// Changes to compile it with MSYS/MINGW
// However, the function get_wall_clock_time will now only have
// a precision of seconds instead of milli seconds. I hope
// this will be not a problem :-)
//----------------------------------------------------------------------*/

/*------------------------------------------------------------------------
// Copyright (C) 1993,1994: 
// J.C. Meza
// Sandia National Laboratories
// meza@california.sandia.gov
//----------------------------------------------------------------------*/


#include <sys/param.h>
#include <sys/types.h>
#include <time.h>
//#include <sys/times.h>
//#include <sys/resource.h>
#include <stddef.h>

#ifdef __alpha
#define HZ 100
#else
#define HZ CLOCKS_PER_SEC
#endif

double get_cpu_time()
{
/* ********************************************************************
**
**  Name: get_cpu_time
**
**  Purpose: general purpose CPU timing routine.
**
**  Arguments: none.
**
**  Return Value: user CPU time in (double) seconds.
**
**  Revision History:
**
**  10-May-94 -- initial development of get_cpu_time ().
**
** *******************************************************************/

    /*  struct tms tms;
     */
    double time;
/*
**  Begin get_cpu_time.
*/
    /*  times (&tms);
     */
    time = (double) clock() / (double) HZ;

    return time;
/*
**  End get_cpu_time.
*/
}
double get_wall_clock_time()
{
/* ********************************************************************
**
**  Name: get_wall_clock_time
**
**  Purpose: general purpose wall-clock timing routine.
**
**  Arguments: none.
**
**  Return Value: time in (double) seconds since the Epoch.
**
**  Notes: The Paragon specific dclock() routine is used to avoid
**         unnecessary references from each node back to the boot
**         node as is required for system calls like gettimeofday(),
**         getrusage() or times().
**
**  Revision History:
**
**  10-May-94 -- TXF; initial development of get_wall_clock_time ().
**
** *******************************************************************/

    /* double time;
    // struct timeval tp;
    */
    

/*
**  Begin get_wall_clock_time.
*/

    /* void* tzp = NULL;
    // gettimeofday (&tp, tzp);
    // time = (double) tp.tv_sec + ((double) tp.tv_usec / (double) 1.0e06);
    */

    return(time(NULL));
/*
**  End get_wall_clock_time.
*/
}
