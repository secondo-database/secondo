/* tabstop=4 */
/***********************************************************************
 *  Project: jpl
 *
 *  File:    $Id$
 *  Date:    $Date$
 *  Author:  Fred Dushin <fadushin@syr.edu>
 *
 *
 *  Description:  This file contains the implementations of all
 * 	the static native methods in the class jpl.fli.Prolog.  For
 *  the most part, the implementations are strightforward class
 *  to the corresponding C functions by the same signature.
 *
 * Conventions:
 * - All java parameter names begin with the letter 'j'
 *   e.g, jint jsize
 * - Java object instance names are all lower-case, _ separated
 *   e.g., jobject jstring_holder
 * - Java class instance names are upper-case separated
 *   e.g., jclass jStringHolder
 *
 * ---------------------------------------------------------------------
 * Copyright (C) 2004, University in Hagen, Department of Computer Science, 
 * Database Systems for New Applications.
 
 * Copyright (c) 1998 Fred Dushin
 * 					  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library Public License for more details.
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <jni.h>
#include <SWI-Prolog.h>
#include <SecondoPL.h>

typedef long pointer;

/************************************************************************
 * Utility functions
 ***********************************************************************/

/*-----------------------------------------------------------------------
 * getLongValue
 *
 * Retrieves the value in a Java LongHolder class instance
 * 
 * @param   env           Java environment
 * @param   jlong_holder  the LongHolder class instance
 * @return                the LongHolder's value
 *---------------------------------------------------------------------*/
static long
getLongValue( 
	JNIEnv     *env, 
	jobject     jlong_holder )
{
	jfieldID fid;
	jclass   jLongHolder;
	
	if ( jlong_holder == NULL ){
		return (long) NULL;
	}
	
	jLongHolder = (*env)->FindClass( env, "jpl/fli/LongHolder" );
	fid = (*env)->GetFieldID( env, jLongHolder, "value", "J" );
	if ( fid == NULL ){
		fprintf( stderr, "getLongValue: Could not access fid for value field\n" );
		return;
	}
	
	return (*env)->GetLongField( env, jlong_holder, fid );
}

/*-----------------------------------------------------------------------
 * getPointerValue
 * 
 * Retrieves the value in a Java Pointer class instance
 * 
 * @param   env       Java environment
 * @param   jpointer  the Pointer class instance
 * @return            the Pointer's value
 *---------------------------------------------------------------------*/
static pointer
getPointerValue( 
	JNIEnv     *env, 
	jobject     jpointer_holder )
{
	jfieldID fid;
	jclass   jPointerHolder;
	
	if ( jpointer_holder == NULL ){
		return (pointer) NULL;
	}
	
	jPointerHolder = (*env)->FindClass( env, "jpl/fli/PointerHolder" );
	fid = (*env)->GetFieldID( env, jPointerHolder, "value", "J" );
	if ( fid == NULL ){
		fprintf( stderr, "getPointerValue: Could not access fid for value field\n" );
		return;
	}
	
	return (*env)->GetLongField( env, jpointer_holder, fid );
}


/*-----------------------------------------------------------------------
 * setPointerValue
 * 
 * Sets the value in a Java Pointer class instance to the supplied value
 * 
 * @param   env       Java environment
 * @param   jpointer  the Pointer class instance
 * @param   value     the new value
 *---------------------------------------------------------------------*/
static void
setPointerValue( 
	JNIEnv     *env, 
	jobject     jpointer_holder,
	pointer     value )
{
	jfieldID fid = 0;
	jclass   jPointerHolder;
	
	if ( jpointer_holder == NULL ){
		return;
	}
	
	jPointerHolder = (*env)->FindClass( env, "jpl/fli/PointerHolder" );
	fid      = (*env)->GetFieldID( env, jPointerHolder, "value", "J" );
	if ( fid == NULL ){
		fprintf( stderr, "setPointerValue: Could not access fid for value field\n" );
		return;
	}
	(*env)->SetLongField( env, jpointer_holder, fid, value );
}

/*-----------------------------------------------------------------------
 * setIntValue
 * 
 * Sets the value in a Java IntHolder class instance to the supplied value
 * 
 * @param   env          Java environment
 * @param   jint_holder  the IntHolder class instance
 * @param   value        the new value
 *---------------------------------------------------------------------*/
static void
setIntValue( 
	JNIEnv     *env, 
	jobject     jint_holder,
	int         value )
{
	jfieldID fid = 0;
	jclass   jIntHolder;
	
	if ( jint_holder == NULL ){
		return;
	}
	
	jIntHolder = (*env)->FindClass( env, "jpl/fli/IntHolder" );
	fid        = (*env)->GetFieldID( env, jIntHolder, "value", "I" );
	if ( fid == NULL ){
		fprintf( stderr, "setIntValue: Could not access fid for value field\n" );
		return;
	}
	(*env)->SetIntField( env, jint_holder, fid, value );
}

/*-----------------------------------------------------------------------
 * setLongValue
 * 
 * Sets the value in a Java LongHolder class instance to the supplied value
 * 
 * @param   env          Java environment
 * @param   jlong_holder  the LongHolder class instance
 * @param   value        the new value
 *---------------------------------------------------------------------*/
static void
setLongValue( 
	JNIEnv     *env, 
	jobject     jlong_holder,
	long        value )
{
	jfieldID fid = 0;
	jclass   jLongHolder;
	
	if ( jlong_holder == NULL ){
		return;
	}
	
	jLongHolder = (*env)->FindClass( env, "jpl/fli/LongHolder" );
	fid         = (*env)->GetFieldID( env, jLongHolder, "value", "J" );
	if ( fid == NULL ){
		fprintf( stderr, "setLongValue: Could not access fid for value field\n" );
		return;
	}
	(*env)->SetLongField( env, jlong_holder, fid, value );
}

/*-----------------------------------------------------------------------
 * setDoubleValue
 * 
 * Sets the value in a Java DoubleHolder class instance to the supplied value
 * 
 * @param   env          Java environment
 * @param   jdouble_holder  the DoubleHolder class instance
 * @param   value        the new value
 *---------------------------------------------------------------------*/
static void
setDoubleValue( 
	JNIEnv     *env, 
	jobject     jdouble_holder,
	double      value )
{
	jfieldID fid = 0;
	jclass   jDoubleHolder;
	
	if ( jdouble_holder == NULL ){
		return;
	}
	
	jDoubleHolder = (*env)->FindClass( env, "jpl/fli/DoubleHolder" );
	fid           = (*env)->GetFieldID( env, jDoubleHolder, "value", "D" );
	if ( fid == NULL ){
		fprintf( stderr, "setDoubleValue: Could not access fid for value field\n" );
		return;
	}
	(*env)->SetDoubleField( env, jdouble_holder, fid, value );
}

/*-----------------------------------------------------------------------
 * setStringValue
 * 
 * Sets the value in a Java StringHolder class instance to the supplied value
 * 
 * @param   env             Java environment
 * @param   jstring_holder  the StringHolder class instance
 * @param   value           the new value
 *---------------------------------------------------------------------*/
static void
setStringValue( 
	JNIEnv     *env, 
	jobject     jstring_holder,
	jstring     value )
{
	jfieldID fid = 0;
	jclass   jStringHolder;
	
	if ( jstring_holder == NULL ){
		return;
	}
	
	jStringHolder = (*env)->FindClass( env, "jpl/fli/StringHolder" );
	fid           = (*env)->GetFieldID( env, jStringHolder, "value", "Ljava/lang/String;" );
	if ( fid == NULL ){
		fprintf( stderr, "setStringValue: Could not access fid for value field\n" );
		return;
	}
	(*env)->SetObjectField( env, jstring_holder, fid, value );
}



/************************************************************************
 * C implementations of the Java native methods
 * 
 * NOTE: Many of these functions are still umimplemented.
 ***********************************************************************/



/*
 * Class:     jpl_fli_PL
 * Method:    new_term_refs
 * Signature: (I)Ljpl/fli/term_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_new_1term_1refs(
	JNIEnv     *env, 
	jclass      jProlog, 
	jint        jn )
{
	jobject rval;
	term_t  t0;
	jclass  jterm_t;
	
	/* create the prolog term_t references */
	t0 = PL_new_term_refs( (int)jn );
	
	/* create the java term_t and set its value */
	jterm_t = (*env)->FindClass( env, "jpl/fli/term_t" );
	rval    = (*env)->AllocObject( env, jterm_t );
	setLongValue( env, rval, (long)t0 );
	
	return rval;
}

/*
 * Class:     jpl_fli_PL
 * Method:    new_term_ref
 * Signature: ()Ljpl/fli/term_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_new_1term_1ref(
	JNIEnv     *env, 
	jclass      jProlog )
{
	jobject rval;
	term_t  term;
	jclass  jterm_t;
	
	/* create the prolog term_t reference */
	term = PL_new_term_ref();
	
	/* create the java term_t and set its value */
	jterm_t = (*env)->FindClass( env, "jpl/fli/term_t" );
	rval = (*env)->AllocObject( env, jterm_t );
	setLongValue( env, rval, (long)term );
	
	return rval;
}

/*
 * Class:     jpl_fli_PL
 * Method:    copy_term_ref
 * Signature: (Ljpl/fli/term_t;)Ljpl/fli/term_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_copy_1term_1ref(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jfrom )
{
	jobject rval;
	term_t  term;
	jclass  jterm_t;
	
	/* create the prolog term_t reference */
	term = PL_copy_term_ref( getLongValue( env, jfrom ) );
	
	/* create the java term_t and set its value */
	jterm_t = (*env)->FindClass( env, "jpl/fli/term_t" );
	rval = (*env)->AllocObject( env, jterm_t );
	setLongValue( env, rval, (long)term );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    reset_term_refs
 * Signature: (Ljpl/fli/term_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_reset_1term_1refs(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jafter )
{
	PL_reset_term_refs( getLongValue( env, jafter ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    new_atom
 * Signature: (Ljava/lang/String;)Ljpl/fli/atom_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_new_1atom(
	JNIEnv     *env, 
	jclass      jProlog, 
	jstring     jname )
{
	jobject     rval = NULL;
	atom_t      atom;
	const char *name;
	jclass      jatom_t;
	
	/* create the prolog atom_t */
	name = (*env)->GetStringUTFChars( env, jname, 0 );

	atom = PL_new_atom( name );
	(*env)->ReleaseStringUTFChars( env, jname, name );
	
	/* create the java atom_t and set its value */
	jatom_t = (*env)->FindClass( env, "jpl/fli/atom_t" );
	rval    = (*env)->AllocObject( env, jatom_t );

	setLongValue( env, rval, (long)atom );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    atom_chars
 * Signature: (Ljpl/fli/atom_t;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL 
Java_jpl_fli_Prolog_atom_1chars(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jatom )
{
	jstring     rval;
	const char *s;
	
	s = (const char *)PL_atom_chars( (atom_t) getLongValue( env, jatom ) );
	
	rval = (*env)->NewStringUTF( env, s );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    new_functor
 * Signature: (Ljpl/fli/atom_t;I)Ljpl/fli/functor_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_new_1functor(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jatom, 
	jint        arity )
{
	jobject     rval;
	functor_t   functor;
	jclass      jfunctor_t;
	
	/* create the prolog functor_t */
	functor = PL_new_functor( getLongValue( env, jatom ), (int)arity );
	
	/* create the java functor_t and set its value */
	jfunctor_t = (*env)->FindClass( env, "jpl/fli/functor_t" );
	rval       = (*env)->AllocObject( env, jfunctor_t );
	setLongValue( env, rval, (long)functor );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    functor_name
 * Signature: (Ljpl/fli/functor_t;)Ljpl/fli/atom_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_functor_1name(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jfunctor )
{
	jobject     rval;
	atom_t      atom;
	jclass      jatom_t;
	
	/* get the prolog atom_t */
	atom = PL_functor_name( getLongValue( env, jfunctor ) );
	
	/* create the java atom_t and set its value */
	jatom_t    = (*env)->FindClass( env, "jpl/fli/atom_t" );
	rval       = (*env)->AllocObject( env, jatom_t );
	setLongValue( env, rval, (long)atom );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    functor_arity
 * Signature: (Ljpl/fli/functor_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_functor_1arity(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jfunctor )
{
	int rval;
	
	/* get the prolog arity */
	rval = PL_functor_arity( getLongValue( env, jfunctor ) );
	
	return (jint) rval;
}





/* various *get* functions */


/*
 * Class:     jpl_fli_PL
 * Method:    get_atom
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/atom_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1atom(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jatom )
{
	jint   rval = JNI_FALSE;
	term_t term;
	atom_t atom;
	
	term = (term_t)getLongValue( env, jterm );
	rval = PL_get_atom( term, &atom );
	setLongValue( env, jatom, (long)atom );

	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_atom_chars
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/StringHolder;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1atom_1chars(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jstring_holder )
{
	jint     rval = JNI_FALSE;
	term_t   term;
	atom_t   atom;
	char    *s;
	jstring  string;
	
	term = (term_t)getLongValue( env, jterm );
	rval = PL_get_atom_chars( term, &s );
	
	string = (*env)->NewStringUTF( env, s );
	
	setStringValue( env, jstring_holder, string );

	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_string
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/StringHolder;Ljpl/fli/IntHolder;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1string(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jstring_holder )
{
	jint     rval = JNI_FALSE;
	term_t   term;
	char    *s;
	jstring  string;
	int      length;
	
	term = (term_t)getLongValue( env, jterm );
	rval = PL_get_string( term, &s, &length );
	
	string = (*env)->NewStringUTF( env, s );
	setStringValue( env, jstring_holder, string );

	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_list_chars
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/StringHolder;I)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1list_1chars(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jstring_holder, 
	jint        jflags )
{
	jint    rval;
	char    *s;
	jstring string;
	
	rval = 
		(int) PL_get_list_chars( 
			(term_t)   getLongValue( env, jterm ),
			           &s, 
			(unsigned) jflags );
	
	string = (*env)->NewStringUTF( env, s );
	setStringValue( env, jstring_holder, string );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_chars
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/StringHolder;I)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1chars(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jstring_holder, 
	jint        jflags )
{
	jint    rval;
	char    *s;
	jstring string;
	
	rval = 
		(int) PL_get_chars( 
			(term_t)   getLongValue( env, jterm ),
			           &s, 
			(unsigned) jflags );
	
	string = (*env)->NewStringUTF( env, s );
	setStringValue( env, jstring_holder, string );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_integer
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/IntHolder;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1integer(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jint_holder )
{
	jint    rval;
	int     i;
	
	rval = 
		(int) PL_get_integer( 
			(term_t)   getLongValue( env, jterm ),
			           &i );
	
	setIntValue( env, jint_holder, i );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_long
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/LongHolder;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1long(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jlong_holder )
{
	jint    rval;
	long    l;
	
	rval = 
		(int) PL_get_long( 
			(term_t)   getLongValue( env, jterm ),
			           &l );
	
	setLongValue( env, jlong_holder, l );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_pointer
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/Pointer;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1pointer(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jpointer )
{
	jint    rval;
	void    *ptr;
	
	rval = 
		(int) PL_get_pointer( 
			(term_t)   getLongValue( env, jterm ),
			           &ptr );
	
	setPointerValue( env, jpointer, (pointer)ptr );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_float
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/DoubleHolder;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1float(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jdouble_holder )
{
	jint    rval;
	double  d;
	
	rval = 
		(int) PL_get_float( 
			(term_t)   getLongValue( env, jterm ),
			           &d );
	
	setDoubleValue( env, jdouble_holder, d );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_functor
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/functor_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1functor(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jfunctor )
{
	jint      rval = JNI_FALSE;
	term_t    term;
	functor_t functor;
	
	term = (term_t)getLongValue( env, jterm );
	rval = PL_get_functor( term, &functor );
	setLongValue( env, jfunctor, (long)functor );

	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_name_arity
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/atom_t;Ljpl/fli/IntHolder;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1name_1arity(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jatom, 
	jobject     jint_holder )
{
	jint      rval = JNI_FALSE;
	term_t    term;
	atom_t    atom;
	int       arity;
	
	term = (term_t)getLongValue( env, jterm );
	rval = PL_get_name_arity( term, &atom, &arity );

	setLongValue( env, jatom, (long)atom );
	setIntValue( env, jint_holder, arity );

	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_module
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/Pointer;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1module(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jmodule )
{
	jint      rval = JNI_FALSE;
	term_t    term;
	module_t  module;
	
	term = (term_t)getLongValue( env, jterm );
	rval = PL_get_module( term, &module );

	setPointerValue( env, jmodule, (pointer)module );

	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_arg
 * Signature: (ILjpl/fli/term_t;Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1arg(
	JNIEnv     *env, 
	jclass      jProlog, 
	jint        jindex, 
	jobject     jterm, 
	jobject     jarg )
{
	jint      rval = JNI_FALSE;
	term_t    term;
	term_t    arg;
	
	term = (term_t)getLongValue( env, jterm );
	arg  = (term_t)getLongValue( env, jarg );
	rval = PL_get_arg( jindex, term, arg );
	setLongValue( env, jarg, (long)arg );

	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_list
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/term_t;Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1list(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jlist, 
	jobject     jhead, 
	jobject     jtail )
{
	jint    rval;
	
	rval = 
		(int) PL_get_list( 
			(term_t) getLongValue( env, jlist ),
			(term_t) getLongValue( env, jhead ),
			(term_t) getLongValue( env, jtail ) );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_head
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1head(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jlist, 
	jobject     jhead )
{
	jint    rval;
	
	rval = 
		(int) PL_get_head( 
			(term_t) getLongValue( env, jlist ),
			(term_t) getLongValue( env, jhead ) );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_tail
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1tail(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jlist, 
	jobject     jtail )
{
	jint    rval;
	
	rval = 
		(int) PL_get_tail( 
			(term_t) getLongValue( env, jlist ),
			(term_t) getLongValue( env, jtail ) );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    get_nil
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_get_1nil(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jlist )
{
	jint    rval;
	
	rval = 
		(int) PL_get_nil( 
			(term_t) getLongValue( env, jlist ) );
	
	return rval;
}








/*
 * Class:     jpl_fli_PL
 * Method:    term_type
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_term_1type(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_term_type(  getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_variable
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1variable(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_variable( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_atom
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1atom(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_variable( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_integer
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1integer(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_integer( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_string
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1string(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_string( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_float
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1float(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_float( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_compound
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1compound(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_compound( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_functor
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/functor_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1functor(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm,
	jobject     jfunctor )
{
	return (jint) PL_is_functor( getLongValue( env, jterm ),  
	                             getLongValue( env, jfunctor ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_list
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1list(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_list( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_atomic
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1atomic(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_atomic( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    is_number
 * Signature: (Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_is_1number(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	return (jint) PL_is_number( getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_variable
 * Signature: (Ljpl/fli/term_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1variable(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm )
{
	PL_put_variable( 
		getLongValue( env, jterm ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_atom
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/atom_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1atom(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jatom )
{
	PL_put_atom( 
		getLongValue( env, jterm ),
		getLongValue( env, jatom ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_atom_chars
 * Signature: (Ljpl/fli/term_t;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1atom_1chars(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jstring     jchars )
{
	const char  *chars;
	
	chars = (*env)->GetStringUTFChars( env, jchars, NULL );

	PL_put_atom_chars( getLongValue( env, jterm ), chars );

	(*env)->ReleaseStringUTFChars( env, jchars, chars );
	return;
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_string_chars
 * Signature: (Ljpl/fli/term_t;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1string_1chars(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jstring     jchars )
{
	const char  *chars;
	
	chars = (*env)->GetStringUTFChars( env, jchars, NULL );

	PL_put_string_chars( getLongValue( env, jterm ), chars );

	(*env)->ReleaseStringUTFChars( env, jchars, chars );
	return;
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_list_chars
 * Signature: (Ljpl/fli/term_t;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1list_1chars(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jstring     jchars )
{
	const char  *chars;
	
	chars = (*env)->GetStringUTFChars( env, jchars, NULL );

	PL_put_string_chars( getLongValue( env, jterm ), chars );

	(*env)->ReleaseStringUTFChars( env, jchars, chars );
	return;
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_integer
 * Signature: (Ljpl/fli/term_t;J)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1integer(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm,
	jlong       ji )
{
	PL_put_integer(
		(term_t) getLongValue( env, jterm ),
		(long) ji );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_pointer
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/Pointer;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1pointer(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jpointer )
{
	PL_put_pointer(
		(term_t) getLongValue( env, jterm ),
		(void *) getPointerValue( env, jpointer ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_float
 * Signature: (Ljpl/fli/term_t;D)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1float(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jdouble     jf )
{
	PL_put_float(
		(term_t) getLongValue( env, jterm ),
		(double) jf );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_functor
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/functor_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1functor(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jfunctor )
{
	PL_put_functor(
		(term_t) getLongValue( env, jterm ),
		(functor_t) getLongValue( env, jfunctor ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_list
 * Signature: (Ljpl/fli/term_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1list(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jlist )
{
	PL_put_list(
		(term_t) getLongValue( env, jlist ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_nil
 * Signature: (Ljpl/fli/term_t;)V
 */
JNIEXPORT void 
JNICALL Java_jpl_fli_Prolog_put_1nil(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jlist )
{
	PL_put_nil(
		(term_t) getLongValue( env, jlist ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    put_term
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/term_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_put_1term(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm1, 
	jobject     jterm2 )
{
	PL_put_term( 
		getLongValue( env, jterm1 ),
		getLongValue( env, jterm2 ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    cons_functor
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/functor_t;[Ljpl/fli/term_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_cons_1functor(
	JNIEnv      *env, 
	jclass       jProlog, 
	jobject      jterm, 
	jobject      jfunctor, 
	jobjectArray jterms )
{
	jsize length;
	term_t term0, term1, term2, term3, term4;
	
	length = (*env)->GetArrayLength( env, jterms );
	
	switch ( length ){
	case 0:
		PL_cons_functor( 
			getLongValue( env, jterm ),
			getLongValue( env, jfunctor ) );
		break;
	case 1:
		term0 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 0 ) );
		PL_cons_functor( 
			getLongValue( env, jterm ),
			getLongValue( env, jfunctor ),
			term0 );
		break;
	case 2:
		term0 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 0 ) );
		term1 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 1 ) );
				 
		PL_cons_functor( 
			getLongValue( env, jterm ),
			getLongValue( env, jfunctor ),
			term0,
			term1 );
		break;
	case 3:
		term0 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 0 ) );
		term1 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 1 ) );
		term2 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 2 ) );

		PL_cons_functor( 
			getLongValue( env, jterm ),
			getLongValue( env, jfunctor ),
			term0,
			term1,
			term2 );
		break;
	case 4:
		term0 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 0 ) );
		term1 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 1 ) );
		term2 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 2 ) );
		term3 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 3 ) );

		PL_cons_functor( 
			getLongValue( env, jterm ),
			getLongValue( env, jfunctor ),
			term0,
			term1,
			term2,
			term3 );
		break;
	case 5:
		term0 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 0 ) );
		term1 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 1 ) );
		term2 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 2 ) );
		term3 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 3 ) );
		term4 = 
			getLongValue( 
				env, (*env)->GetObjectArrayElement( env, jterms, 4 ) );

		PL_cons_functor( 
			getLongValue( env, jterm ),
			getLongValue( env, jfunctor ),
			term0,
			term1,
			term2,
			term3,
			term4 );
		break;
	default:
		printf( "Unsupported arg list list.  You should not have access to this function anyway.\n" );
	}
}


/*
 * Class:     jpl_fli_PL
 * Method:    cons_functor_v
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/functor_t;Ljpl/fli/term_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_cons_1functor_1v(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jfunctor, 
	jobject     jterm0 )
{
	PL_cons_functor_v( 
		getLongValue( env, jterm ),
		getLongValue( env, jfunctor ),
		getLongValue( env, jterm0 ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    cons_list
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/term_t;Ljpl/fli/term_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_cons_1list(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jlist, 
	jobject     jhead, 
	jobject     jtail )
{
	PL_cons_list( 
		getLongValue( env, jlist ),
		getLongValue( env, jhead ),
		getLongValue( env, jtail ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    unify
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_unify(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jt1, 
	jobject     jt2 )
{
	PL_unify( 
		getLongValue( env, jt1 ),
		getLongValue( env, jt2 ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    open_foreign_frame
 * Signature: ()Ljpl/fli/fid_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_open_1foreign_1frame(
	JNIEnv     *env, 
	jclass      jProlog )
{
	jobject      rval;
	fid_t        value;
	jobject      jfid_t;
	
	value = PL_open_foreign_frame();
	
	/* create the java atom_t and set its value */
	jfid_t       = (*env)->FindClass( env, "jpl/fli/fid_t" );
	rval         = (*env)->AllocObject( env, jfid_t );
	setLongValue( env, rval, (long)value );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    close_foreign_frame
 * Signature: (Ljpl/fli/fid_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_close_1foreign_1frame(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jfid )
{
	PL_close_foreign_frame(
		(fid_t) getLongValue( env, jfid ) );
}

/*
 * Class:     jpl_fli_PL
 * Method:    discard_foreign_frame
 * Signature: (Ljpl/fli/fid_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_discard_1foreign_1frame(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jfid )
{
	PL_discard_foreign_frame(
		(fid_t) getLongValue( env, jfid ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    pred
 * Signature: (Ljpl/fli/functor_t;Ljpl/fli/module_t;)Ljpl/fli/predicate_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_pred(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jfunctor, 
	jobject     jmodule )
{
	jobject      rval;
	predicate_t  value;
	jclass       jpredicate_t;
	
	value = PL_pred( 
		(functor_t) getLongValue( env, jfunctor ),
		(module_t) getPointerValue( env, jmodule ) );
	
	jpredicate_t = (*env)->FindClass( env, "jpl/fli/predicate_t" );
	rval         = (*env)->AllocObject( env, jpredicate_t );
	setPointerValue( env, rval, (pointer)value );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    predicate
 * Signature: (Ljava/lang/String;ILjava/lang/String;)Ljpl/fli/predicate_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_predicate(
	JNIEnv     *env, 
	jclass      jProlog, 
	jstring     jname, 
	jint        jarity, 
	jstring     jmodule )
{
	jobject      rval;
	predicate_t  value;
	const char  *name;
	jclass       jpredicate_t;
	
	name = (*env)->GetStringUTFChars( env, jname, 0 );
	value = PL_predicate( 
		name, 
		(int)jarity, 
		(module_t) getPointerValue( env, jmodule ) );
	(*env)->ReleaseStringUTFChars( env, jname, name );
	
	jpredicate_t = (*env)->FindClass( env, "jpl/fli/predicate_t" );
	rval         = (*env)->AllocObject( env, jpredicate_t );
	setPointerValue( env, rval, (pointer)value );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    predicate_info
 * Signature: (Ljpl/fli/predicate_t;Ljpl/fli/atom_t;ILjpl/fli/module_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_predicate_1info(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jpredicate, 
	jobject     jatom, 
	jobject     jint_holder, 
	jobject     jmodule )
{
	jint rval = JNI_FALSE;
	predicate_t predicate;
	atom_t      atom;
	int         arity;
	module_t    module;
	jclass      jIntHolder;
	
	predicate = (predicate_t) getPointerValue( env, jpredicate );
	rval = PL_predicate_info( predicate, &atom, &arity, &module );

	setLongValue( env, jatom, (long)atom );
	setPointerValue( env, jmodule, (pointer) module );
	setIntValue( env, jint_holder, arity );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    open_query
 * Signature: (Ljpl/fli/module_t;ILjpl/fli/predicate_t;Ljpl/fli/term_t;)Ljpl/fli/qid_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_open_1query(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jmodule, 
	jint        jflags, 
	jobject     jpredicate, 
	jobject     jterm0 )
{
	jobject      rval;
	qid_t        qid;
	jclass       jqid_t;
	
	qid = PL_open_query( 
		(module_t)    getPointerValue( env, jmodule ),
		(int)         jflags,
		(predicate_t) getPointerValue( env, jpredicate ),
		(term_t)      getLongValue( env, jterm0 ) );
	
	jqid_t = (*env)->FindClass( env, "jpl/fli/qid_t" );
	rval   = (*env)->AllocObject( env, jqid_t );
	setLongValue( env, rval, (long)qid );
	
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    next_solution
 * Signature: (Ljpl/fli/qid_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_next_1solution(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jqid )
{
	return PL_next_solution( getLongValue( env, jqid ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    close_query
 * Signature: (Ljpl/fli/qid_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_close_1query(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jqid )
{
	PL_close_query( getLongValue( env, jqid ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    cut_query
 * Signature: (Ljpl/fli/qid_t;)V
 */
JNIEXPORT void JNICALL 
Java_jpl_fli_Prolog_cut_1query(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jqid )
{
	PL_cut_query( getLongValue( env, jqid ) );
}


/*
 * Class:     jpl_fli_PL
 * Method:    call
 * Signature: (Ljpl/fli/term_t;Ljpl/fli/module_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_call(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jterm, 
	jobject     jmodule )
{
	jint rval;
	
	rval = (jint) PL_call(
		(term_t)   getLongValue( env, jterm ),
		(module_t) getPointerValue( env, jmodule ) );
		
	return rval;
}


/*
 * Class:     jpl_fli_PL
 * Method:    call_predicate
 * Signature: (Ljpl/fli/module_t;ILjpl/fli/predicate_t;Ljpl/fli/term_t;)I
 */
JNIEXPORT jint JNICALL 
Java_jpl_fli_Prolog_call_1predicate(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jmodule, 
	jint        jdebug, 
	jobject     jpredicate, 
	jobject     jt0 )
{
	jint rval;
	
	rval = (jint) PL_call_predicate(
		(module_t)    getPointerValue( env, jmodule ),
		(int)         jdebug,
		(predicate_t) getPointerValue( env, jpredicate ),
		(term_t)      getLongValue( env, jt0 ) );
		
	return rval;
	
}


/*
 * Class:     jpl_fli_PL
 * Method:    exception
 * Signature: (Ljpl/fli/qid_t;)Ljpl/fli/term_t;
 */
JNIEXPORT jobject JNICALL 
Java_jpl_fli_Prolog_exception(
	JNIEnv     *env, 
	jclass      jProlog, 
	jobject     jqid )
{
	jobject rval;
	term_t term = PL_exception( getLongValue( env, jqid ) );

	/* create the java term_t and set its value */
	jobject jterm_t = (*env)->FindClass( env, "jpl/fli/term_t" );
	rval = (*env)->AllocObject( env, jterm_t );
	setLongValue( env, rval, (long)term );

	return rval;
}



/*
 * Class:     jpl_fli_PL
 * Method:    initialise
 * Signature: (I[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
Java_jpl_fli_Prolog_initialise(
	JNIEnv      *env,
	jclass       jProlog,
	jint         jargc,
	jobjectArray jargv )
{
	jsize length;
	const char **argv;
	jboolean rval = JNI_TRUE;
	int i;

	length = (*env)->GetArrayLength( env, jargv );
/*printf( "initialize: length=%i\n", length );	*/

	argv = (const char **) malloc( length*sizeof( char * ) + 1 );
	if ( argv == NULL ){
		return JNI_FALSE;
	}


	for ( i = 0;  i < length;  ++i ){
		jstring     jarg;
		jarg = (jstring) (*env)->GetObjectArrayElement( env, jargv, i );
		argv[i] = (*env)->GetStringUTFChars( env, jarg, 0 );
/*printf( "initialize: argv[%i]=%s\n", i, argv[i] );*/
	}
	argv[length] = NULL;

	if ( ! PL_initialise( length, (char **)argv ) ){
		PL_halt( 1 );
		rval = JNI_FALSE;
	}

	for ( i = 0;  i < length;  ++i ){
		jstring     jarg;
		jarg = (jstring) (*env)->GetObjectArrayElement( env, jargv, i );
		(*env)->ReleaseStringUTFChars( env, jarg, argv[i] );
	}

	free( argv );

 	return rval;
}

/*
 * Class:     jpl_fli_PL
 * Method:    halt
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_jpl_fli_Prolog_halt(
	JNIEnv     *env,
	jclass      jProlog,
	jint        jstatus )
{
	PL_halt( (int) jstatus );
}


/*
 * Class:     jpl_fli_Prolog
 * Method:    registerSecondo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_jpl_fli_Prolog_registerSecondo(JNIEnv * end , jclass cl){
   return registerSecondo();
}



