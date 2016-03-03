#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "minlib.h"

void openImage(Image *image) {
    image->file = fopen(image->imageFile, "r");
    if (!image->file) {
        fprintf(stderr, "Couldn't open image file.\n");
        exit(EXIT_FAILURE);
    }
}
