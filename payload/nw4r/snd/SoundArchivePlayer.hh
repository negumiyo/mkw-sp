#pragma once

#include "nw4r/snd/SoundArchiveLoader.hh"

namespace nw4r::snd {

class SoundArchivePlayer {
public:
    bool isAvailable() const;
#if ENABLE_SASR
    REPLACE bool loadGroup(u32 groupId, void *r5, u32 r6);
    bool loadFile(SoundArchiveLoader *loader, u32 fileId, void *r6);
    REPLACE void *getFileAddress(u32 fileId);
    REPLACE void *getFileWaveDataAddress(u32 fileId);
#endif

private:
    struct FileAddress {
        void *address;
        void *waveDataAddress;
    };
    static_assert(sizeof(FileAddress) == 0x8);

    struct FileTable {
        u32 count;
        FileAddress entries[];
    };

    u8 _00[0x10 - 0x00];
    SoundArchive *m_archive;
#if ENABLE_SASR
    FileTable *m_fileTable; // Modified (groupTable originally)
    u8 _18[0xe0 - 0x18];
#else
    u8 _14[0xe0 - 0x14];
#endif
};
static_assert(sizeof(SoundArchivePlayer) == 0xe0);

} // namespace nw4r::snd
