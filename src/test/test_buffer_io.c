#include <stdlib.h>
#include <string.h>

#include "../rdata.h"

#include "test_buffer.h"
#include "test_buffer_io.h"

int rt_open_handler(const char *path, void *io_ctx) {
    return 0;
}

int rt_close_handler(void *io_ctx) {
    return 0;
}

rdata_off_t rt_seek_handler(rdata_off_t offset,
        rdata_io_flags_t whence, void *io_ctx) {
    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;
    rdata_off_t newpos = -1;
    if (whence == RDATA_SEEK_SET) {
        newpos = offset;
    } else if (whence == RDATA_SEEK_CUR) {
        newpos = buffer_ctx->pos + offset;
    } else if (whence == RDATA_SEEK_END) {
        newpos = buffer_ctx->buffer->used + offset;
    }

    if (newpos < 0)
        return -1;

    if (newpos > buffer_ctx->buffer->used)
        return -1;

    buffer_ctx->pos = newpos;
    return newpos;
}

ssize_t rt_read_handler(void *buf, size_t nbytes, void *io_ctx) {
    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;
    ssize_t bytes_copied = 0;
    ssize_t bytes_left = buffer_ctx->buffer->used - buffer_ctx->pos;
    if (nbytes <= bytes_left) {
        memcpy(buf, buffer_ctx->buffer->bytes + buffer_ctx->pos, nbytes);
        bytes_copied = nbytes;
    } else if (bytes_left > 0) {
        memcpy(buf, buffer_ctx->buffer->bytes + buffer_ctx->pos, bytes_left);
        bytes_copied = bytes_left;
    }
    buffer_ctx->pos += bytes_copied;
    return bytes_copied;
}

rdata_error_t rt_update_handler(long file_size, rdata_progress_handler progress_handler,
        void *user_ctx, void *io_ctx) {
    if (!progress_handler)
        return RDATA_OK;

    rt_buffer_ctx_t *buffer_ctx = (rt_buffer_ctx_t *)io_ctx;

    if (progress_handler(1.0 * buffer_ctx->pos / buffer_ctx->buffer->used, user_ctx))
        return RDATA_ERROR_USER_ABORT;

    return RDATA_OK;
}
