//tabstop=4
//*****************************************************************************/
// Project: jpl
//
// File:    $Id$
// Date:    $Date$
// Author:  Fred Dushin <fadushin@syr.edu>
//          
//
// Description:
//    
//
// -------------------------------------------------------------------------
// Copyright (c) 1998 Fred Dushin
//                    All rights reserved.
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library Public License for more details.
//*****************************************************************************/
package jpl;



//----------------------------------------------------------------------/
// PrologException
/**
 * An exception of this type is thrown if, in evaluating a Query,
 * an exception is thrown in Prolog via the Prolog throw/1 predicate.
 * <p>
 * This method provides High-Level interface programmers to handle
 * such exceptions, providing error management between Prolog and Java.
 * <p>
 * Use the exception_term() accessor to obtain the Term that was
 * thrown via the throw/1 Prolog predicate.
 * 
 * <hr><i>
 * Copyright (C) 1998  Fred Dushin<p>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.<p>
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library Public License for more details.<p>
 * </i><hr>
 * @author  Fred Dushin <fadushin@syr.edu>
 * @version $Revision$
 */
// Implementation notes:  
// 
//----------------------------------------------------------------------/
public final class
PrologException extends JPLException
{
	private Term term_ = null;
	
	protected
	PrologException( Term term )
	{
		super( "PrologException: " + term.toString() );
		
		this.term_ = term;
	}
	
	/**
	 * @return a reference to the Term thrown by the call to throw/1
	 */
	public Term
	term()
	{
		return this.term_;
	}
}
