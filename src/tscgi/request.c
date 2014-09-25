#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tscgi/errors.h"
#include "tscgi/netstring.h"
#include "tscgi/header.h"
#include "tscgi/request.h"
#include "tscgi/buffer.h"

int parse_request(struct buffer *stream, struct request *request,
                  size_t body_max_len)
{
    int ret;
    char *headers_buffer, *body_buffer;
    size_t headers_len, body_len;
    struct header_list *headers;

    /* parse netstring */
    ret = parse_netstring(stream, &headers_buffer, &headers_len);
    if (ret != NETSTRING_OK)
        return ret;

    /* create header list and parse headers */
    headers = create_header_list();
    parse_headers((const char *) headers_buffer, headers_len, headers);

    /* the first header must be content length */
    if (strcmp(headers->item.name, "CONTENT_LENGTH") != 0)
        return REQUEST_ERROR_CONTENT_LENGTH;
    body_len = atoi(headers->item.value);
    /* failed to convert digest */
    if (body_len <= 0 && strcmp(headers->item.value, "0") != 0)
        return REQUEST_ERROR_CONTENT_LENGTH;

    /* read body */
    if (body_len > body_max_len)
        return REQUEST_ERROR_BODY_TOO_MAX;
    if (body_len > 0)
    {
        if ((buffer_len(stream) - buffer_pos(stream)) < body_len)
            return REQUEST_ERROR_BROKEN_PIPE;
        body_buffer = (char *) malloc(sizeof(char) * body_len);
        memcpy(body_buffer, buffer_current(stream), body_len);
        /* be safe: strange content may be read out buffer size in some
         * operation system, such as Mac OSX. */
        body_buffer[body_len] = '\0';
    }
    else
    {
        body_buffer = EMPTY_BUFFER;
    }

    /* save public attributes */
    request->headers = headers;
    request->body = body_buffer;

    /* save private resource handle */
    request->_headers_buffer = headers_buffer;

    return REQUEST_OK;
}

int destory_request(struct request *request)
{
    if (request->headers)
        destory_header_list(request->headers);
    if (request->body && request->body[0])
        free(request->body);
    if (request->_headers_buffer && request->_headers_buffer[0])
        free(request->_headers_buffer);

    return REQUEST_OK;
}
