#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "minlib.h"
#include "minprint.h"


int main(int argc, char **argv) {
    Image image;
    File foundFile;
    INode root;

    parseArgs(argc, argv, &image, parseOptions(argc, argv, &image));

    openImageFile(&image);
    image.startOfPartition = 0;
    openPartitions(&image);
    openSuperBlock(&image);

    if (image.path && strlen(image.path) > 0) {
        foundFile = getFile(image, strdup(image.path), getINode(image, 1));
    } else {
        foundFile = createFile(image, getINode(image, 1));
    }

    printls(image, foundFile, image.path);
    
    return 0;
}

void printls(Image image, File foundFile, char *path) {
    DirEnt *dirent;
    uint32_t posData = 0;
    INode inode;
    
    if (foundFile.isDir) {
        dirent = (DirEnt *) foundFile.data;
        for ( ; posData < foundFile.size; posData += DIRENT_SIZE, dirent++) {
            /*
            printf("%4d     %s\n", dirent->inode, dirent->name);
            */
            inode = getINode(image, dirent->inode);
            printf("%s %9u %s\n", getModeString(inode.mode), inode.size, dirent->name);
        }
    } else {
        printf("%s %9u %s\n", getModeString(foundFile.mode), foundFile.size, path);
    }
}

/* 
 * Parses the arguments in argv. indexParameter is the first argument to parse.
 */
void parseArgs(int argc, char **argv, Image *image, int indexParameter) {
    image->path = NULL;
    if (argc <= indexParameter) { /* missing imagefile */
        printUsageAndExit();
    } else {
        image->imageFile = (char *) malloc(strlen(argv[indexParameter]) + 1);
        strcpy(image->imageFile, argv[indexParameter]);
        indexParameter++;
    }

    if (argc > indexParameter + 1) { /* too many args */
        printUsageAndExit();
    } 
    if (argc == indexParameter + 1) { 
        image->path = (char *) malloc(strlen(argv[indexParameter]) + 1);
        strcpy(image->path, argv[indexParameter]);
    }
}

/* 
 * Parses the options in argv, and returns the index of the next parameter 
 * that's not an option 
 */
int parseOptions(int argc, char **argv, Image *image) {
	char ch;
    bool pFlag = FALSE;

    image->verbose = FALSE;
    image->partition = -1;
    image->subpartition = -1;

	while ((ch = getopt(argc, argv, "vhp:s:")) != -1) {
		switch (ch) {
		case 'h':
            printUsageAndExit();
            break;

		case 'v':
			image->verbose = TRUE;
			break;

		case 'p':
			if ((image->partition = (int) strtol(optarg, NULL, 10)) < 0 || 
             errno == EINVAL) {
				fprintf(stderr, "Partion must be a positive integer\n");
				exit(EXIT_FAILURE);
			} 
            pFlag = TRUE;
			break;

		case 's':
			if ((image->subpartition = (int) strtol(optarg, NULL, 10)) < 0 || 
             errno == EINVAL) {
				fprintf(stderr, "Subpartion must be a positive integer\n");
				exit(1);
			}
			if (!pFlag) {
                printUsageAndExit();
			}
			break;
		case '?':
		default:
			printUsageAndExit();
		}
	}
    return optind;
}

void printUsageAndExit() {
    fprintf(stderr, 
     "usage: minls [ -v ] [ -p part [ -s subpart ] ] imagefile [ path ]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t-p  part    ---- select partition for filesystem "
     "(default: none)\n");
    fprintf(stderr, "\t-s  sub     ---- select subpartition for filesystem "
     "(default: none)\n");
    fprintf(stderr, "\t-h  help    ---- print usage information and exit\n");
    fprintf(stderr, "\t-v  verbose ---- increase verbosity level\n");
    exit(EXIT_FAILURE);
}
