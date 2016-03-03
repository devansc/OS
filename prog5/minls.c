#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "minlib.h"


int main(int argc, char **argv) {
    Image image;

    parseArgs(argc, argv, &image, parseOptions(argc, argv, &image));
    openImage(&image);

    return 0;
}

/* 
 * Parses the arguments in argv. indexParameter is the first argument to parse.
 */
void parseArgs(int argc, char **argv, Image *image, int indexParameter) {
    if (argc <= indexParameter) { /* missing imagefile */
        printUsageAndExit();
    } else {
        image->imageFile = malloc(strlen(argv[indexParameter]) + 1);
        strcpy(image->imageFile, argv[indexParameter]);
        indexParameter++;
    }

    if (argc > indexParameter + 1) { /* too many args */
        printUsageAndExit();
    } 
    if (argc == indexParameter + 1) { 
        image->path = malloc(strlen(argv[indexParameter]) + 1);
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
	while ((ch = getopt(argc, argv, "vp:s:")) != -1) {
		switch (ch) {
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
    exit(EXIT_FAILURE);
}
