#include <minix/drivers.h>
#include <minix/driver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include <sys/ucred.h>
#include <sys/ioctl.h>
#include <minix/const.h>
#include "secret.h"

#define MAX_SECRET_SIZE 8192
#define NO_OWNER -1
#define TRUE 1
#define FALSE 0

/*
 * Function prototypes for the secret driver.
 */
FORWARD _PROTOTYPE( char * secret_name,   (void) );
FORWARD _PROTOTYPE( int secret_open,      (struct driver *d, message *m) );
FORWARD _PROTOTYPE( int secret_ioctl,     (struct driver *d, message *m) );
FORWARD _PROTOTYPE( int secret_close,     (struct driver *d, message *m) );
FORWARD _PROTOTYPE( struct device * secret_prepare, (int device) );
FORWARD _PROTOTYPE( int secret_transfer,  (int procnr, int opcode,
                                          u64_t position, iovec_t *iov,
                                          unsigned nr_req) );
FORWARD _PROTOTYPE( void secret_geometry, (struct partition *entry) );

/* SEF functions and variables. */
FORWARD _PROTOTYPE( void sef_local_startup, (void) );
FORWARD _PROTOTYPE( int sef_cb_init, (int type, sef_init_info_t *info) );
FORWARD _PROTOTYPE( int sef_cb_lu_state_save, (int) );
FORWARD _PROTOTYPE( int lu_state_restore, (void) );

/* Entry points to the secret driver. */
PRIVATE struct driver secret_tab =
{
    secret_name,
    secret_open,
    secret_close,
    secret_ioctl,
    secret_prepare,
    secret_transfer,
    nop_cleanup,
    secret_geometry,
    nop_alarm,
    nop_cancel,
    nop_select,
    nop_ioctl,
    do_nop,
};

/** Represents the /dev/secret device. */
PRIVATE struct device secret_device;

/** State variable to count the number of times the device has been opened. */
PRIVATE int open_counter;
PRIVATE char secret[MAX_SECRET_SIZE];
PRIVATE uid_t current_owner;
PRIVATE int readingSecret;
PRIVATE int readPosition, writePosition;

PRIVATE char * secret_name(void)
{
    printf("secret_name()\n");
    return "secret";
}

PRIVATE int secret_open(d, m)
    struct driver *d;
    message *m;
{
    struct ucred openedUser;

    /* Device may only be opened with read OR write access */
    if (m->COUNT & W_BIT && m->COUNT & R_BIT) {
        printf("Device may not be opened with read and write permissions.\n");
        return EACCES;
    }

    if (getnucred(m->IO_ENDPT, &openedUser) != 0) {
        perror("getnucred");
        return -1;
    }

    printf("openedUser is %p UID %d is trying to open file\n", openedUser, openedUser.uid);

    /* Make sure the device is being opened by the owner, or has no owner */
    if (current_owner == NO_OWNER) {
        ++open_counter;
        if (m->COUNT & W_BIT) {
            current_owner = openedUser.uid;
            printf("owner is %d\n", current_owner);
        }
    } else if (current_owner != openedUser.uid) { /* HAS OWNER */
        printf("User doesn't have access to device\n");
        return EACCES;
    } else if (m->COUNT & W_BIT) {
        printf("Secret device in use by another user\n");
        return ENOSPC;
    } else {
        ++open_counter;
    }

    if (m->COUNT & R_BIT)
        readingSecret = TRUE;

    return OK;
}

PRIVATE int secret_close(d, m)
    struct driver *d;
    message *m;
{
    /* If closing and no other readers, then clear the secret */
    if (--open_counter == 0 && readingSecret) {
        current_owner = NO_OWNER;
        memset(secret, 0, MAX_SECRET_SIZE);
        readingSecret = FALSE;
        printf("clearing secret\n");
        readPosition = writePosition = 0;
    }
    printf("have %d open file descriptors after closing\n", open_counter);
    return OK;
}

PRIVATE int secret_ioctl(d, m)
    struct driver *d;
    message *m;
{
    uid_t grantee; /* the uid of the new owner of the secret */
    int res;

    printf("secret_ioctl()\n");
    /* Grant access to another user with SSGRANT */
    switch (m->REQUEST) {
    case SSGRANT:
        res = sys_safecopyfrom(m->IO_ENDPT, (vir_bytes)m->IO_GRANT,
                0, (vir_bytes)&grantee, sizeof(grantee), D);
        current_owner = grantee;
        break;
    default:
        printf("Unsupported ioctl call to secret driver\n");
        return ENOTTY;
    }
    return res;
}

PRIVATE struct device * secret_prepare(dev)
    int dev;
{
    secret_device.dv_base.lo = 0;
    secret_device.dv_base.hi = 0;
    secret_device.dv_size.lo = MAX_SECRET_SIZE;
    secret_device.dv_size.hi = 0;
    return &secret_device;
}

PRIVATE int secret_transfer(proc_nr, opcode, unusedPosition, iov, nr_req)
    int proc_nr;
    int opcode;
    u64_t unusedPosition;
    iovec_t *iov;
    unsigned nr_req;
{
    int readBytes, writeBytes, ret;

    printf("secret_transfer()\n");

    /* Limit the number of bytes to write or read to the MAX_SECRET_SIZE */
    writeBytes = MAX_SECRET_SIZE - writePosition < iov->iov_size ?
            MAX_SECRET_SIZE - writePosition : iov->iov_size;

    readBytes = writePosition - readPosition < iov->iov_size ?
            writePosition - readPosition : iov->iov_size;

    /* Return now if not actually reading or writing bytes */
    if ((readBytes <= 0 && opcode == DEV_GATHER_S) || 
        (writeBytes <= 0 && opcode == DEV_SCATTER_S))
    {
        return OK;
    }
    switch (opcode)
    {
        case DEV_GATHER_S:
            printf("Reading from device, pos %d, bytes %d, iov_size %d\n", readPosition, readBytes, iov->iov_size);
            ret = sys_safecopyto(proc_nr, iov->iov_addr, 0,
                                (vir_bytes) (secret + readPosition),
                                 readBytes, D);
            iov->iov_size -= readBytes;
            readPosition += readBytes;
            break;

        case DEV_SCATTER_S:
            printf("Writing to device, pos %d, bytes %d\n", writePosition, writeBytes);
            ret = sys_safecopyfrom(proc_nr, iov->iov_addr, 0,
                                  (vir_bytes) (secret + writePosition),
                                  writeBytes, D);
            iov->iov_size -= writeBytes;
            writePosition += writeBytes;
            break;
        default:
            printf("Unknown transfer opcode\n");
            return EINVAL;
    }
    return ret;
}

PRIVATE void secret_geometry(entry)
    struct partition *entry;
{
    printf("secret_geometry()\n");
    entry->cylinders = 0;
    entry->heads     = 0;
    entry->sectors   = 0;
}

PRIVATE int sef_cb_lu_state_save(int state) {
    /* Save the state. */
    printf("saving open_counter %d\n", open_counter);
    ds_publish_u32("open_counter", open_counter, DSF_OVERWRITE);
    printf("saving readPosition %d\n", readPosition);
    ds_publish_u32("readPosition", readPosition, DSF_OVERWRITE);
    ds_publish_u32("readingSecret", readingSecret, DSF_OVERWRITE);
    ds_publish_u32("current_owner", current_owner, DSF_OVERWRITE);
    ds_publish_u32("writePosition", writePosition, DSF_OVERWRITE);
    ds_publish_mem("secret", secret, MAX_SECRET_SIZE, 0);

    return OK;
}

PRIVATE int lu_state_restore() {
    /* Restore the state. */
    u32_t saveCounter, saveReadPos, saveWritePos, saveReadingSecret, saveCurrentOwner;
    size_t size_secret = MAX_SECRET_SIZE;

    ds_retrieve_u32("open_counter", &saveCounter);
    ds_delete_u32("open_counter");
    open_counter = (int) saveCounter;
    printf("recovered open_counter %d\n", open_counter);

    ds_retrieve_u32("readPosition", &saveReadPos);
    ds_delete_u32("readPosition");
    readPosition = (int) saveReadPos;
    printf("recovered readPosition %d\n", readPosition);

    ds_retrieve_u32("writePosition", &saveWritePos);
    ds_delete_u32("writePosition");
    writePosition = (int) saveWritePos;

    ds_retrieve_u32("readingSecret", &saveReadingSecret);
    ds_delete_u32("readingSecret");
    readingSecret = (int) saveReadingSecret;

    ds_retrieve_u32("current_owner", &saveCurrentOwner);
    ds_delete_u32("current_owner");
    current_owner = (int) saveCurrentOwner;

    ds_retrieve_mem("secret", secret, &size_secret);
    ds_delete_mem("secret");

    return OK;
}

PRIVATE void sef_local_startup()
{
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);

    /*
     * Register live update callbacks.
     */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);

    /* Let SEF perform startup. */
    sef_startup();
}

PRIVATE int sef_cb_init(int type, sef_init_info_t *info)
{
/* Initialize the secret driver. */
    int do_announce_driver = TRUE;

    open_counter = 0;
    switch(type) {
        case SEF_INIT_FRESH:
            printf("%s", HELLO_MESSAGE);
        break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;

            printf("%sHey, I'm a new version!\n", HELLO_MESSAGE);
        break;

        case SEF_INIT_RESTART:
            printf("%sHey, I've just been restarted!\n", HELLO_MESSAGE);
        break;
    }

    /* Announce we are up when necessary. */
    if (do_announce_driver) {
        driver_announce();
    }

    /* Initialization completed successfully. */
    return OK;
}

PUBLIC int main(int argc, char **argv)
{
    /*
     * Perform initialization.
     */
    sef_local_startup();

    /* Initialize global variables */
    current_owner = NO_OWNER;
    readingSecret = FALSE;
    readPosition = writePosition = 0;

    /*
     * Run the main loop.
     */
    driver_task(&secret_tab, DRIVER_STD);
    return OK;
}

