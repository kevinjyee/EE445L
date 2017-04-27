/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file  R0.04a   (C)ChaN, 2007
/-----------------------------------------------------------------------
 * Modified by Jonathan Valvano to simplify usage in Lab 5
 * March 17, 2014
 */


#define _READONLY	0	/* 1: Read-only mode */

typedef signed int		INT;
typedef unsigned int	UINT;

/* These types are assumed as 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types are assumed as 16-bit integer */
typedef signed short	SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;

/* These types are assumed as 32-bit integer */
typedef signed long		LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

/* Boolean type */
typedef enum { FALSE = 0, TRUE } BOOL;

/* Status of Disk Functions */
typedef BYTE	DSTATUS;




/*---------------------------------------*/
/* Prototypes for disk control functions */

//*************** eDisk_Init ***********
// Initialize the interface between microcontroller and the SD card
// Inputs: drive number (only drive 0 is supported)
// Outputs: status
//  STA_NOINIT   0x01   Drive not initialized
//  STA_NODISK   0x02   No medium in the drive
//  STA_PROTECT  0x04   Write protected
// since this program initializes the disk, it must run with 
//    the disk periodic task operating
DSTATUS eDisk_Init(BYTE drive);
//*************** eDisk_Status ***********
// Check the status of the SD card
// Inputs: drive number (only drive 0 is supported)
// Outputs: status
//  STA_NOINIT   0x01   Drive not initialized
//  STA_NODISK   0x02   No medium in the drive
//  STA_PROTECT  0x04   Write protected
DSTATUS eDisk_Status (BYTE drive);

