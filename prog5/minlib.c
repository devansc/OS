#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "minlib.h"
#include "minprint.h"

void openImageFile(Image *image) {
    image->file = fopen(image->imageFile, "rb");
    if (!image->file) {
        fprintf(stderr, "Couldn't open image file.\n");
        exit(EXIT_FAILURE);
    }
}

void readBytesFromImage(Image image, void *buffer, int len, const char *errorMessage) {
    int res = fread(buffer, len, 1, image.file);
    if (ferror(image.file) || res != 1) {
        fprintf(stderr, errorMessage);
        exit(EXIT_FAILURE);
    }
}

File createFile(Image image, INode in) {
    File file = { 
                 in.mode & DIRECTORY_MASK,
                 in.mode, 
                 in.size,
                 getFileData(image, in)
                };
    return file;
}

INode getNextINode(Image image, INode inode, char *filename) {
    char *fileData;
    DirEnt *dirent;
    uint32_t posData = 0;

    if (!(inode.mode & DIRECTORY_MASK)) {
        fprintf(stderr, "Error -- %s is not a directory\n", filename);
        exit(EXIT_FAILURE);
    }

    if (inode.size % DIRENT_SIZE) {
        fprintf(stderr, "Found invalid directory contents\n");
        exit(EXIT_FAILURE);
    }
    fileData = getFileData(image, inode);
    dirent = (DirEnt *) fileData;
    for ( ; posData < inode.size; posData += DIRENT_SIZE, dirent++) {
        if (!strcmp(filename, dirent->name)) { /* found file */
            return getINode(image, dirent->inode);
        }
    }
    fprintf(stderr, "No such file or directory.\n");
    exit(EXIT_FAILURE);
}

File getFile(Image image, char *path, INode inode) {
    char *nextFileName = strsep(&path, "/");

    /* No more string to parse */
    if (nextFileName == NULL) 
        return createFile(image, inode);

    /* Ignore extra '/' characters */
    if (!strcmp(nextFileName, ""))
        return getFile(image, path, inode);

    return getFile(image, path, getNextINode(image, inode, nextFileName));
}

int *createZoneSizes(int zoneSize, int lenData) {
    int *zoneSizes = (int *) malloc(DIRECT_ZONES);
    int lenCurZone;

    if (zoneSizes == NULL) {
        fprintf(stderr, "Program ran out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < DIRECT_ZONES && lenData > 0; i++) {
        lenCurZone = lenData > zoneSize ? zoneSize : lenData;
        zoneSizes[i] = lenCurZone;
        lenData -= lenCurZone;
    }
    return zoneSizes;
}

char *getZone(Image image, int zoneNumber, int len) {
    char *zone = (char *) malloc(len);
    uint64_t position = image.startOfPartition + image.zonesize * zoneNumber;
    
    if (zone == NULL) {
        fprintf(stderr, "Ran out of memory\n");
        exit(EXIT_FAILURE);
    }

    goToLoc(image, position);
    readBytesFromImage(image, (void *) zone, len, 
     "An error occured reading the filesystem.\n");
    return zone;
}

char *getFileData(Image image, INode inode) {
    uint32_t lenData = inode.size;
    int numZones = lenData / image.zonesize + 
     lenData % image.zonesize ? 1 : 0;
    int i;
    char *fileData = (char *) malloc(lenData);
    int *zoneSizes = createZoneSizes(image.zonesize, lenData);
    int position = 0;

    if (fileData == NULL) {
        fprintf(stderr, "Program ran out of memory.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < DIRECT_ZONES && inode.zone[i] != 0; i++) {
        char *zone = getZone(image, inode.zone[i], zoneSizes[i]);
        memcpy(fileData + position, zone, zoneSizes[i]);
        position += zoneSizes[i];
        free(zone);
    }
    if (numZones > i) {
        printf("Warning, did not get all data from file, wanted %d zones\n", numZones);
    }
    return fileData;
}

INode getINode(Image image, int number) {
    uint64_t diskPosition;
    uint16_t blocksize = image.superBlock.blocksize;
    INode inode;

    /* Get to start of imap */
    diskPosition = image.startOfPartition + 2 * blocksize;
    /* Go past imap */
    diskPosition += image.superBlock.i_blocks * blocksize;
    /* Go past zmap */
    diskPosition += image.superBlock.z_blocks * blocksize;
    /* Go to correct iNode # */
    diskPosition += (number - 1) * sizeof(INode);

    /*printf("in getINode getting position %" PRIu64 "\n", diskPosition);*/
    goToLoc(image, diskPosition);
    readBytesFromImage(image, (void *) &inode, sizeof(INode), 
     "Trouble finding INode\n");
    if (image.verbose) 
        printINode(inode);
    return inode;
}

void openPartitions(Image *image) {
    if (image->partition == -1)
        return; 
    
    openPartition(image, image->partition);

    if (image->subpartition == -1)
        return;

    openPartition(image, image->subpartition);
}

void goToLoc(Image image, uint64_t location) {
    if (fseek(image.file, location, SEEK_SET) == -1) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
}

void openPartition(Image *image, int partitionNumber) {
    uint8_t block[BLOCK_SIZE];
    PartitionTable *pt;

    goToLoc(*image, image->startOfPartition);

    readBytesFromImage(*image, (void *) block, BLOCK_SIZE, 
     "An error occured while reading the filesystem.\n");

    if (block[510] != MINIX_MAGIC_510 || block[511] != MINIX_MAGIC_511) {
        fprintf(stderr, "Could not find the minix partition table\n");
        exit(EXIT_FAILURE);
    }

    pt = (PartitionTable *) (block + PARTITION_TABLE_LOC);
    pt += partitionNumber;

    if (pt->sysind != MINIX_PARTITION_TYPE) {
        fprintf(stderr, "Did not find a minix partition\n");
        exit(EXIT_FAILURE);
    }
    image->startOfPartition =  pt->lowsec * SECTOR_SIZE;
}

void openSuperBlock(Image *image) {
    uint8_t block[BLOCK_SIZE];
    SuperBlock *sprBlock;

    goToLoc(*image, image->startOfPartition + BLOCK_SIZE);
    readBytesFromImage(*image, (void *) block, BLOCK_SIZE, 
     "An error occured reading the super block of the filesystem.\n");
    sprBlock = (SuperBlock *)block;


    if (sprBlock->magic !=  MINIX_MAGIC_NUM) {
        fprintf(stderr, "Filesystem is not a Minix Filesystem\n");
        exit(EXIT_FAILURE);
    }
    image->superBlock = *sprBlock;
    image->zonesize = sprBlock->blocksize << sprBlock->log_zone_size;
    if (image->verbose) 
        printSuperBlock(*sprBlock);
}
