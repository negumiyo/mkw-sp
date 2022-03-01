#include "Storage.h"

#include "FatStorage.h"
#include "NetStorage.h"

#include <revolution.h>

// Primary storage: FAT/SD
static Storage storage;
// Secondary storage: LAN NetStorage
static Storage sNetStorage;

static bool Storage_find(void) {
    bool result = false;

    if (FatStorage_init(&storage)){
        result = true;
    } else {
        assert(!"Failed to initialize FatStorage");
    }
    storage.next = NULL;

    if (NetStorage_init(&sNetStorage)) {
        storage.next = &sNetStorage;
        sNetStorage.next = NULL;
        result = true;
    }

    return result;
}

bool Storage_init(void) {
    return Storage_find();
}

bool Storage_open(File *file, const wchar_t *path, u32 mode) {
    assert(file);
    assert(path);

    for (Storage *s = &storage; s != NULL; s = s->next) {
        assert(s->open);
        if (s->open(file, path, mode)) {
            file->storage = s;
            return true;
        }
    }

    file->storage = NULL;
    return false;
}

bool Storage_close(File *file) {
    assert(file);
    assert(file->storage);

    return file->storage->close(file);
}

bool Storage_read(File *file, void *dst, u32 size, u32 *readSize) {
    assert(file);
    assert(dst);
    assert(readSize);
    assert(file->storage);

    return file->storage->read(file, dst, size, readSize);
}

bool Storage_write(File *file, const void *src, u32 size, u32 *writtenSize) {
    assert(file);
    assert(src);
    assert(writtenSize);

    return file->storage->write(file, src, size, writtenSize);
}

u64 Storage_size(File *file) {
    assert(file);
    assert(file->storage);

    return file->storage->size(file);
}

bool Storage_lseek(File *file, u64 offset) {
    assert(file);
    assert(file->storage);

    if (!file->storage->lseek(file, offset)) {
        return false;
    }

    return file->storage->tell(file) == offset;
}

u64 Storage_tell(File *file) {
    assert(file);

    return file->storage->tell(file);
}

bool Storage_readFile(const wchar_t *path, void *dst, u32 size, u32 *readSize) {
    File file;
    if (!Storage_open(&file, path, MODE_READ)) {
        return false;
    }

    u64 fileSize = Storage_size(&file);
    if (fileSize < size) {
        size = fileSize;
    }

    if (!Storage_read(&file, dst, size, readSize)) {
        Storage_close(&file);
        return false;
    }

    return Storage_close(&file);
}

bool Storage_writeFile(const wchar_t *path, bool overwrite, const void *src, u32 size) {
    File file;
    u32 mode = MODE_WRITE;
    if (overwrite) {
        mode |= MODE_CREATE_ALWAYS;
    } else {
        mode |= MODE_CREATE_NEW;
    }
    if (!Storage_open(&file, path, mode)) {
        return false;
    }

    u32 writtenSize;
    if (!Storage_write(&file, src, size, &writtenSize)) {
        Storage_close(&file);
        return false;
    }
    if (writtenSize != size) {
        Storage_close(&file);
        return false;
    }

    return Storage_close(&file);
}

// WARNING: Only operates on the primary storage
bool Storage_createDir(const wchar_t *path, bool allowNop) {
    assert(path);

    return storage.createDir(path, allowNop);
}

bool Storage_openDir(Dir *dir, const wchar_t *path) {
    assert(dir);
    assert(path);

    for (Storage *s = &storage; s != NULL; s = s->next) {
        assert(s->openDir);
        if (s->openDir(dir, path)) {
            dir->storage = s;
            return true;
        }
    }

    dir->storage = NULL;
    return false;
}

bool Storage_readDir(Dir *dir, DirEntry *entry) {
    assert(dir);
    assert(dir->storage);
    assert(entry);

    return dir->storage->readDir(dir, entry);
}

bool Storage_closeDir(Dir *dir) {
    assert(dir);
    assert(dir->storage);

    return dir->storage->closeDir(dir);
}

u32 Storage_type(const wchar_t *path) {
    assert(path);

    for (Storage *s = &storage; s != NULL; s = s->next) {
        assert(s->type);
        u32 type = s->type(path);
        if (type != NODE_TYPE_NONE) {
            return type;
        }
    }

    return NODE_TYPE_NONE;
}

// WARNING: Only operates on the primary storage
bool Storage_rename(const wchar_t *srcPath, const wchar_t *dstPath) {
    assert(srcPath);
    assert(dstPath);

    return storage.rename(srcPath, dstPath);
}

// WARNING: Only operates on the primary storage
bool Storage_delete(const wchar_t *path, bool allowNop) {
    assert(path);

    return storage.delete(path, allowNop);
}
