#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "minlib.h"

#define STDOUT_FD 1

int main(int argc, char **argv) {
    Image image;
    File foundFile;
    INode root;

    parseArgs(argc, argv, &image, parseOptions(argc, argv, &image));

    openImageFile(&image);
    image.startOfPartition = 0;
    openPartitions(&image);
    openSuperBlock(&image);

    root = getINode(image, 1);
    foundFile = getFile(image, strdup(image.path), root);

    printFile(foundFile, image.fdWrite, image.path);
    
    return 0;
}

void printFile(File foundFile, int writeFD, char *path) {
    uint32_t res;

    if (foundFile.isDir) {
        fprintf(stderr, "%s is a directory\n", path);
        exit(EXIT_FAILURE);
    }

    res = write(writeFD, foundFile.data, foundFile.size);
    if (res != foundFile.size) {
        fprintf(stderr, "Couldn't write all output... Exitting\n");
        exit(EXIT_FAILURE);
    }
}


/* 
 * Parses the arguments in argv. indexParameter is the first argument to parse.
 */
void parseArgs(int argc, char **argv, Image *image, int indexParameter) {
    image->path = NULL;
    image->fdWrite = STDOUT_FD;
    if (argc <= indexParameter) { /* missing imagefile */
        printUsageAndExit();
    } else {
        image->imageFile = strdup(argv[indexParameter]);
        strcpy(image->imageFile, argv[indexParameter]);
        indexParameter++;
    }

    if (argc > indexParameter) { 
        image->path = strdup(argv[indexParameter]);
        strcpy(image->path, argv[indexParameter]);
        indexParameter++;
    }

    if (argc > indexParameter + 1) { /* too many args */
        printUsageAndExit();
    } else if (argc == indexParameter + 1) {
        image->fdWrite = open(argv[indexParameter], 
         O_WRONLY | O_TRUNC | O_CREAT);
        if (image->fdWrite < 0) {
            fprintf(stderr, "Could not open %s for writing.\n", 
             argv[indexParameter]);
            exit(EXIT_FAILURE);
        }
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
			if ((image->partition = (int) strtol(optarg, NULL, 10))
             < 0 || errno == EINVAL) {
				fprintf(stderr, 
                 "Partion must be a positive integer\n");
				exit(EXIT_FAILURE);
			} 
            pFlag = TRUE;
			break;

		case 's':
            image->subpartition = (int) strtol(optarg, NULL, 10);
			if (image->subpartition < 0 || errno == EINVAL) {
				fprintf(stderr, 
                 "Subpartion must be a positive integer\n");
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
