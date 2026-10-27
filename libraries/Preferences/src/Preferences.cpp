/*
 * Preferences.cpp - ajustes clave/valor sobre NVS (WM IoT SDK).
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Preferences.h"

extern "C" {
#include "wm_nvs.h"
#include "wm_error.h"
}

#define H ((wm_nvs_handle_t)_handle)
#define OK(x) ((x) == WM_ERR_SUCCESS)

bool Preferences::begin(const char *name, bool readOnly, const char *partition_label)
{
    if (_handle) {
        return false;
    }
    wm_nvs_handle_t h = nullptr;
    const char *part  = partition_label ? partition_label : WM_NVS_DEF_PARTITION;
    if (!OK(wm_nvs_open(part, name, readOnly ? WM_NVS_OP_READ_ONLY : WM_NVS_OP_READ_WRITE, &h))) {
        /* un grupo que aun no existe no se puede abrir en solo lectura */
        if (!readOnly || !OK(wm_nvs_open(part, name, WM_NVS_OP_READ_WRITE, &h))) {
            return false;
        }
    }
    _handle   = h;
    _readOnly = readOnly;
    return true;
}

void Preferences::end()
{
    if (_handle) {
        wm_nvs_close(H);
        _handle = nullptr;
    }
}

bool Preferences::clear()
{
    return _handle && !_readOnly && OK(wm_nvs_del_group(H));
}

bool Preferences::remove(const char *key)
{
    return _handle && !_readOnly && OK(wm_nvs_del_key(H, key));
}

bool Preferences::isKey(const char *key)
{
    return getBytesLength(key) > 0;
}

#define PUT(fn, type, setter)                                                 \
    size_t Preferences::fn(const char *key, type value)                       \
    {                                                                         \
        if (!_handle || _readOnly) return 0;                                  \
        return OK(setter(H, key, value)) ? sizeof(type) : 0;                  \
    }
PUT(putChar, int8_t, wm_nvs_set_i8)
PUT(putUChar, uint8_t, wm_nvs_set_u8)
PUT(putShort, int16_t, wm_nvs_set_i16)
PUT(putUShort, uint16_t, wm_nvs_set_u16)
PUT(putInt, int32_t, wm_nvs_set_i32)
PUT(putUInt, uint32_t, wm_nvs_set_u32)
PUT(putLong64, int64_t, wm_nvs_set_i64)
PUT(putULong64, uint64_t, wm_nvs_set_u64)

#define GET(fn, type, getter)                                                 \
    type Preferences::fn(const char *key, type defaultValue)                  \
    {                                                                         \
        type v = defaultValue;                                                \
        if (!_handle || !OK(getter(H, key, &v))) return defaultValue;         \
        return v;                                                             \
    }
GET(getChar, int8_t, wm_nvs_get_i8)
GET(getUChar, uint8_t, wm_nvs_get_u8)
GET(getShort, int16_t, wm_nvs_get_i16)
GET(getUShort, uint16_t, wm_nvs_get_u16)
GET(getInt, int32_t, wm_nvs_get_i32)
GET(getUInt, uint32_t, wm_nvs_get_u32)
GET(getLong64, int64_t, wm_nvs_get_i64)
GET(getULong64, uint64_t, wm_nvs_get_u64)

size_t Preferences::putFloat(const char *key, float value)
{
    return putBytes(key, &value, sizeof(value));
}

size_t Preferences::putDouble(const char *key, double value)
{
    return putBytes(key, &value, sizeof(value));
}

float Preferences::getFloat(const char *key, float defaultValue)
{
    float v;
    return getBytes(key, &v, sizeof(v)) == sizeof(v) ? v : defaultValue;
}

double Preferences::getDouble(const char *key, double defaultValue)
{
    double v;
    return getBytes(key, &v, sizeof(v)) == sizeof(v) ? v : defaultValue;
}

size_t Preferences::putString(const char *key, const char *value)
{
    if (!_handle || _readOnly || !value) return 0;
    return OK(wm_nvs_set_str(H, key, value)) ? strlen(value) : 0;
}

size_t Preferences::getString(const char *key, char *value, size_t maxLen)
{
    if (!_handle || !value || maxLen == 0) return 0;
    size_t len = maxLen;
    if (!OK(wm_nvs_get_str(H, key, value, &len))) return 0;
    return len;
}

String Preferences::getString(const char *key, String defaultValue)
{
    if (!_handle) return defaultValue;
    wm_nvs_type_t t;
    size_t len = 0;
    if (!OK(wm_nvs_get_info(H, key, &t, &len)) || len == 0) return defaultValue;
    char *buf = (char *)malloc(len + 1);
    if (!buf) return defaultValue;
    size_t l2 = len + 1;
    String r  = defaultValue;
    if (OK(wm_nvs_get_str(H, key, buf, &l2))) {
        buf[len] = 0;
        r        = String(buf);
    }
    free(buf);
    return r;
}

size_t Preferences::putBytes(const char *key, const void *value, size_t len)
{
    if (!_handle || _readOnly || !value || len == 0) return 0;
    return OK(wm_nvs_set_binary(H, key, value, len)) ? len : 0;
}

size_t Preferences::getBytesLength(const char *key)
{
    if (!_handle) return 0;
    wm_nvs_type_t t;
    size_t len = 0;
    return OK(wm_nvs_get_info(H, key, &t, &len)) ? len : 0;
}

size_t Preferences::getBytes(const char *key, void *buf, size_t maxLen)
{
    if (!_handle || !buf) return 0;
    size_t len = maxLen;
    if (!OK(wm_nvs_get_binary(H, key, buf, &len))) return 0;
    return len;
}
