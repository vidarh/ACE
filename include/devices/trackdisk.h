#ifndef DEVICES_TRACKDISK_H
#define DEVICES_TRACKDISK_H 1
/*
** trackdisk.h for ACE Basic
**
** Note: Translated to ACE by ConvertC2ACE
**       @ MapMeadow Software, Nils Sjoholm
**
**
** Date: 09/03/95
**
**
*/


/*
** This are the StructPointer defines for trackdisk.h
*/
#ifndef DriveGeometryPtr
#define DriveGeometryPtr ADDRESS
#endif
#ifndef IOExtTDPtr
#define IOExtTDPtr ADDRESS
#endif
#ifndef TDU_PublicUnitPtr
#define TDU_PublicUnitPtr ADDRESS
#endif
/*
** End of StructPointer defines for trackdisk.h
*/
        
#ifndef EXEC_IO_H
#include <exec/io.h>
#endif

#ifndef EXEC_DEVICES_H
#include <exec/devices.h>
#endif


/*
 *--------------------------------------------------------------------
 *
 * Physical drive constants
 *
 *--------------------------------------------------------------------
 */

/* OBSOLETE -- use the TD_GETNUMTRACKS command! */
/*#define   NUMCYLS 80*/        /*  normal # of cylinders */
/*#define   MAXCYLS (NUMCYLS+20)*/  /* max # cyls to look for during cal */
/*#define   NUMHEADS 2*/
/*#define   NUMTRACKS (NUMCYLS*NUMHEADS)*/

#define NUMSECS 11
#define NUMUNITS 4

/*
 *--------------------------------------------------------------------
 *
 * Useful constants
 *
 *--------------------------------------------------------------------
 */

/*-- sizes before mfm encoding */
#define TD_SECTOR 512
#define TD_SECSHIFT 9       /* log TD_SECTOR */

/*
 *--------------------------------------------------------------------
 *
 * Driver Specific Commands
 *
 *--------------------------------------------------------------------
 */

/*
 *-- TD_NAME is a generic macro to get the name of the driver.  This
 *-- way if the name is ever changed you will pick up the change
 *-- automatically.
 *--
 *-- Normal usage would be:
 *--
 *-- BYTE internalName[] = TD_NAME 
 *--
 */

#define TD_NAME "trackdisk.device"

#define TDF_EXTCOM (32768)      /* for internal use only! */


#define TD_MOTOR    (CMD_NONSTD+0)  /* control the disk's motor */
#define TD_SEEK     (CMD_NONSTD+1)  /* explicit seek (for testing) */
#define TD_FORMAT   (CMD_NONSTD+2)  /* format disk */
#define TD_REMOVE   (CMD_NONSTD+3)  /* notify when disk changes */
#define TD_CHANGENUM    (CMD_NONSTD+4)  /* number of disk changes */
#define TD_CHANGESTATE  (CMD_NONSTD+5)  /* is there a disk in the drive? */
#define TD_PROTSTATUS   (CMD_NONSTD+6)  /* is the disk write protected? */
#define TD_RAWREAD  (CMD_NONSTD+7)  /* read raw bits from the disk */
#define TD_RAWWRITE (CMD_NONSTD+8)  /* write raw bits to the disk */
#define TD_GETDRIVETYPE (CMD_NONSTD+9)  /* get the type of the disk drive */
#define TD_GETNUMTRACKS (CMD_NONSTD+10) /* # of tracks for this type drive */
#define TD_ADDCHANGEINT (CMD_NONSTD+11) /* TD_REMOVE done right */
#define TD_REMCHANGEINT (CMD_NONSTD+12) /* remove softint set by ADDCHANGEINT */
#define TD_GETGEOMETRY  (CMD_NONSTD+13) /* gets the disk geometry table */
#define TD_EJECT    (CMD_NONSTD+14) /* for those drives that support it */
#define TD_LASTCOMM (CMD_NONSTD+15)

/*
 *
 * The disk driver has an "extended command" facility.  These commands
 * take a superset of the normal IO Request block.
 *
 */

#define ETD_WRITE   (CMD_WRITE OR TDF_EXTCOM)
#define ETD_READ    (CMD_READ OR TDF_EXTCOM)
#define ETD_MOTOR   (TD_MOTOR OR TDF_EXTCOM)
#define ETD_SEEK    (TD_SEEK OR TDF_EXTCOM)
#define ETD_FORMAT  (TD_FORMAT OR TDF_EXTCOM)
#define ETD_UPDATE  (CMD_UPDATE OR TDF_EXTCOM)
#define ETD_CLEAR   (CMD_CLEAR OR TDF_EXTCOM)
#define ETD_RAWREAD (TD_RAWREAD OR TDF_EXTCOM)
#define ETD_RAWWRITE    (TD_RAWWRITE OR TDF_EXTCOM)

/*
 *
 * extended IO has a larger than normal io request block.
 *
 */

STRUCT IOExtTD  
    IOStdReq iotd_Req 
    LONGINT   iotd_Count 
    LONGINT   iotd_SecLabel 
END STRUCT 

/*
 *  This is the structure returned by TD_DRIVEGEOMETRY
 *  Note that the layout can be defined three ways:
 *
 *  1. TotalSectors
 *  2. Cylinders and CylSectors
 *  3. Cylinders,  Heads,  and TrackSectors.
 *
 *  #1 is most accurate,  #2 is less so,  and #3 is least accurate.  All
 *  are usable,  though #2 and #3 may waste some portion of the available
 *  space on some drives.
 */
STRUCT DriveGeometry  
    LONGINT   dg_SectorSize       /* in bytes */
    LONGINT   dg_TotalSectors     /* total # of sectors on drive */
    LONGINT   dg_Cylinders        /* number of cylinders */
    LONGINT   dg_CylSectors       /* number of sectors/cylinder */
    LONGINT   dg_Heads        /* number of surfaces */
    LONGINT   dg_TrackSectors     /* number of sectors/track */
    LONGINT   dg_BufMemType       /* preferred buffer memory type */
                    /* (usually MEMF_PUBLIC) */
    BYTE    dg_DeviceType       /* codes as defined in the SCSI-2 spec*/
    BYTE    dg_Flags        /* flags,  including removable */
    SHORTINT   dg_Reserved 
END STRUCT 

/* device types */
#define DG_DIRECT_ACCESS    0
#define DG_SEQUENTIAL_ACCESS    1
#define DG_PRINTER      2
#define DG_PROCESSOR        3
#define DG_WORM         4
#define DG_CDROM        5
#define DG_SCANNER      6
#define DG_OPTICAL_DISK     7
#define DG_MEDIUM_CHANGER   8
#define DG_COMMUNICATION    9
#define DG_UNKNOWN      31

/* flags */
#define DGB_REMOVABLE       0
#define DGF_REMOVABLE       1

/*
** raw read and write can be synced with the index pulse.  This flag
** in io request's IO_FLAGS field tells the driver that you want this.
*/

#define IOTDB_INDEXSYNC 4
#define IOTDF_INDEXSYNC (16)
/*
** raw read and write can be synced with a $4489 sync pattern.  This flag
** in io request's IO_FLAGS field tells the driver that you want this.
*/
#define IOTDB_WORDSYNC  5
#define IOTDF_WORDSYNC (32)


/* labels are TD_LABELSIZE bytes per sector */

#define TD_LABELSIZE 16

/*
** This is a bit in the FLAGS field of OpenDevice.  If it is set,  then
** the driver will allow you to open all the disks that the trackdisk
** driver understands.  Otherwise only 3.5" disks will succeed.
*/

#define TDB_ALLOW_NON_3_5   0
#define TDF_ALLOW_NON_3_5   (1)

/*
**  If you set the TDB_ALLOW_NON_3_5 bit in OpenDevice,  then you don't
**  know what type of disk you really got.  These defines are for the
**  TD_GETDRIVETYPE command.  In addition,  you can find out how many
**  tracks are supported via the TD_GETNUMTRACKS command.
*/

#define DRIVE3_5    1
#define DRIVE5_25   2
#define DRIVE3_5_150RPM 3

/*
 *--------------------------------------------------------------------
 *
 * Driver error defines
 *
 *--------------------------------------------------------------------
 */

#define TDERR_NotSpecified  20  /* general catchall */
#define TDERR_NoSecHdr      21  /* couldn't even find a sector */
#define TDERR_BadSecPreamble    22  /* sector looked wrong */
#define TDERR_BadSecID      23  /* ditto */
#define TDERR_BadHdrSum     24  /* header had incorrect checksum */
#define TDERR_BadSecSum     25  /* data had incorrect checksum */
#define TDERR_TooFewSecs    26  /* couldn't find enough sectors */
#define TDERR_BadSecHdr     27  /* another "sector looked wrong" */
#define TDERR_WriteProt     28  /* can't write to a protected disk */
#define TDERR_DiskChanged   29  /* no disk in the drive */
#define TDERR_SeekError     30  /* couldn't find track 0 */
#define TDERR_NoMem     31  /* ran out of memory */
#define TDERR_BadUnitNum    32  /* asked for a unit > NUMUNITS */
#define TDERR_BadDriveType  33  /* not a drive that trackdisk groks */
#define TDERR_DriveInUse    34  /* someone else allocated the drive */
#define TDERR_PostReset     35  /* user hit reset  awaiting doom */

/*
 *--------------------------------------------------------------------
 *
 * public portion of the unit structure
 *
 *--------------------------------------------------------------------
 */

STRUCT TDU_PublicUnit  
    Unit tdu_Unit       /* base message port */
    SHORTINT   tdu_Comp01Track     /* track for first precomp */
    SHORTINT   tdu_Comp10Track     /* track for second precomp */
    SHORTINT   tdu_Comp11Track     /* track for third precomp */
    LONGINT   tdu_StepDelay       /* time to wait after stepping */
    LONGINT   tdu_SettleDelay     /* time to wait after seeking */
    BYTE    tdu_RetryCnt        /* # of times to retry */
    BYTE    tdu_PubFlags        /* public flags,  see below */
    SHORTINT   tdu_CurrTrk         /* track the heads are over... */
                    /* ONLY ACCESS WHILE UNIT IS STOPPED! */
    LONGINT   tdu_CalibrateDelay  /* time to wait after stepping */
                    /* during a recalibrate */
    LONGINT   tdu_Counter         /* counter for disk changes... */
                    /* ONLY ACCESS WHILE UNIT IS STOPPED! */
END STRUCT 

/* flags for tdu_PubFlags */
#define TDPB_NOCLICK    0
#define TDPF_NOCLICK    (1)

#endif  /* DEVICES_TRACKDISK_H */
