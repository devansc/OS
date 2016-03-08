#include <stdio.h>
#include <stdlib.h>
#include "minlib.h"
#include <time.h>

#define MODE_STRING_SIZE 10

char *getTime(uint32_t timeValue) {
    uint64_t longTime = (uint64_t) timeValue;
    char *time = ctime((time_t *) &longTime);
    if (time == NULL) {
        perror("ctime");
    }
    return time;
}

char * getModeString(uint16_t mode) {
    char *modeString = (char *) malloc(MODE_STRING_SIZE + 1);
    char *pos = modeString;
    *pos++ = mode & DIRECTORY_MASK   ? 'd' : '-';
    *pos++ = mode & OWNER_READ_MASK  ? 'r' : '-';
    *pos++ = mode & OWNER_WRITE_MASK ? 'w' : '-';
    *pos++ = mode & OWNER_EXEC_MASK  ? 'x' : '-';
    *pos++ = mode & GROUP_READ_MASK  ? 'r' : '-';
    *pos++ = mode & GROUP_WRITE_MASK ? 'w' : '-';
    *pos++ = mode & GROUP_EXEC_MASK  ? 'x' : '-';
    *pos++ = mode & OTHER_READ_MASK  ? 'r' : '-';
    *pos++ = mode & OTHER_WRITE_MASK ? 'w' : '-';
    *pos++ = mode & OTHER_EXEC_MASK  ? 'x' : '-';
    *pos = '\0';

    return modeString;
}

void printINode(INode in) {
    printf("\n");
    printf("File inode:\n");
    printf("  uint16_t mode %12dx%.4x (%s)\n", 0, in.mode, 
     getModeString(in.mode));
    printf("  uint16_t links %16hd \n", in.links);
    printf("  uint16_t uid %18hd\n", in.uid);
    printf("  uint16_t gid %18hd\n", in.gid);
    printf("  uint32_t size %17d\n", in.size);
    printf("  uint32_t atime %16d --- %s", in.atime, getTime(in.atime));
    printf("  uint32_t mtime %16d --- %s", in.mtime, getTime(in.mtime));
    printf("  uint32_t ctime %16d --- %s", in.ctime, getTime(in.ctime));
    printf("\n");

    printf("Direct zones:\n");
    for (int i = 0; i < DIRECT_ZONES; i++) {
        printf("           zone[%d]   = %15d\n", i, in.zone[i]);
    }
    printf("  uint32_t indirect %18d\n", in.indirect);
    printf("  uint32_t double %20d\n", in.two_indirect);
}

void printSuperBlock(SuperBlock sb) {
    printf("\n");
    printf("Superblock Contents:\n");
    printf("Stored fields:\n");
    printf("  ninodes %15u\n", sb.ninodes);
    printf("  i_blocks %14hu\n", sb.i_blocks);
    printf("  z_blocks %14hd\n", sb.z_blocks);
    printf("  firstdata %13hu\n", sb.firstdata);
    printf("  log_zone_size %9hd (zone size: %d)\n", sb.log_zone_size,
     sb.blocksize << sb.log_zone_size);
    printf("  max_file %14u\n", sb.max_file);
    printf("  zones %17u\n", sb.zones);
    printf("  magic %12dx%.4x\n", 0, sb.magic);
    printf("  blocksize %13hu\n", sb.blocksize);
    printf("  subversion %12d\n", sb.subversion);
}
