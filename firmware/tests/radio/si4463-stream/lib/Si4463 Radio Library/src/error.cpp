#include "Arduino.h"

int error(int err, const char *file, int line) {
    Serial.printf("Error: %s (%d) at %s:%d\n\r", "" /* strerror[err] */ , err, file, line);
    // while (1);
    return err;
}
