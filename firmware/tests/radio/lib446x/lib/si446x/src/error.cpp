#include "stdio.h"

int error(int err, const char *file, int line) {
    printf("Error: %s (%d) at %s:%d\n", "" /* strerror[err] */ , err, file, line);
    // while (1);
    return err;
}
