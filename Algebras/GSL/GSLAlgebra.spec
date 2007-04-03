#This file is part of SECONDO.

#Copyright (C) 2007, University in Hagen, Department of Computer Science,
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

operator rng_init alias RNG_INIT pattern op ( _, _ )
operator rng_setSeed alias RNG_SETSEED pattern op ( _ )
operator rng_getSeed alias RNG_GETSEED pattern op ( _ )
operator rng_getType alias RNG_GETTYPE pattern op ( _ )
operator rng_getMin alias RNG_GETMIN pattern op ( _ )
operator rng_getMax alias RNG_GETMAX pattern op ( _ )

operator rng_TypeDescriptors alias RNG_TYPEDESCRIPTORS pattern op ( _ )
operator rng_NoGenerators alias RNG_NOGENERATORS pattern op ( _ )
operator rng_GeneratorName alias RNG_GENERATORNAME pattern op ( _ )
operator rng_GeneratorMaxRand alias RNG_GENERATORMAXRAND pattern op ( _ )
operator rng_GeneratorMinRand alias RNG_GENERATORMINRAND pattern op ( _ )

operator rng_int alias RNG_INT pattern op ( _ )
operator rng_intN alias RNG_INTN pattern op ( _ )
operator rng_real alias RNG_REAL pattern op ( _ )
operator rng_realpos alias RNG_REALPOS pattern op ( _ )
operator rng_flat alias RNG_FLAT pattern op ( _ , _ )
operator rng_binomial alias RNG_BINOMIAL pattern op ( _, _ )
operator rng_gaussian alias RNG_GAUSSIAN pattern op ( _ )
operator rng_exponential alias RNG_EXPONENTIAL pattern op ( _ )
operator rng_geometric alias RNG_GEOMETRIC pattern op ( _ )
operator rng_poisson alias RNG_POISSON pattern op ( _ )
