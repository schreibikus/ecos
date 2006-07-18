/* =================================================================
 *
 *      handler.c
 *
 *      C callbacks as well as directory listings.
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####
 * -------------------------------------------
 * This file is part of eCos, the Embedded Configurable Operating
 * System.
 * Copyright (C) 2005 eCosCentric Ltd.
 * 
 * eCos is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 or (at your option)
 * any later version.
 * 
 * eCos is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with eCos; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * As a special exception, if other files instantiate templates or
 * use macros or inline functions from this file, or you compile this
 * file and link it with other works to produce a work based on this
 * file, this file does not by itself cause the resulting work to be
 * covered by the GNU General Public License. However the source code
 * for this file must still be made available in accordance with
 * section (3) of the GNU General Public License.
 * 
 * This exception does not invalidate any other reasons why a work
 * based on this file might be covered by the GNU General Public
 * License.
 *
 * -------------------------------------------
 * ####ECOSGPLCOPYRIGHTEND####
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    Anthony Tonizzo (atonizzo@gmail.com)
 *  Contributors: 
 *  Date:         2006-06-12
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/hal/hal_tables.h>
#include <cyg/fileio/fileio.h>
#include <dirent.h>
#include <network.h>

#include <string.h>
#include <stdio.h>                     // sprintf().

#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>
#include <cyg/athttpd/handler.h>
#include <cyg/athttpd/forms.h>

#ifdef CYGOPT_NET_ATHTTPD_USE_DIRLIST
char folder_gif[] = {
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x12, 0x00,
    0x12, 0x00, 0xd5, 0x00, 0x00, 0xfb, 0xfb, 0xfb,
    0xef, 0xef, 0xef, 0xdb, 0xb7, 0x52, 0xcc, 0x99,
    0x34, 0xe4, 0xe4, 0xe4, 0xc1, 0xc1, 0xc1, 0xa3,
    0x71, 0x0b, 0xc2, 0x8f, 0x2a, 0xb7, 0xb7, 0xb7,
    0x9e, 0x6c, 0x06, 0xba, 0x87, 0x22, 0x9c, 0x6a,
    0x04, 0xc9, 0x96, 0x31, 0xbd, 0x8a, 0x25, 0xb3,
    0x81, 0x1b, 0x99, 0x67, 0x01, 0xdb, 0xb7, 0x71,
    0xb5, 0x82, 0x1d, 0xa0, 0x6e, 0x08, 0xcb, 0x98,
    0x33, 0xff, 0xff, 0xba, 0xb0, 0x7e, 0x18, 0xc5,
    0x92, 0x2d, 0xa7, 0x75, 0x0f, 0xc7, 0x94, 0x2f,
    0xa6, 0xa6, 0xa6, 0xa5, 0x73, 0x0d, 0xb7, 0x84,
    0x1f, 0xab, 0x79, 0x13, 0xc8, 0x95, 0x30, 0xc0,
    0x8d, 0x28, 0xc6, 0xc6, 0xc6, 0xae, 0x7c, 0x16,
    0x9a, 0x68, 0x02, 0xa8, 0x76, 0x10, 0xac, 0x7a,
    0x14, 0xff, 0xd4, 0x6f, 0xff, 0xf8, 0x93, 0xeb,
    0xeb, 0xeb, 0xff, 0xff, 0x9c, 0xff, 0xdb, 0x75,
    0xff, 0xe4, 0x7f, 0xff, 0xef, 0x89, 0xb6, 0xb6,
    0xb6, 0xff, 0xff, 0xff, 0xff, 0xff, 0x99, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xf9, 0x04,
    0x01, 0x00, 0x00, 0x2e, 0x00, 0x2c, 0x00, 0x00,
    0x00, 0x00, 0x12, 0x00, 0x12, 0x00, 0x00, 0x06,
    0x91, 0x40, 0x97, 0x70, 0x48, 0x2c, 0x1a, 0x8f,
    0xc8, 0xa4, 0xd2, 0x35, 0x18, 0x4c, 0x18, 0x9d,
    0x00, 0x40, 0x39, 0x60, 0x59, 0x29, 0x94, 0x03,
    0x61, 0x7a, 0x1c, 0x40, 0x04, 0x60, 0x30, 0xa4,
    0xa1, 0xd8, 0x44, 0x1c, 0x95, 0xc0, 0x70, 0x62,
    0x6d, 0xbb, 0xdb, 0xa3, 0xcf, 0x90, 0xd1, 0x3a,
    0xd9, 0xef, 0xf8, 0xd6, 0x05, 0x31, 0xc4, 0xb4,
    0xfe, 0x80, 0x81, 0x7f, 0x06, 0x2b, 0x43, 0x16,
    0x2d, 0x25, 0x89, 0x8a, 0x8b, 0x2d, 0x12, 0x85,
    0x42, 0x07, 0x2d, 0x2a, 0x93, 0x94, 0x95, 0x2d,
    0x09, 0x8f, 0x2e, 0x1e, 0x2d, 0x29, 0x9d, 0x9e,
    0x9f, 0x2d, 0x0b, 0x99, 0x0d, 0x2d, 0x28, 0xa6,
    0xa7, 0xa8, 0x2d, 0x21, 0x99, 0x0a, 0x2d, 0x24,
    0xaf, 0xb0, 0xb1, 0x2d, 0x0f, 0x7c, 0x42, 0x01,
    0x67, 0x15, 0x20, 0x1c, 0x22, 0x1a, 0x06, 0x12,
    0x09, 0x0b, 0x0f, 0x19, 0x05, 0x43, 0x00, 0x04,
    0x05, 0x2b, 0xc9, 0xca, 0xcb, 0x05, 0x04, 0x44,
    0x00, 0x01, 0x26, 0xd2, 0xd3, 0xd4, 0x52, 0x4b,
    0x4a, 0x41, 0x00, 0x3b
};

char doc_gif[] = {
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x12, 0x00,
    0x12, 0x00, 0xe6, 0x00, 0x00, 0xfb, 0xfb, 0xfb,
    0xef, 0xef, 0xef, 0xf8, 0xfb, 0xff, 0xed, 0xf6,
    0xff, 0xc1, 0xc1, 0xc1, 0xe4, 0xe4, 0xe4, 0xfd,
    0xfd, 0xfd, 0xd8, 0xeb, 0xff, 0xc0, 0xdf, 0xff,
    0xfe, 0xff, 0xff, 0xf3, 0xf9, 0xff, 0xc3, 0xe1,
    0xff, 0xf4, 0xf4, 0xf5, 0xf9, 0xf9, 0xf9, 0x81,
    0x95, 0xc3, 0x6b, 0x74, 0xa5, 0xce, 0xe7, 0xff,
    0x5e, 0x61, 0x90, 0x55, 0xa7, 0xed, 0x93, 0xaa,
    0xd2, 0x7a, 0x83, 0xb0, 0xc0, 0xc0, 0xff, 0xd9,
    0xe1, 0xed, 0x84, 0x9b, 0xca, 0x8a, 0xa8, 0xd9,
    0xe4, 0xf1, 0xff, 0x91, 0xb8, 0xea, 0x92, 0xbc,
    0xef, 0x73, 0x7e, 0xb0, 0xf5, 0xfa, 0xff, 0xf7,
    0xf7, 0xf7, 0x90, 0xa3, 0xcb, 0xcd, 0xe6, 0xff,
    0xba, 0xdd, 0xff, 0xd8, 0xe5, 0xf6, 0x6d, 0x77,
    0xa8, 0x65, 0x6a, 0x9a, 0x87, 0xa1, 0xd2, 0x7f,
    0x8e, 0xbc, 0x8f, 0xb3, 0xe5, 0x8c, 0xae, 0xdf,
    0xce, 0xd3, 0xff, 0xf6, 0xfa, 0xff, 0x67, 0x6d,
    0x9e, 0xfc, 0xfd, 0xff, 0x7d, 0x89, 0xb6, 0x94,
    0xbf, 0xf2, 0xc6, 0xc6, 0xc6, 0xfe, 0xfe, 0xfe,
    0xdb, 0xe7, 0xf7, 0x69, 0x70, 0xa2, 0x6f, 0x7a,
    0xac, 0x72, 0x7c, 0xae, 0x74, 0x80, 0xb2, 0x87,
    0xa9, 0xdc, 0xd1, 0xe9, 0xff, 0xc9, 0xe4, 0xff,
    0xb7, 0xb7, 0xb7, 0xeb, 0xeb, 0xeb, 0xb6, 0xb6,
    0xb6, 0xe9, 0xf5, 0xff, 0x75, 0x78, 0xa3, 0xe1,
    0xf0, 0xff, 0xdb, 0xed, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xf9, 0x04,
    0x01, 0x00, 0x00, 0x41, 0x00, 0x2c, 0x00, 0x00,
    0x00, 0x00, 0x12, 0x00, 0x12, 0x00, 0x00, 0x07,
    0xa3, 0x80, 0x41, 0x82, 0x83, 0x84, 0x85, 0x86,
    0x87, 0x88, 0x2e, 0x36, 0x8b, 0x8c, 0x36, 0x35,
    0x01, 0x00, 0x87, 0x1b, 0x40, 0x06, 0x00, 0x0d,
    0x1e, 0x0c, 0x16, 0x35, 0x13, 0x90, 0x86, 0x1a,
    0x30, 0x40, 0xa1, 0x09, 0x02, 0x31, 0x35, 0x12,
    0x1f, 0x01, 0x86, 0x27, 0x06, 0xa2, 0x02, 0x1d,
    0x22, 0x35, 0xb1, 0x2f, 0x86, 0x28, 0xa1, 0x02,
    0x02, 0x03, 0x3c, 0x3c, 0x29, 0x15, 0x1c, 0x39,
    0x86, 0x18, 0x2c, 0xb8, 0xb9, 0x3c, 0x3f, 0x3f,
    0x07, 0x34, 0xbf, 0x85, 0x25, 0x2a, 0x0a, 0xb9,
    0x3e, 0x3e, 0xc7, 0x37, 0x33, 0xca, 0x84, 0x17,
    0xce, 0x3c, 0xd0, 0xc6, 0x37, 0x20, 0x23, 0xd5,
    0x83, 0x0e, 0xba, 0xda, 0x3f, 0x37, 0x38, 0x38,
    0x0f, 0xdf, 0x82, 0x26, 0xd9, 0xd1, 0xe4, 0xe6,
    0x0b, 0x32, 0xe9, 0x41, 0x2d, 0x19, 0xed, 0x37,
    0xe5, 0x0b, 0x08, 0x2b, 0xf2, 0x14, 0xf6, 0x10,
    0xe6, 0x08, 0x42, 0x90, 0x90, 0xd7, 0xa3, 0xa0,
    0xc1, 0x83, 0x11, 0x08, 0x18, 0x2a, 0x40, 0x60,
    0x87, 0xc3, 0x87, 0x0e, 0x09, 0x14, 0x30, 0x04,
    0x20, 0x80, 0x8e, 0x8b, 0x18, 0x2f, 0x76, 0x42,
    0xc4, 0xd1, 0x50, 0x20, 0x00, 0x3b
};

char back_gif[] = {
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x0a, 0x00,
    0x09, 0x00, 0xf7, 0x00, 0x00, 0xf8, 0xfa, 0xfb,
    0x3a, 0x6b, 0x9d, 0xe5, 0xeb, 0xf2, 0x87, 0xa5,
    0xc3, 0x9d, 0xb5, 0xce, 0xd0, 0xdc, 0xe7, 0xa9,
    0xbe, 0xd4, 0x74, 0x97, 0xb9, 0x65, 0x8c, 0xb2,
    0xd9, 0xe2, 0xec, 0xf1, 0xf4, 0xf8, 0xe7, 0xed,
    0xf3, 0x55, 0x7f, 0xaa, 0xc8, 0xd6, 0xe3, 0xcc,
    0xcc, 0xcc, 0x91, 0xac, 0xc8, 0x3f, 0x6f, 0x9f,
    0xfd, 0xfd, 0xfe, 0x4d, 0x7a, 0xa6, 0xac, 0xc1,
    0xd5, 0x4b, 0x78, 0xa5, 0x33, 0x66, 0x99, 0xff,
    0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
    0x00, 0x00, 0x0a, 0x00, 0x09, 0x00, 0x00, 0x08,
    0x3b, 0x00, 0x2d, 0x08, 0x1c, 0x68, 0x61, 0xc1,
    0x83, 0x02, 0x04, 0x2d, 0x00, 0x20, 0x10, 0x80,
    0xc2, 0x04, 0x82, 0x13, 0x28, 0x54, 0xa8, 0xe0,
    0x50, 0x60, 0x02, 0x04, 0x13, 0x27, 0x56, 0x04,
    0x70, 0x20, 0xa3, 0xc6, 0x87, 0x02, 0x1b, 0x30,
    0xc8, 0x58, 0x71, 0x60, 0x04, 0x03, 0x10, 0x28,
    0x82, 0x24, 0xa8, 0x60, 0x80, 0x84, 0x95, 0x09,
    0x05, 0x38, 0x08, 0x08, 0x00, 0x3b
};
#endif

// =============================================================================
// Internal resources
// =============================================================================
CYG_HAL_TABLE_BEGIN(cyg_httpd_ires_table, httpd_ires_table);
CYG_HAL_TABLE_END(cyg_httpd_ires_table_end, httpd_ires_table);
__externC cyg_httpd_ires_table_entry cyg_httpd_ires_table[];
__externC cyg_httpd_ires_table_entry cyg_httpd_ires_table_end[];

#ifdef CYGOPT_NET_ATHTTPD_USE_DIRLIST
// These threes internal resources are used for the directory listing.
CYG_HTTPD_IRES_TABLE_ENTRY(cyg_httpd_ires_folder, \
                           "/___folder__gif.gif", \
                           folder_gif,            \
                           sizeof(folder_gif));
CYG_HTTPD_IRES_TABLE_ENTRY(cyg_httpd_ires_doc,    \
                           "/___doc__gif.gif",    \
                           doc_gif,               \
                           sizeof(doc_gif));
CYG_HTTPD_IRES_TABLE_ENTRY(cyg_httpd_ires_back,   \
                           "/___back__gif.gif",   \
                           back_gif,              \
                           sizeof(back_gif));
#endif                           

cyg_httpd_ires_table_entry *
cyg_httpd_find_ires(char *path_name)
{
    cyg_httpd_ires_table_entry *entry = cyg_httpd_ires_table;
    
    while (entry != cyg_httpd_ires_table_end)
    {
        if (!strcasecmp((char*)path_name, entry->f_pname))
            return entry;
        entry++;
    }
            
    return (cyg_httpd_ires_table_entry*)NULL;
}

void 
cyg_httpd_send_ires(cyg_httpd_ires_table_entry *entry)
{
    httpstate.status_code  = CYG_HTTPD_STATUS_OK;

    // Here we'll look for extension to the file. Consider the case where
    //  there might be more than one dot in the file name. We'll look for
    //  the last dot, then we'll check the extension.
    char *extension = rindex(entry->f_pname, '.');
    if (extension == NULL)
        // No extension in the file name.
        httpstate.mime_type = 0;
    else
        httpstate.mime_type = cyg_httpd_find_mime_string(++extension);

    httpstate.payload_len  = entry->f_size;
    cyg_int32 header_length = cyg_httpd_format_header();
    
    // As always, the header is always sent out.
    send(httpstate.sockets[httpstate.client_index].descriptor, 
         httpstate.outbuffer, 
         header_length,
         0);
    if (httpstate.mode & CYG_HTTPD_SEND_HEADER_ONLY) 
        return;
    cyg_httpd_write(entry->f_ptr, entry->f_size);
}

// =============================================================================
// C callbacks
// =============================================================================
CYG_HAL_TABLE_BEGIN(cyg_httpd_handler_table, httpd_handler_table);
CYG_HAL_TABLE_END(cyg_httpd_handler_table_end, httpd_handler_table);

__externC cyg_httpd_handler_table_entry cyg_httpd_handler_table[];
__externC cyg_httpd_handler_table_entry cyg_httpd_handler_table_end[];

handler
cyg_httpd_find_handler(void)
{
    cyg_httpd_handler_table_entry *entry = cyg_httpd_handler_table;
    while (entry != cyg_httpd_handler_table_end)
    {
        if (!strcmp((const char*)httpstate.url, entry->path))
            return entry->h;
        entry++;
    }
            
    return (handler)NULL;
}

// =============================================================================
// Directory listing
// =============================================================================
#ifdef CYGOPT_NET_ATHTTPD_USE_DIRLIST
void
cyg_httpd_print_directory_entry(char *path_name, 
                                struct stat *s, 
                                struct dirent *de)
{
    char fname[CYG_HTTPD_MAXPATH];
    char time_str[32];
    time_t last_mod = s->st_mtime;

    struct tm *mod_time = gmtime(&last_mod);
    sprintf(time_str,
            "%02d-%s-%04d %02d:%02d",
            mod_time->tm_mday,
            month_of_year[mod_time->tm_mon],
            mod_time->tm_year + 1900,
            mod_time->tm_hour,
            mod_time->tm_min);

    if (S_ISREG(s->st_mode))
    {
        strcpy(fname, path_name);
        if (fname[strlen(fname)-1] != '/')
            strcat(fname, "/");
        strcat(fname, de->d_name);
        cyg_httpd_cleanup_filename(fname);
        sprintf(httpstate.outbuffer,
                "<tr><td><img src='/___doc__gif.gif'></td>"
                "<td><a href='%s'>%s</a></td>"
                "<td>&nbsp &nbsp &nbsp &nbsp &nbsp &nbsp</td>"
                "<td>%s&nbsp;&nbsp;</td>"
                "<td style='color:black'>%d</td></tr>",
                fname,
                de->d_name,
                time_str,
                (int)s->st_size);
    }
    else
    {
        strcpy(fname, "/");
        strcat(fname, path_name);
        if (fname[strlen(fname)-1] != '/')
            strcat(fname, "/");
        strcat(fname, de->d_name);
        cyg_httpd_cleanup_filename(fname);
        sprintf(httpstate.outbuffer,
                "<tr><td><img src='/___folder__gif.gif'></td>"
                "<td><a href='%s/'>%s</a></td>"
                "<td>&nbsp &nbsp &nbsp &nbsp &nbsp &nbsp</td>"
                "<td>%s&nbsp;&nbsp;</td>"
                "<td style='color:black'></td></tr>",
                fname,
                de->d_name,
                time_str);
    }
    cyg_httpd_write_chunked(httpstate.outbuffer, strlen(httpstate.outbuffer));
}

void
cyg_httpd_send_directory_listing(char *path_name)
{
    struct dirent next_file;
    struct dirent last_printed;
    struct stat s;
    char fname[CYG_HTTPD_MAXPATH];

    cyg_httpd_start_chunked("html");
    sprintf(httpstate.outbuffer,
            "<html><body><h3>Index of %s</h3><p></p>"
            "<table style='font-family:courier'>",
            path_name);
    cyg_httpd_write_chunked(httpstate.outbuffer, strlen(httpstate.outbuffer));

    sprintf(httpstate.outbuffer,
            "<tr><td></td><td align='center' style='border-style:none none"
            " solid none;border-width:thin'>Name</td>"
            "<td style='border-style:none none solid none; border-width:thin'>"
            "&nbsp &nbsp &nbsp &nbsp &nbsp &nbsp</td>"
             "<td  align='center' style='border-style:none none solid none;"
             " border-width:thin'>Last Modified</td>"
             "<td style='border-style:none none solid none; border-width:thin'>"
            "Size</td></tr>");
    cyg_httpd_write_chunked(httpstate.outbuffer, strlen(httpstate.outbuffer));

    // Provide a pointer to the parent directory.
    strcpy(fname, path_name);
    if (fname[strlen(fname)-1] == '/')
        fname[strlen(fname)-1] = '\0';
    char *slash = rindex(fname, '/');
    slash[1] = '\0';
    sprintf(httpstate.outbuffer,
            "<tr><td><img src='/___back__gif.gif'></td></td>"
            "<td><a href='%s' style='color:red'>Parent "
            "Directory</a></td>"
            "<td>&nbsp &nbsp &nbsp &nbsp &nbsp &nbsp</td>"
            "<td>&nbsp &nbsp &nbsp &nbsp &nbsp &nbsp</td>"
            "<td style='color:black'>-</td></tr>",
            fname);
    cyg_httpd_write_chunked(httpstate.outbuffer, strlen(httpstate.outbuffer));

    strcpy(fname, CYGDAT_NET_ATHTTPD_SERVEROPT_ROOTDIR);
    if (fname[strlen(fname)-1] != '/')
        strcat(fname, "/");
    strcat(fname, path_name);
    if (fname[strlen(fname)-1] != '/')
        strcat(fname, "/");
    cyg_httpd_cleanup_filename(fname);
    int abs_path_len = strlen(fname);

    DIR* pdir = opendir(fname);
    struct dirent *dirent = readdir(pdir);

    // First time for the directories.
    rewinddir(pdir);
    memset((void*)&last_printed, 0, sizeof(struct dirent));
    dirent = readdir(pdir);
    int new_entry = 1;
    while (1)
    {
        memset((void*)&next_file, 0, sizeof(struct dirent));
        new_entry = 0;
        while(dirent)
        {
            if (strcmp(dirent->d_name, "..") &&
                                     strcmp(dirent->d_name, "."))
                if ((next_file.d_name[0] == '\0') ||
                      (strcmp(next_file.d_name, dirent->d_name) > 0))
                    if ((last_printed.d_name[0] == '\0') ||
                      (strcmp(last_printed.d_name, dirent->d_name) < 0))
                    {
                        fname[abs_path_len] = '\0';
                        strcat(fname, dirent->d_name);
                        struct stat sp;
                        stat(fname, &sp);
                        if (S_ISDIR(sp.st_mode))
                        {
                            memcpy(&next_file, dirent, sizeof(struct dirent));
                            memcpy(&s, &sp, sizeof(struct stat));
                            new_entry = 1;
                        }
                    }
            dirent = readdir(pdir);
        }
        if (new_entry == 0)
            break;
        cyg_httpd_print_directory_entry(path_name, &s, &next_file);
        memcpy(&last_printed, &next_file, sizeof(struct dirent));
        rewinddir(pdir);
        dirent = readdir(pdir);
    }

    // A second time for the files.
    rewinddir(pdir);
    memset((void*)&last_printed, 0, sizeof(struct dirent));
    dirent = readdir(pdir);
    new_entry = 1;
    while (1)
    {
        memset((void*)&next_file, 0, sizeof(struct dirent));
        new_entry = 0;
        while(dirent)
        {
            if (strcmp(dirent->d_name, "..") &&
                                     strcmp(dirent->d_name, "."))
                if ((next_file.d_name[0] == '\0') ||
                      (strcmp(next_file.d_name, dirent->d_name) > 0))
                    if ((last_printed.d_name[0] == '\0') ||
                      (strcmp(last_printed.d_name, dirent->d_name) < 0))
                    {
                        fname[abs_path_len] = '\0';
                        strcat(fname, dirent->d_name);
                        struct stat sp;
                        stat(fname, &sp);
                        if (S_ISREG(sp.st_mode))
                        {
                            memcpy(&next_file, dirent, sizeof(struct dirent));
                            memcpy(&s, &sp, sizeof(struct stat));
                            new_entry = 1;
                        }
                    }
            dirent = readdir(pdir);
        }
        if (new_entry == 0)
            break;
        cyg_httpd_print_directory_entry(path_name, &s, &next_file);
        memcpy(&last_printed, &next_file, sizeof(struct dirent));
        rewinddir(pdir);
        dirent = readdir(pdir);
    }

    strcpy(httpstate.outbuffer, "</table><html><body>");
    cyg_httpd_write_chunked(httpstate.outbuffer, strlen(httpstate.outbuffer));
    cyg_httpd_end_chunked();
    closedir(pdir);
}
#endif
