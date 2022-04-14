#include "dvd.h"

#include "dvdex.h"

typedef struct {
    u8 isDir : 8;
    u32 stringOffset : 24;
    union {
        struct {
            u32 _4;
            u32 next;
        } dir;
        struct {
            u32 startAddr;
            u32 length;
        } file;
    };
} FstEntry;
static_assert(sizeof(FstEntry) == 0xc);

extern u32 MaxEntryNum;
extern char *FstStringStart;
extern FstEntry *FstStart;

BOOL my_DVDOpen(const char *fileName, DVDFileInfo *fileInfo) {
    if (DVDExOpen(fileName, fileInfo)) {
        return true;
    }

    s32 entrynum = DVDConvertPathToEntrynum(fileName);
    if (entrynum < 0 || (u32)entrynum >= MaxEntryNum) {
        return false;
    }
    if (FstStart[entrynum].isDir) {
        return false;
    }

    fileInfo->startAddr = FstStart[entrynum].file.startAddr;
    fileInfo->length = FstStart[entrynum].file.length;
    fileInfo->callback = NULL;
    fileInfo->cb.state = 0;
    fileInfo->cb.command = 0; // We need to initialize it to mark the file as non-replaced
    return true;
}
PATCH_B(DVDOpen, my_DVDOpen);

BOOL my_DVDClose(DVDFileInfo *fileInfo) {
    if (fileInfo->cb.command == (u32)-1) {
        return DVDExClose(fileInfo);
    }

    DVDCancel(&fileInfo->cb);
    return true;
}
PATCH_B(DVDClose, my_DVDClose);
