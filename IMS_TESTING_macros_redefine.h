#ifndef __IMS_TESTING_MACROS_REDEFINE_H
#define __IMS_TESTING_MACROS_REDEFINE_H

#include <stdio.h>

#define IMS_IBCF_LOGHIGH
#define IMS_IBCF_LOGLOW

#define IMS_DLOG(_para_1, _para_2, args...) \
    printf(_para_2, ##args);                \
    printf("\n");

#define TEST_J

#endif  // end of __IMS_TESTING_MACROS_REDEFINE_H