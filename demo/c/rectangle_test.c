/* -*- Mode: c; c-basic-offset: 4 -*- 
 *
 * Written 2013- by Phil Weir
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
    gasket_train_station_send(train, "");
    gasket_train_station_reset(train);

    miniview_id = gasket_train_add_carriage(train, "Miniview");
    printf("MINIVIEW ID : %s\n", miniview_id);
    gasket_train_update_carriage(train, miniview_id,
            "<rect x='0' y='0' width='4' height='4' style='fill:rgb(255,0,0);fill-opacity:1.'/>"
    );

    gasket_train_redisplay(train);

    gasket_train_flush(train);

    sleep(1);

    gasket_train_shutdown(train);

    return 0;
}
