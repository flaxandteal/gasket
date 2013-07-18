/* -*- Mode: c; c-basic-offset: 4 -*- 
 *
 * GOBject Introspection Tutorial 
 * 
 * Written in 2013 by Simon Kågedal Reimer <skagedal@gmail.com>
 *
 * To the extent possible under law, the author have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * CC0 Public Domain Dedication:
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include <glib.h>
#include <girepository.h>
#include "gasket-train.h"

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
