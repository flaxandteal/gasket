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

#include <glib.h>
#include <gio/gio.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <uuid/uuid.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <cairo.h>
#include <librsvg/rsvg.h>

#include "gasket/server.h"
#include "server-private.h"

/**
 * gasket_server_paint_overlay:
 * @gasket: a #GasketServer
 *
 * Paint an overlay during the cairo write.
 */
void
gasket_server_paint_overlay(GasketServer *gasket, cairo_t* cr)
{
    GasketServerPrivate *priv = gasket->priv;

    struct _GasketServerTargetExtents *extents = &priv->extents;

    RsvgHandle *handle, *new_handle;
    GString *svg;
    RsvgDimensionData dims;

    GError *err;
    FILE* err_back; 
    gchar err_back_filename[GASKET_SERVER_MAXFILEPATH], uuid_str[1024], tmpdir[GASKET_SERVER_MAXFILEPATH];

    GHashTableIter iter;
    gpointer k, v;
    gint connection_index;
    GasketServerTrain* train;

    /* Only bother if we are initialized */
    if (priv->uuid == NULL || g_hash_table_size(priv->train_hash) == 0)
        return;

    /* Locate the current tmpdir */
    uuid_unparse(*gasket->priv->uuid, uuid_str);
    sprintf(tmpdir, GASKET_SERVER_TMPDIR_PRINTF, gasket->priv->parent_pid);
    sprintf(err_back_filename, "%s/%s", tmpdir, "gasket_errback.svg");

    g_hash_table_iter_init(&iter, priv->train_hash);

    while (g_hash_table_iter_next(&iter, &k, &v))
    {
        connection_index = *(int*)k;
        train = (GasketServerTrain*)v;

        /* Pick out SVG content and handle from Gasket object */
        handle = (RsvgHandle*)train->rsvg_handle;
        new_handle = NULL;
        svg = train->svg;

        err = NULL;
        /* Only regenerate the RSVG if marked as invalid */
        if (svg != NULL && train->invalid) {
            if (svg->len > 0) {
                //TODO: add in debug flag
                // fprintf(stderr,
                //     "Re-parsing Train #%d (=connection fd) as flagged\n",
                //     connection_index);
                new_handle = rsvg_handle_new_from_data(svg->str, strlen(svg->str), &err);
                if (err == NULL) {
                    if (handle != NULL) {
                        g_object_unref(handle);
                    }
                    handle = new_handle;
                } else {
                    //TODO: this really needs tidied and secured
                    fprintf(stderr, "Creating handle problem: %s\n", err->message);
                    g_error_free(err);
                    err = NULL;
                    err_back = fopen(err_back_filename, "w");
                    if (err_back == NULL) {
                        perror("Couldn't save problematic SVG backup");
                    } else {
                        fprintf(err_back, "%s", svg->str);
                        fclose(err_back);
                    }
                }
            } else {
                if (handle != NULL) {
                    g_object_unref(handle);
                }
                handle = NULL;
            }

            train->invalid = FALSE;
        }

        /* Get dimension data to rescale appropriately */
        if (handle != NULL) {
            err = NULL;
            rsvg_handle_get_dimensions(handle, &dims);

            cairo_save(cr);
            cairo_scale(cr, extents->width, extents->height);
            rsvg_handle_render_cairo(handle, cr);
            cairo_restore(cr);
        }
        train->rsvg_handle = handle;
    }
}
