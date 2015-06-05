/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2004-2007, University in Hagen, Department of Computer Science,
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

*/

#define STDOUT        std::clog

#ifndef LOGGING_SWITCH_OFF

#ifdef LOGGING        // full logging
#undef LOG_EXP
#define LOG_EXP(act)       act;

#ifdef __GNUC__
#undef LOGP
#define LOGP        STDOUT<<__FILE \
__<<"   "<<__LINE__<<"   "<<__func__<<"\n"<<std::endl;
#else
#undef LOGP
#define LOGP        STDOUT<<__func__<<"\n";
#endif
#undef LOG
#define LOG(msg)        STDOUT<<msg<<"\n"<<std::endl;

#else

#ifdef LOGGINGP        //just log places
#ifdef __GNUC__
#undef LOGP
#define LOGP        STDOUT<<__FILE__<<"   "<<__LINE__<<"   "<<__func__<<"\n";
#else
#undef LOGP
#define LOGP        STDOUT<<__func__<<"\n";
#endif
#else
#undef LOGP
#define LOGP
#endif

#undef LOG
#undef LOG_EXP
#define LOG(msg)
#define LOG_EXP(act)
#endif
#else //local switch off
#undef LOG
#undef LOGP
#undef LOG_EXP
#define LOGP
#define LOG(msg)
#define LOG_EXP(act)
#undef LOGGING_SWITCH_OFF
#endif
