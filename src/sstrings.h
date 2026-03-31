/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */
#ifndef MPRIS_SCROBBLER_SSTRINGS_H
#define MPRIS_SCROBBLER_SSTRINGS_H

#ifdef DEBUG
#include <assert.h>
#else
#ifndef assert
#define assert(A)
#endif
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#ifndef grrrs_std_alloc
#include <stdlib.h>
#define grrrs_std_alloc malloc
#endif

#ifndef grrrs_std_realloc
#include <stdlib.h>
#define grrrs_std_realloc realloc
#endif

#ifndef grrrs_std_free
#include <stdlib.h>
#define grrrs_std_free free
#endif

#ifndef GRRRS_OOM
#define GRRRS_OOM
#endif

#ifndef GRRRS_ERR
#define GRRRS_ERR(...)
#endif

#define internal static
#define _VOID(A) (NULL == (A))
#define _OKP(A) (NULL != (A))
#define _GRRRS_NULL_TOP_PTR (ptrdiff_t)(-2 * (ptrdiff_t)sizeof(uint32_t))

#define _grrr_sizeof(C) (sizeof(struct grrr_string) + ((size_t)(C+1) * sizeof(char)))

#define grrrs_from_string(A) (_VOID(A) ? \
    (char *)&_grrrs_new_empty(0)->data : \
    (char *)&_grrrs_new_from_cstring(A)->data)

#define grrrs_new(A) (char*)&(_grrrs_new_empty(A)->data)
#define grrrs_free(A) _grrrs_free(A)

// TODO(marius): investigate how this aligns
struct grrr_string {
    uint32_t len; /* currently used size of data[] */
    uint32_t cap; /* available size for data[] */
    char data[];
};

//typedef struct grrr_string grrrs;

internal struct grrr_string *_grrrs_ptr(char *s)
{
    return (void*)(s + _GRRRS_NULL_TOP_PTR);
}

internal void _grrrs_free(void *s)
{
    if (_VOID(s)) { return; }

    struct grrr_string *gs = _grrrs_ptr(s);

    if (s != (void*)&gs->data) {
        grrrs_std_free(s);
        return;
    }
    grrrs_std_free(gs);
}

static struct grrr_string *_grrrs_new_empty(const size_t cap)
{
    struct grrr_string *result = grrrs_std_alloc(_grrr_sizeof(cap));
    if (_VOID(result)) {
        GRRRS_OOM;
        return NULL;
    }

    result->len = 0;
    result->cap = (uint32_t)cap;
    for (size_t i = 0; i <= cap; i++) {
        result->data[i] = '\0';
    }

    return result;
}

internal uint32_t __strlen(const char *s)
{
    if (_VOID(s)) { return 0; }

    uint32_t result = 0;

    while (*s++ != '\0') { result++; }

    return result;
}

static void __cstrncpy(char *dest, const char *src, uint32_t len)
{
    if (_VOID(dest)) {
        return;
    }
    if (_VOID(src)) {
        return;
    }
    for (uint32_t i = 0; i < len; i++) {
        dest[i] = src[i];
    }
    dest[len] = '\0';
}

/* Copy at most dst_size-1 bytes from src into dst, always NUL-terminates. */
static void safe_strncpy(char *dst, const char *src, size_t dst_size)
{
    if (_VOID(dst) || dst_size == 0) { return; }
    if (_VOID(src)) { dst[0] = '\0'; return; }
    size_t i;
    for (i = 0; i < dst_size - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

/* Append src to dst, writing at most dst_size-strlen(dst)-1 bytes, always NUL-terminates. */
static void safe_strncat(char *dst, const char *src, size_t dst_size)
{
    if (_VOID(dst) || _VOID(src) || dst_size == 0) { return; }
    size_t dst_len = 0;
    while (dst_len < dst_size && dst[dst_len] != '\0') { dst_len++; }
    if (dst_len >= dst_size - 1) { return; }
    size_t remaining = dst_size - dst_len - 1;
    size_t i;
    for (i = 0; i < remaining && src[i] != '\0'; i++) {
        dst[dst_len + i] = src[i];
    }
    dst[dst_len + i] = '\0';
}

internal struct grrr_string *_grrrs_new_from_cstring(const char* s)
{
    const uint32_t len = __strlen(s);
    struct grrr_string *result = grrrs_std_alloc(_grrr_sizeof(len));
    if (_VOID(result)) {
        GRRRS_OOM;
        return (void*)_GRRRS_NULL_TOP_PTR;
    }

    result->len = len;
    result->cap = len;

    __cstrncpy(result->data, s, result->len);

    return result;
}

static uint32_t grrrs_cap(const char* s)
{
#ifdef DEBUG
    assert(_OKP(s));
#endif
    struct grrr_string *gs = _grrrs_ptr((char*)s);

#ifdef DEBUG
    assert(_OKP(gs));
    assert(__strlen(s) <= gs->cap);
#endif
    return gs->cap;
}

static uint32_t grrrs_len(const char* s)
{
#ifdef DEBUG
    assert(_OKP(s));
#endif
    struct grrr_string *gs = _grrrs_ptr((char*)s);

#ifdef DEBUG
    assert(_OKP(gs));
#endif

#ifdef DEBUG
    assert(gs->data == s);
    assert(__strlen(gs->data) == gs->len);
#endif
    return gs->len;
}

static int __grrrs_cmp(const struct grrr_string *s1, const struct grrr_string *s2)
{
    assert(_OKP(s1));
    assert(_OKP(s2));

    if (s1->len != s2->len) {
        return (int32_t)s1->len - (int32_t)s2->len;
    }
    for (uint32_t i = 0; i < s1->len; i++) {
        if (s1->data[i] == '\0') {
            GRRRS_ERR("NULL value in string data before length[%" PRIu32 ":%" PRIu32 "]", i, s1->len);
        }
        if (s2->data[i] == '\0') {
            GRRRS_ERR("NULL value in string data before length[%" PRIu32 ":%" PRIu32 "]", i, s2->len);
        }
        if (s1->data[i] != s2->data[i]) {
            return s1->data[i] - s2->data[i];
        }
    }
    return 0;
}

internal struct grrr_string *__grrrs_resize(struct grrr_string *gs, uint32_t new_cap)
{
#ifdef DEBUG
    assert(_OKP(gs));
#endif
    if (new_cap < gs->len) {
        GRRRS_ERR("new cap should be larger than existing length %" PRIu32 " \n", gs->len);
    }
    // TODO(marius): cover the case where new_cap is smaller than gs->len
    // and maybe when it's smaller than gs->cap
    gs = grrrs_std_realloc(gs, _grrr_sizeof(new_cap));
    if (_VOID(gs)) {
        GRRRS_OOM ;
        return (void*)_GRRRS_NULL_TOP_PTR;
    }
    if ((uint32_t)new_cap < gs->cap) {
        // ensure existing string is null terminated
        gs->data[new_cap] = '\0';
    }

    for (unsigned i = gs->cap; i <= new_cap; i++) {
        // ensure that the new capacity is zeroed
        gs->data[i] = '\0';
    }
    gs->cap = new_cap;

    return gs;
}

static void *_grrrs_resize(void *s, uint32_t new_cap)
{
    assert(_OKP(s));
    struct grrr_string *gs = _grrrs_ptr((char*)s);
    return __grrrs_resize(gs, new_cap)->data;
}

static void *_grrrs_trim_left(char *s, const char *c)
{
    char *result = s;
    char *to_trim = NULL;
    if (_VOID(s)) { return result; }

    struct grrr_string *gs = _grrrs_ptr(s);
    if (_VOID(gs)) { return result; }
    if (gs->len > 100000) {
        gs->len = __strlen(s);
    }

    if (_VOID(c)) {
        to_trim = _grrrs_new_from_cstring(" \t\r\n")->data;
    } else {
        to_trim = _grrrs_new_from_cstring(c)->data;
    }
    const uint32_t len_to_trim = grrrs_len(to_trim);

    int32_t trim_end = -1;
    uint32_t new_len = gs->len;
    for (uint32_t i = 0; i < gs->len; i++) {
        for (uint32_t j = 0; j < len_to_trim; j++) {
            const char t = to_trim[j];
            if (gs->data[i] == '\0') {
                break;
            }
            if (gs->data[i] == t) {
                new_len--;
                break;
            }
            if (j == len_to_trim - 1) {
                trim_end = (int32_t)i;
            }
        }
        if (trim_end >= 0) {
            break;
        }
    }
    if (new_len == gs->len) {
        goto _to_trim_free;
    }

    char *temp = grrrs_std_alloc((new_len+1)*sizeof(char));
    if (trim_end < 0) { trim_end = 0; }

    for (uint32_t k = 0; k < new_len; k++) {
        temp[k] = gs->data[(uint32_t)trim_end + k];
    }
    for (uint32_t k = 0; k < new_len; k++) {
        gs->data[k] = temp[k];
    }
    for (uint32_t k = new_len; k < gs->len; k++) {
        gs->data[k] = '\0';
    }
    gs->len = (uint32_t)new_len;
    grrrs_std_free(temp);

_to_trim_free:
    _grrrs_free(to_trim);

    return result;
}

static void *_grrrs_trim_right(char *s, const char *c)
{
    char *result = s;
    char *to_trim = NULL;
    if (_VOID(s)) { return result; }

    struct grrr_string *gs = _grrrs_ptr(s);
    if (_VOID(gs)) { return result; }
    if (gs->len > 100000) {
        gs->len = __strlen(s);
    }

    if (_VOID(c)) {
        to_trim = _grrrs_new_from_cstring("\r \t\n")->data;
    } else {
        to_trim = _grrrs_new_from_cstring(c)->data;
    }
    uint32_t len_to_trim = grrrs_len(to_trim);

    //assert(gs->len, len(gs->data);

    int8_t stop = 0;
    int32_t new_len = (int32_t)gs->len;
    for (int32_t i = (int32_t)gs->len - 1; i >= 0; i--) {
        for (uint32_t j = 0; j < len_to_trim; j++) {
            const char t = to_trim[j];
            // if we encounter \0 on the right side, we consider the string terminated
            // and we save the new length
            if (gs->data[i] == '\0') {
                new_len = i + 1;
                break;
            }
            if (gs->data[i] == t) {
                gs->data[i] = '\0';
                new_len--;
                break;
            }
            if (j == len_to_trim - 1) {
                stop = 1;
            }
        }
        if (stop) {
            break;
        }
    }
    if (new_len == (int32_t)gs->len) {
        goto _to_trim_free;
    }
    gs->len = (uint32_t)new_len;

_to_trim_free:
    _grrrs_free(to_trim);

    return result;
}

#define grrrs_trim(A, B) _grrrs_trim_right(_grrrs_trim_left((A), (B)), (B))

/* ---------- Capacity-aware string builder ---------- */

struct str_builder {
    char   *buf;
    size_t  len;
    size_t  cap;
};

/* Initialise a builder that writes into buf[0..cap] (buf must be cap+1 bytes). */
static void sb_init(struct str_builder *sb, char *buf, size_t cap)
{
    if (_VOID(sb)) { return; }
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
    if (_VOID(buf) || cap == 0) { return; }
    sb->buf = buf;
    sb->cap = cap;
    buf[0] = '\0';
}

/* Append src to the builder, silently stopping at capacity.
 * sb_init must have been called before the first sb_append. */
static void sb_append(struct str_builder *sb, const char *src)
{
    if (_VOID(sb) || _VOID(src)) { return; }
    if (sb->len >= sb->cap) { return; }
    const size_t remaining = sb->cap - sb->len;
    size_t i;
    for (i = 0; i < remaining && src[i] != '\0'; i++) {
        sb->buf[sb->len + i] = src[i];
    }
    sb->buf[sb->len + i] = '\0';
    sb->len += i;
}

/* Append a printf-formatted string, silently stopping at capacity. */
static void sb_append_fmt(struct str_builder *sb, const char *fmt, ...)
{
    if (_VOID(sb) || _VOID(fmt)) { return; }
    if (sb->len >= sb->cap) { return; }
    va_list args;
    va_start(args, fmt);
    const int written = vsnprintf(sb->buf + sb->len, sb->cap - sb->len + 1, fmt, args);
    va_end(args);
    if (written > 0) {
        const size_t added = ((size_t)written <= sb->cap - sb->len)
                             ? (size_t)written : sb->cap - sb->len;
        sb->len += added;
    }
}

/* Ensure the buffer is NUL-terminated and return it. */
static char *sb_finish(struct str_builder *sb)
{
    if (_VOID(sb)) { return NULL; }
    sb->buf[sb->len] = '\0';
    return sb->buf;
}

#endif // MPRIS_SCROBBLER_SSTRINGS_H
