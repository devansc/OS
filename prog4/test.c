#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int fd, res;
    uid_t uid;
    char *msg = "This is a test message\n";

    fd = open(FILENAME, O WRONLY);
    printf("Opening... fd=%d\n",fd);
    res = write(fd,msg,strlen(msg));
    printf("Writing... res=%d\n",res);
    /* try grant */
    if ( argc > 1 && 0 != (uid=atoi(argv[1]))) {
        if ( res = ioctl(fd,SSGRANT,&uid) )
            perror("ioctl");
        printf("Trying to change owner to %d...res=%d\n",uid, res);
    }
    res=close(fd);

    return 0;
}
