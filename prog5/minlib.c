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

void readBytesFromImage(Image image, void *buffer, int len, 
 const char *errorMessage) {
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
    uint16_t filemask = file.mode & FILETYPE_MASK;
    /* make sure file is regular file or directory */
    if (!(filemask == REGULAR_FILE_MASK || filemask == DIRECTORY_MASK)) {
        fprintf(stderr, "This is not a regular file or directory.\n");
        exit(EXIT_FAILURE);
    }
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
        if (!strcmp(filename, dirent->name) && dirent->inode != 0) { 
            /* found file */
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

int *createZoneSizes(int sizeZone, int lenData, int numTotalZones) {
    int *zoneSizes = (int *) calloc(numTotalZones, sizeof(int));
    int lenCurZone, i;

    if (zoneSizes == NULL) {
        fprintf(stderr, "Program ran out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < numTotalZones && lenData > 0; i++) {
        lenCurZone = lenData > sizeZone ? sizeZone : lenData;
        zoneSizes[i] = lenCurZone;
        lenData -= lenCurZone;
    }
    return zoneSizes;
}

char *getZone(Image image, int zoneNumber, int len) {
    char *zone = (char *) calloc(1, len);
    uint64_t position = image.startOfPartition + image.zonesize * zoneNumber;
    
    if (zone == NULL) {
        fprintf(stderr, "Ran out of memory\n");
        exit(EXIT_FAILURE);
    }

    if (zoneNumber == 0) { /* hole */
        memset((void *) zone, len, 0);
        return zone;
    }
    goToLoc(image, position);
    readBytesFromImage(image, (void *) zone, len, 
     "An error occured reading the filesystem.\n");
    return zone;
}

uint32_t getDataFromZones(Image image, char *fileData, int numZones, 
 uint32_t *zones, uint32_t lenData, uint32_t *position) {
    int *zoneSizes = createZoneSizes(image.zonesize, lenData, numZones);
    int i;
    char *zone;
    
    for (i = 0; i < numZones && zoneSizes[i] != 0; i++) {
        zone = getZone(image, zones[i], zoneSizes[i]);
        memcpy(fileData + *position, zone, zoneSizes[i]);
        *position += zoneSizes[i];
        lenData -= zoneSizes[i];
        free(zone);
    }

    free(zoneSizes);
    return lenData;
}

char *getFileData(Image image, INode inode) {
    uint32_t lenData = inode.size;
    char *fileData = (char *) malloc(inode.size);
    uint32_t position = 0;
    uint32_t *indirectZones, *doublyIndirectZones;
    int i, numZonesInIndirect = image.zonesize / sizeof(uint32_t);

    if (fileData == NULL) {
        fprintf(stderr, "Program ran out of memory.\n");
        exit(EXIT_FAILURE);
    }

    /* Go through direct zones */
    lenData = getDataFromZones(image, fileData, DIRECT_ZONES, inode.zone, 
     lenData, &position);
    
    /* Go through indirect zones */
    if (lenData > 0) {
        indirectZones = (uint32_t *) getZone(image, inode.indirect, 
         image.zonesize);
        lenData = getDataFromZones(image, fileData, 
         numZonesInIndirect, indirectZones, lenData, &position);
    }
    /* Go through doubly indirect zones */
    if (lenData > 0) {
        
        doublyIndirectZones = (uint32_t *) getZone(image, inode.two_indirect, 
         image.zonesize);
        for (i = 0; i < numZonesInIndirect && lenData > 0; i++) {
            indirectZones = (uint32_t *) getZone(image, doublyIndirectZones[i],
              image.zonesize);
            lenData = getDataFromZones(image, fileData, numZonesInIndirect, 
             indirectZones, lenData, &position);
        }
    }

    if (lenData > 0) {
        printf("Warning, did not get all data from file, missing %u"
         " bytes\n", lenData);
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
