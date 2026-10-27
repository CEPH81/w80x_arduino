/*
 * EEPROM.cpp - EEPROM emulada sobre NVS (WM IoT SDK).
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "EEPROM.h"

extern "C" {
#include "wm_nvs.h"
#include "wm_error.h"
}

#define CHUNK  64
#define GROUP  "eeprom"

EEPROMClass EEPROM;

EEPROMClass::EEPROMClass(uint16_t size) : _size(size), _data(nullptr), _dirty(nullptr)
{
    if (_size == 0) _size = EEPROM_DEFAULT_SIZE;
    if (_size > EEPROM_MAX_SIZE) _size = EEPROM_MAX_SIZE;
}

EEPROMClass::~EEPROMClass()
{
    flush();
    free(_data);
    free(_dirty);
}

bool EEPROMClass::begin(size_t size)
{
    if (size > 0 && size != _size) {
        flush();
        free(_data);
        free(_dirty);
        _data  = nullptr;
        _dirty = nullptr;
        _size  = size > EEPROM_MAX_SIZE ? EEPROM_MAX_SIZE : size;
    }
    return load();
}

static void chunk_key(char *k, int c)
{
    snprintf(k, 8, "c%03d", c);
}

bool EEPROMClass::load()
{
    if (_data) {
        return true;
    }
    int chunks = (_size + CHUNK - 1) / CHUNK;
    _data      = (uint8_t *)malloc(chunks * CHUNK);
    _dirty     = (uint32_t *)calloc((chunks + 31) / 32, 4);
    if (!_data || !_dirty) {
        free(_data);
        free(_dirty);
        _data  = nullptr;
        _dirty = nullptr;
        return false;
    }
    memset(_data, 0xFF, chunks * CHUNK);
    wm_nvs_handle_t h;
    if (wm_nvs_open(WM_NVS_DEF_PARTITION, GROUP, WM_NVS_OP_READ_ONLY, &h) == WM_ERR_SUCCESS) {
        char k[8];
        for (int c = 0; c < chunks; c++) {
            size_t len = CHUNK;
            chunk_key(k, c);
            wm_nvs_get_binary(h, k, _data + c * CHUNK, &len);
        }
        wm_nvs_close(h);
    }
    return true;
}

bool EEPROMClass::saveChunk(int c)
{
    wm_nvs_handle_t h;
    if (wm_nvs_open(WM_NVS_DEF_PARTITION, GROUP, WM_NVS_OP_READ_WRITE, &h) != WM_ERR_SUCCESS) {
        return false;
    }
    char k[8];
    chunk_key(k, c);
    int r = wm_nvs_set_binary(h, k, _data + c * CHUNK, CHUNK);
    wm_nvs_close(h);
    if (r == WM_ERR_SUCCESS) {
        _dirty[c / 32] &= ~(1UL << (c % 32));
        return true;
    }
    return false;
}

bool EEPROMClass::flush()
{
    if (!_data) {
        return true;
    }
    bool ok    = true;
    int chunks = (_size + CHUNK - 1) / CHUNK;
    for (int c = 0; c < chunks; c++) {
        if (_dirty[c / 32] & (1UL << (c % 32))) {
            ok &= saveChunk(c);
        }
    }
    return ok;
}

uint8_t EEPROMClass::read(int idx)
{
    if (idx < 0 || idx >= _size || !load()) {
        return 0xFF;
    }
    return _data[idx];
}

uint8_t EEPROMClass::write(int idx, uint8_t val)
{
    if (idx < 0 || idx >= _size || !load()) {
        return 0;
    }
    if (_data[idx] == val) {
        return 1; /* igual que antes: no se gasta la flash */
    }
    _data[idx] = val;
    _dirty[(idx / CHUNK) / 32] |= 1UL << ((idx / CHUNK) % 32);
    return saveChunk(idx / CHUNK) ? 1 : 0;
}

uint8_t EEPROMClass::readBlock(int idx, uint8_t *ptr, uint16_t size)
{
    if (idx < 0 || idx + size > _size || !load()) {
        return 0;
    }
    memcpy(ptr, _data + idx, size);
    return 1;
}

uint8_t EEPROMClass::writeBlock(int idx, const uint8_t *ptr, uint16_t size)
{
    if (idx < 0 || idx + size > _size || !load()) {
        return 0;
    }
    for (uint16_t i = 0; i < size; i++) {
        if (_data[idx + i] != ptr[i]) {
            _data[idx + i] = ptr[i];
            int c = (idx + i) / CHUNK;
            _dirty[c / 32] |= 1UL << (c % 32);
        }
    }
    return flush() ? 1 : 0;
}

size_t EEPROMClass::writeString(int idx, const String &s)
{
    size_t n = s.length() + 1;
    if (!writeBlock(idx, (const uint8_t *)s.c_str(), n)) {
        return 0;
    }
    return s.length();
}

String EEPROMClass::readString(int idx)
{
    String s;
    while (idx >= 0 && idx < _size) {
        uint8_t c = read(idx++);
        if (c == 0 || c == 0xFF) {
            break;
        }
        s += (char)c;
    }
    return s;
}

bool EEPROMClass::clear()
{
    if (!load()) {
        return false;
    }
    memset(_data, 0xFF, _size);
    wm_nvs_handle_t h;
    if (wm_nvs_open(WM_NVS_DEF_PARTITION, GROUP, WM_NVS_OP_READ_WRITE, &h) != WM_ERR_SUCCESS) {
        return false;
    }
    int r = wm_nvs_del_group(h);
    wm_nvs_close(h);
    int chunks = (_size + CHUNK - 1) / CHUNK;
    memset(_dirty, 0, ((chunks + 31) / 32) * 4);
    return r == WM_ERR_SUCCESS;
}
