/**
 * WebTester Server - server of on-line testing system
 *
 * Errors' codes
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _errors_h_
#define _errors_h_

#define ERR_OK                   0x00000000
#define PE_BROKEN_STRING         0x00000001
#define PE_UNEXCEPTED_CHAR       0x00000002
#define PE_UNEXCEPTED_EOF        0x00000004
#define PE_INVALID_ECRAN         0x00000008
#define PE_UNKNOWN_TOKEN         0x00000010
#define PE_INVALID_NAMED_STRING  0x00000020

#define PF_ERROR                 0x00000001
#define PF_NOPARSE               0x00000002
#define PF_NAMED_STRING          0x00000004

#endif
