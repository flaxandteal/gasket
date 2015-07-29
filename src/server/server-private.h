/* -*- Mode: c; c-basic-offset: 4 -*- 
 *
 * Copyright (C) 2013- Phil Weir
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, distribute with modifications, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

G_BEGIN_DECLS

struct _GasketServerConnectionData {
    GasketServer *gasket;
    unsigned int connection_index;
};

struct _GasketServerTargetExtents {
    int row;
    int col;

    int row_count;
    int col_count;

    long width;
    long height;
};

typedef struct _GasketServerTrain {
    GString* svg;
    gboolean invalid;
    void* rsvg_handle;
} GasketServerTrain;

struct _GasketServerPrivate {
    uuid_t* uuid;

    struct _GasketServerTargetExtents extents;
    GHashTable* train_hash;

    GSourceFunc invalidation_function;
    gpointer invalidation_data;

    unsigned int socket;
    gboolean socket_made;
    int parent_pid;
};

struct _GasketServerUpdateSVGData {
    int connection_index;
    GString* svg;
    GasketServer* gasket;
};

gboolean _gasket_server_close_socket(GasketServer *gasket);
gboolean _gasket_server_setenv(int parent_pid, uuid_t *uuid_ptr);
gboolean _gasket_server_make_tmpdir(GasketServer* gasket);
gboolean _gasket_server_update_svg(gpointer user_data);
gpointer _gasket_server_handle_new_connection(struct _GasketServerConnectionData *data);
gboolean _gasket_server_close_connection(gpointer data);

G_END_DECLS
