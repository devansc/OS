#include <stdio.h>
#include <unistd.h>


int main(int argc, char **argv) {
    Image image;


    parseArgs(argc, argv, &image);
    return 0;
}
