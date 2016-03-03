#include <stdint.h>

#define PARTITION_TABLE_LOC 0x1BE
#define PARTION_TYPE 0x81
#define MINIX_MAGIC_510 0x55
#define MINIX_MAGIC_511 0xAA
#define MINIX_MAGIC_NUM 0x4D5A
#define MINIX_MAGIC_REVERSED 0x5A4D
#define INODE_SIZE 64
#define DIRENT_SIZE 64

struct superblock {
	uint32_t ninodes;      /* number of inodes in this filesystem*/
	uint16_t pad1;         /* make things line up properly */
	int16_t i_blocks;      /* # of blocks used by inode bit map */
	int16_t z_blocks;      /* # of blocks used by zone bit map */
	uint16_t firstdata;    /* number of first data zone */
	int16_t log_zone_size; /* log2 of blocks per zone */
	int16_t pad2;          /* make things line up again */
	uint32_t max_file;     /* maximum file size */
	uint32_t zones;        /* number of zones on disk */
	int16_t magic;         /* magic number */
	int16_t pad3;          /* make things line up again */
	uint16_t blocksize;    /* block size in bytes */
	uint8_t subversion;    /* filesystem sub-version */
};

struct partition_table {
	uint8_t bootind;        /* boot indicator 0/ACTIVE_FLAG  */
	uint8_t start_head;     /* head value for first sector   */
	uint8_t start_sec;      /* sector value + cyl bits for first sector */
	uint8_t start_cyl;      /* track value for first sector  */
	uint8_t sysind;         /* system indicator              */
	uint8_t last_head;      /* head value for last sector    */
	uint8_t last_sec;       /* sector value + cyl bits for last sector */
	uint8_t last_cyl;       /* track value for last sector   */
	uint32_t lowsec;        /* logical first sector          */
	uint32_t size;          /* size of partition in sectors  */
};

#define DIRECT_ZONES 7

struct inode {
	uint16_t mode;     /* mode */
	uint16_t links;    /* number of links */
	uint16_t uid;
	uint16_t gid;
	int32_t atime;
	int32_t mtime;
	int32_t ctime;
	uint32_t zone[DIRECT_ZONES];
	uint32_t indirect;
	uint32_t two_indirect;
	uint32_t unused;
};

#define FILETYPE_MASK     0170000
#define REGULAR_FILE_MASK 0100000
#define DIRECTORY_MASK    0040000
#define OWNER_READ_MASK   0000400
#define OWNER_WRITE_MASK  0000200
#define OWNER_EXEC_MASK   0000100
#define GROUP_READ_MASK   0000040
#define GROUP_WRITE_MASK  0000020
#define GROUP_EXEC_MASK   0000010
#define OTHER_READ_MASK   0000004
#define OTHER_WRITE_MASK  0000002
#define OTHER_EXEC_MASK   0000001

typedef enum boolean {
    FALSE, TRUE
} bool;

typedef struct image {
    bool verbose;
    int partition;
    int subpartition;
    char *imageFile;
    char *path;
    FILE *file;
} Image;


void openImage(Image *image);
void printUsageAndExit();
void parseArgs(int argc, char **argv, Image *image, int indexParameter);
int parseOptions(int argc, char **argv, Image *image);
