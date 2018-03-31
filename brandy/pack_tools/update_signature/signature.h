/*
 * (C) Copyright 2012
 *     wangflord@allwinnertech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */

#ifndef  __SIGNATURE_H__
#define  __SIGNATURE_H__

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

//#define  DEBUG_MODE
#undef  DEBUG_MODE

#ifdef  DEBUG_MODE
#define debug(fmt,args...)	printf(fmt ,##args)
#else
#define debug(fmt,args...)
#endif


#endif
