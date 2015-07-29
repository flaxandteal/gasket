/* -*- Mode: c; c-basic-offset: 4 -*- 
 *
 * Copyright (C) 2013- Phil Weir
 *
 * This file is based on GOBject Introspection Tutorial by
 * Simon Kågedal Reimer <skagedal@gmail.com> (2013).
 *
 * This file, as part of Gasket is placed in the
 * public domain (CC0-PD).
 *
 * CC0 Public Domain Dedication:
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include <glib.h>
#include <girepository.h>
#include "gasket/client.h"

int main (int argc, char *argv[]) 
{
    GOptionContext *ctx;
    GError *error = NULL;
    gchar* gasket_socket_file;
    gint connect_result;
    const gchar* miniview_id;
    
    GasketTrain *train;

    ctx = g_option_context_new(NULL);
    g_option_context_add_group (ctx, g_irepository_get_option_group ());

    if (!g_option_context_parse (ctx, &argc, &argv, &error)) {
        g_print("gasket_reset: %s\n", error->message);
        return 1;
    }

    train = gasket_train_new ();

    connect_result = gasket_train_station_connect(train);
    gasket_train_station_reset(train);

    gasket_train_redisplay(train);

    return 0;
}
