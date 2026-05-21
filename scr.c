#include "mac_types.h"


#define PROT 1
#pragma pack(push, 1);
struct fieldhat {
    u8 row,col,len,attr;
    char buf[64];
};
#pragma pack(pop);








