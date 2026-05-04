	.z80

;-----------------------------------------------------------------------------
;
; Partition table layout, partition type codes, and FAT/filesystem
; constants used when manipulating MBR-partitioned FAT volumes.
;
;-----------------------------------------------------------------------------

	INCLUDE ../macros/const.inc

;-----------------------------------------------------------------------------
;
; MBR layout
;

;Start of the partition table within the MBR.
	const	MBR_PSTART, 01BEh

;Size of one partition-table entry, in bytes.
	const	MBR_PSIZE,  16


;-----------------------------------------------------------------------------
;
; Partition table entry field offsets
;

	const	POFF_STATUS,    0
	const	POFF_CHS_START, 1
	const	POFF_TYPE,      4
	const	POFF_CHS_END,   5
	const	POFF_PSTART,    8
	const	POFF_PSIZE,    12


;-----------------------------------------------------------------------------
;
; Partition type codes (low byte of the POFF_TYPE field)
;

	const	PT_UNUSED,    0
	const	PT_FAT12,     1
	const	PT_FAT16_S,   4		;FAT16 (small, < 32 MB)
	const	PT_EXT,       5		;Extended partition (CHS)
	const	PT_FAT16,     6
	const	PT_FAT16_L,  14		;FAT16 (LBA)
	const	PT_EXT_LBA,  15		;Extended partition (LBA)


;-----------------------------------------------------------------------------
;
; Partition state codes (used by fdisk while editing partitions in memory)
;

	const	PSTATE_EXISTS,  0
	const	PSTATE_ADDED,   1
	const	PSTATE_DELETED, 2


;-----------------------------------------------------------------------------
;
; Partition policy / sizing limits
;

	const	MAX_PARTITIONS_TO_HANDLE,                  256
	const	MIN_DEVICE_SIZE_IN_K,                       10
	const	MIN_DEVICE_SIZE_FOR_PARTITIONS_IN_K,      1024
	const	MIN_REMAINING_SIZE_FOR_NEW_PARTITIONS_IN_K, 100
	const	MIN_PARTITION_SIZE_IN_K,                   100
	const	MAX_DEVICE_SIZE_FOR_DIRECT_FORMAT_IN_K,  32768
	const	MAX_FAT16_PARTITION_SIZE_IN_M,            4096
	const	MAX_FAT12_PARTITION_SIZE_IN_K,           32768
	const	MAX_SECTORS_PER_TRACK,                      63

;Each partition has 1 extra sector for MBR/EBR.
	const	EXTRA_PARTITION_SECTORS, 1


;-----------------------------------------------------------------------------
;
; FAT filesystem structural limits
;

	const	MAX_FAT12_CLUSTER_COUNT,  4084
	const	MAX_FAT16_CLUSTER_COUNT, 65524

	const	FAT12_ROOT_DIR_ENTRIES,    112
	const	FAT16_ROOT_DIR_ENTRIES,    512
	const	DIR_ENTRIES_PER_SECTOR,    512/32

	const	MAX_FAT12_SECTORS_PER_FAT,  12
	const	MAX_FAT16_SECTORS_PER_FAT, 256
	const	FAT_COPIES,                  2
