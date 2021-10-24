/*
 * error.h
 *
 *  Created on: Jan 23, 2019
 *      Author: iracigt
 */

#pragma once

// Import all lib446x errors
#include "si446x.h"


#define EOVERFLOW 11
#define EUNDERFLOW 12
#define ECMDINVAL   19
#define ECMDBADSUM  22

int error(int err, const char *file, int line);
