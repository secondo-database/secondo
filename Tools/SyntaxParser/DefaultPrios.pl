/*

----
This file is part of SECONDO.

Copyright (C) 2014, University in Hagen, Department of Computer Science,
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



:- op(800, xf , ++).
%:- op(200, xfx, :).
% NVK MODIFIED for nested relations support
% With xfy tests for nested relations like A:_ and X:Y:_ are supported.
% But note: (a:b):c is not valid. The ~:~ is used like a list, hence a
% predicate like the ~append/3~ for lists is needed to add an attribute to a
% existing x:y term.
:- op(200, xfy, :).
% NVK MODIFIED END
:- op(800, xfx, =>).
:- op(800, xfx, ==).
:- op(800, xfx, <=).
:- op(800, xfx, <<).
:- op(800, xfx, <<==).
:- op(800, xfx, >>).
:- op(800, xfx, >>==).
:- op(800, xfx, #).
:- op(800, xfx, ##).
:- op(800, xfx, adjacent).
:- op(800, xfx, always).
:- op(800, xfx, and).
:- op(800, xfx, at).
:- op(800, xfx, atinstant).
:- op(800, xfx, atperiods).
:- op(800, xfx, attached).
:- op(800, xfx, before).
:- op(800, xf , getCategory).
:- op(800, xfx, commonborder).
:- op(800, xfx, commonborderscan).
:- op(800, xfx, contains).
:- op(800, xfx, div).
:- op(800, xfx, eplus).
:- op(800, xfx, equalway).
:- op(800, xfx, ininterior).
:- op(800, xfx, inside).
:- op(800, xfx, insideold).
:- op(800, xfx, intersection).
:- op(800, xfx, intersects).
:- op(800, xfx, intersects_new).
:- op(800, xf , isgrayscale).
:- op(800, xf , isportrait).
:- op(800, xf , getHeight).
:- op(800, xf , leapyear).
:- op(800, xf , line2region).
:- op(800, xfx, matches).
:- op(800, xfx, mod).
:- op(800, xfx, never).
:- op(800,  fx, not).
:- op(800, xfx, onborder).
:- op(800, xfx, overlaps).
:- op(800, xfx, or).
:- op(800, xfx, p_intersects).
:- op(800, xfx, passes).
:- op(800, xf , getPictureDate).
:- op(800, xfx, present).
:- op(800, xf , rect2region).
:- op(800, xf , relcount).
:- op(800, xf , relcount2).
:- op(800, xfx, satisfies).
:- op(800, xfx, simpleequals).
:- op(800, xfx, starts).
:- op(800, xfx, touchpoints).
:- op(800, xfx, union).
:- op(800, xfx, when).
 
% NVK ADDED NR 
:- op(799, yfx, unnest),
   op(799, yfx, nest).
% NVK ADDED NR END
