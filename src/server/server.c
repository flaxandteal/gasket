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

#include "gasket/server.h"
#include "server-private.h"

gboolean
_gasket_server_close_socket(GasketServer *gasket)
{  
    char uuid_str[300];
    char socket_str[330];
    char tmpdir[300];
        
    //TODO: Implement close socket
    if (gasket->priv->socket_made)
    {
        close(gasket->priv->socket);

        uuid_unparse(*gasket->priv->uuid, uuid_str);
        sprintf(socket_str, GASKET_SERVER_SOCKET_PRINTF, gasket->priv->parent_pid, uuid_str);
        sprintf(tmpdir, GASKET_SERVER_TMPDIR_PRINTF, gasket->priv->parent_pid);

        if (unlink(socket_str) == -1)
        {
            fprintf(stderr,
                "Could not unlink closed socket %s\n",
                socket_str);

            return FALSE;
        }
        if (rmdir(tmpdir) == -1 && errno != ENOTEMPTY)
        {
            fprintf(stderr,
                "Could not remove directory %s\n",
                tmpdir);

            return FALSE;
        }

        gasket->priv->socket_made = FALSE;
    }
    return TRUE;
}

gboolean
_gasket_server_setenv(int parent_pid, uuid_t *uuid_ptr)
{
    char uuid_str[300], socket_str[330];
    uuid_unparse(*uuid_ptr, uuid_str);

    fprintf(stderr,
        "Setting up Gasket for child pty: UUID = %s\n",
        uuid_str);

    /* Set the Gasket socket path from the UUID */
    sprintf(socket_str, GASKET_SERVER_SOCKET_PRINTF, parent_pid, uuid_str);

    /* Set the environment variables within the PTY that called us */
    if (!g_setenv(GASKET_SERVER_ENVIRONMENT_SERVER_ID, uuid_str, TRUE) ||
        !g_setenv(GASKET_SERVER_ENVIRONMENT_SERVER_SOCKET, socket_str, TRUE)) {
        fprintf(stderr,
            "Could not set Gasket environment variables\n");
        return FALSE;
    }
    return TRUE;
}


enum {
    PROP_0,
    PROP_UUID,
};

static gboolean
gasket_server_initable_init (GInitable *initable,
                          GCancellable *cancellable,
                          GError **error)
{
    gboolean ret = TRUE;

    if (cancellable != NULL) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                             "Cancellable initialization not supported");
        return FALSE;
    }

    fprintf(stderr,
        "gasket_server_initable_init returning %s\n",
        ret ? "TRUE" : "FALSE");

    return ret;
}

static gboolean
gasket_server_initable_iface_init (GInitableIface *iface)
{
    iface->init = gasket_server_initable_init;

    return TRUE;
}

G_DEFINE_TYPE_WITH_CODE (GasketServer, gasket_server, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, gasket_server_initable_iface_init))

static void
gasket_server_init(GasketServer* gasket)
{
    GasketServerPrivate *priv;

    priv = gasket->priv = G_TYPE_INSTANCE_GET_PRIVATE (gasket, GASKET_TYPE_SERVER, GasketServerPrivate);

    uuid_t* uuid = (uuid_t*)malloc(sizeof(uuid_t));
    uuid_generate(*uuid);

    priv->uuid = uuid;
    priv->train_hash = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, g_free);
    priv->socket = 0;
    priv->socket_made = FALSE;
    priv->parent_pid = getpid();
}

static void
gasket_server_finalize(GObject *object)
{
    GasketServer *gasket = GASKET_SERVER (object);

    //FIXME: check for outstanding members
    g_hash_table_destroy(gasket->priv->train_hash);

    gasket_server_close(gasket);
}

static void
gasket_server_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    GasketServer *gasket = GASKET_SERVER (object);
    GasketServerPrivate *priv = gasket->priv;
    char uuid_str[300];

    switch (property_id)
    {
        case PROP_UUID:
            uuid_unparse(*priv->uuid, uuid_str);
            g_value_set_string(value, uuid_str);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
gasket_server_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    GasketServer *gasket = GASKET_SERVER (object);

    switch (property_id)
    {
        case PROP_UUID:
            gasket_server_set_uuid(gasket, g_value_get_string(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
gasket_server_class_init (GasketServerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private(object_class, sizeof(GasketServerPrivate));

    object_class->set_property = gasket_server_set_property;
    object_class->get_property = gasket_server_get_property;
    object_class->finalize     = gasket_server_finalize;

    /**
     * GasketServer:uuid:
     *
     * The UUID reference for the current Gasket instance - this is the
     * basis of finding resources, such as a Unix socket, related to a
     * specific Gasket train.
     */
    g_object_class_install_property(object_class, PROP_UUID,
        g_param_spec_string("uuid", NULL, NULL, "", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)
    );
}

/* Public API */

/**
 * gasket_server_error_quark:
 *
 * Error domain for GASKET Gasket errors. These errors will be from the #GasketServerError
 * enum. See #GError for further details.
 *
 * Returns: the error domain for GASKET Gasket errors
 */
GQuark
gasket_server_error_quark(void)
{
    static GQuark error_quark = 0;

    if (G_UNLIKELY (error_quark == 0))
        error_quark = g_quark_from_static_string("gasket-server-error");

    return error_quark;
}

/**
 * gasket_server_new:
 * @error: (allow-none): return location for a #GError, or %NULL
 * 
 * Creates a new gasket and generates a UUID for it.
 *
 * Returns: (transfer full): a new #GasketServer, or %NULL on error with @error filled in
 */
GasketServer*
gasket_server_new(GError **error)
{
    GasketServer *ret = NULL;

    ret = g_initable_new (GASKET_TYPE_SERVER,
                          NULL /* (i.e. not cancellable) */,
                          error,
                          NULL);

    return ret;
}

/**
 * gasket_server_set_uuid:
 * @gasket: a #GasketServer
 * @uuid: a UUID as a string. Unsets if empty.
 *
 * Set the UUID for the gasket, as used in sockets, etc..
 *
 * Returns: (transfer none): a boolean indicating success
 */
gboolean
gasket_server_set_uuid(GasketServer *gasket, const gchar* uuid_str)
{  
    int uuid_ret = 0;

    GasketServerPrivate *priv = gasket->priv;

    if (strlen(uuid_str) > 0)
    {
        if (priv->uuid == NULL)
            priv->uuid = (uuid_t*)malloc(sizeof(uuid_t));

        uuid_ret = uuid_parse(uuid_str, *priv->uuid);

        if (uuid_ret != 0)
        {
            fprintf(stderr,
                "Could not set Gasket UUID : %s\n",
                uuid_str);

            g_object_notify(G_OBJECT(gasket), "uuid");
            priv->uuid = NULL;

            return FALSE;
        }
    }
    else
    {
        //FIXME: deallocate old uuid
        if (priv->uuid != NULL)
            free(priv->uuid);

        priv->uuid = NULL;
    }

    //FIXME: I think this needs to be changed to update the _child_ process env.
    //_gasket_server_setenv(priv->uuid);

    g_object_notify(G_OBJECT(gasket), "uuid");
    return TRUE;
}

gboolean
_gasket_server_make_tmpdir(GasketServer* gasket)
{
    char tmpdir[300];
    
    sprintf(tmpdir, GASKET_SERVER_TMPDIR_PRINTF, gasket->priv->parent_pid);

    if (mkdir(tmpdir, S_IRWXU) == -1 && errno != EEXIST)
    {
        g_warning("Error (%s) creating temporary directory.",
              g_strerror(errno));
        fprintf(stderr,
          "Cannot create a temporary directory\n");
        return FALSE;
    }
    return TRUE;
}

/**
 * gasket_server_make_socket:
 * @gasket: a #GasketServer
 *
 * Sets up a new socket for the current UUID.
 *
 * Returns: (transfer none): a boolean indicating success
 */
gboolean
gasket_server_make_socket(GasketServer* gasket)
{
    GasketServerPrivate *priv = gasket->priv;

    gchar uuid_str[300];
    int socket_ret;
    unsigned int gasket_socket;
    struct sockaddr_un local;
    gchar socket_str[330];

    if (priv->uuid == NULL)
    {
        fprintf(stderr,
          "Cannot make socket without UUID set\n");
        return FALSE;
    }

    /* Create a temporary directory for this (parent) process */
    _gasket_server_make_tmpdir(gasket);

    /* Get a string representation of the UUID */
    uuid_unparse(*priv->uuid, uuid_str);

    fprintf(stderr,
      "Making socket for Gasket with UUID: %s\n",
      uuid_str);

    /* Set up the socket itself
     *
     * With thanks for sockets tips to...
     * http://beej.us/guide/bgipc/output/html/multipage/unixsock.html */

    /* Name socket after UUID */
    sprintf(socket_str, GASKET_SERVER_SOCKET_PRINTF, priv->parent_pid, uuid_str);
    //socket_ret = socket(AF_UNIX, SOCK_STREAM, 0);
    if ((socket_ret = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        g_warning("Error (%s) creating new Unix socket.",
              g_strerror(errno));
        fprintf(stderr,
          "Cannot create a new Unix socket\n");
        return FALSE;
    }
    gasket_socket = (unsigned int)socket_ret;

    /* Working with Unix sockets here */
    local.sun_family = AF_UNIX;

    /* Ensure we start afresh */
    strcpy(local.sun_path, socket_str);
    unlink(local.sun_path);

    /* Attempt to bind the socket to a local address */
    if (bind(gasket_socket, (struct sockaddr*)&local, sizeof(struct sockaddr_un)) == -1) {
        g_warning("Error (%s) binding unix socket.",
              g_strerror(errno));
        fprintf(stderr,
          "Cannot bind new Unix socket\n");
        return FALSE;
    }

    priv->socket = gasket_socket;
    priv->socket_made = TRUE;

    /* Mark this socket as for listening on */
    listen(gasket_socket, 5);

    return TRUE;
}

/**
 * gasket_server_launch_listen:
 * @gasket: a #GasketServer
 *
 * Start the listener on the socket.
 */
void
gasket_server_launch_listen(GasketServer *gasket)
{
    fprintf(stderr,
      "Launching a listener thread for Gasket\n");

    /* Start the listener */
    g_thread_new("gasket_station", (GThreadFunc)gasket_server_listen, gasket);
}

gboolean
_gasket_server_update_svg(gpointer user_data)
{
    struct _GasketServerUpdateSVGData *update_data =
        (struct _GasketServerUpdateSVGData*)user_data;

    GasketServerPrivate *priv = update_data->gasket->priv;

    GasketServerTrain *train = (GasketServerTrain*)g_hash_table_lookup(priv->train_hash, &update_data->connection_index);
    gint *k;

    if (train == NULL)
    {
        train = (GasketServerTrain*)malloc(sizeof(GasketServerTrain));
        train->svg = g_string_new("");
        train->rsvg_handle = NULL;

        k = g_new(gint, 1);
        *k = update_data->connection_index;
        g_hash_table_insert(priv->train_hash, k, train);
    }

    /* Inject the SVG string (done here as thread-safe) */
    g_string_assign(train->svg, update_data->svg->str);

    /* Flag the SVG as being new */
    train->invalid = TRUE;

    /* Tidy up carrier */
    g_string_free(update_data->svg, TRUE);
    free(update_data);

    /* Invalidate the terminal */
    priv->invalidation_function(priv->invalidation_data);

    return FALSE;
}

/**
 * _gasket_server_reset_train
 * @key:a #gpointer to the key
 * @value: a #gpointer to the value
 * @data: a #gpointer to a #GasketServer
 *
 * This function resets a train in the hash table. It assumes the caller
 * handles terminal invalidation. This is a #GHFunc.
 */
void
_gasket_server_reset_train(gpointer key, gpointer value, gpointer data)
{
    GasketServerTrain *train = (GasketServerTrain*)value;

    g_string_truncate(train->svg, 0);
    train->invalid = TRUE;
}

/**
 * gasket_server_reset_all:
 * @gasket: a #GasketServer
 *
 * Reset all trains to blank. Used to forcibly clear the screen.
 * Returns FALSE to allow use as a GThreadFunc.
 */
gboolean
gasket_server_reset_all(GasketServer* gasket)
{
    GasketServerPrivate *priv = gasket->priv;

    /* Reset via foreach */
    g_hash_table_foreach(priv->train_hash, _gasket_server_reset_train, gasket);

    /* Invalidate the terminal */
    priv->invalidation_function(priv->invalidation_data);

    return FALSE;
}

/**
 * _gasket_server_handle_new_connection:
 * @gasket: a #GasketServer
 * @fd: a file descriptor for the new connection
 *
 * Handle connections from the socket and read in svg.
 */
gpointer
_gasket_server_handle_new_connection(struct _GasketServerConnectionData *data)
{
    GasketServer *gasket = data->gasket;
    int gasket_socket_conn = data->connection_index;

    GError* err = NULL;

    gint buffer_len = 128;
    gchar read_buffer[buffer_len+1], buffer[2*buffer_len+2], layover_buffer[2*buffer_len+2];
    gchar** train_set;
    gchar* eol;
    gsize bytes_read;
    gint train_set_index;

    GIOChannel* gasket_channel;
    struct _GasketServerUpdateSVGData* update_data;
    struct _GasketServerConnectionData* destroy_data;

    GString* svg = g_string_new("");
    layover_buffer[0] = '\0';

    free(data);

    /* Start reading in SVG */
    gasket_channel = g_io_channel_unix_new(gasket_socket_conn);
    g_io_channel_set_encoding(gasket_channel, NULL, &err);
    g_io_channel_set_buffered(gasket_channel, FALSE);

    /* Read this chunk into buffer */
    while (g_io_channel_read_chars(gasket_channel, read_buffer, buffer_len, &bytes_read, &err) != G_IO_STATUS_EOF) {
        read_buffer[bytes_read] = '\0';
        sprintf(buffer, "%s%s", layover_buffer, read_buffer);
        train_set = g_strsplit(buffer, GASKET_SERVER_CABOOSE, -1);
        train_set_index = -1;
        do
        {
            eol = train_set[++train_set_index];
        }
        while (eol != NULL);

        if (train_set_index == 0)
            continue;

        /* If we find a caboose, process and continue */
        if (train_set_index > 1) {
            if (train_set_index == 2) {
                g_string_append_printf(svg, "%s", train_set[0]);
                FILE *f=fopen("/tmp/testc", "a"); fprintf(f, "%s*\n", train_set[0]); fclose(f);//RMV
            }
            else {
                g_string_printf(svg, "%s", train_set[train_set_index - 3]);
                FILE *f=fopen("/tmp/testc", "a"); fprintf(f, "<%s>\n", train_set[train_set_index - 3]); fclose(f);//RMV
            }

            /* Clean whitespace */
            //FIXME: based on its code definition, this updates GString appropriately - it seems abusive but no better alternative presents itself
            g_strstrip(svg->str);
            svg->len = strlen(svg->str);

            /* Check for station command */
            if (g_strcmp0(svg->str, GASKET_SERVER_STATION_RESET) == 0) {
                /* Reset all trains */
                g_main_context_invoke(NULL, (GSourceFunc)gasket_server_reset_all, gasket);
            }
            else {
                /* Prepare a temporary variable to handle update info */
                update_data =
                    (struct _GasketServerUpdateSVGData*)malloc(sizeof(struct _GasketServerUpdateSVGData));

                /* Inject the SVG string (hold here for the moment) */
                update_data->svg = g_string_new(svg->str);

                update_data->gasket = gasket;

                /* Use the file descriptor as an index for the SVG */
                update_data->connection_index = gasket_socket_conn;

                /* Break into main thread and force redraw */
                g_main_context_invoke(NULL, _gasket_server_update_svg, update_data);
            }

            /* Wipe the SVG string to start the next chunk, beginning with any left-over chars */
            g_string_truncate(svg, 0);
            sprintf(layover_buffer, "%s", train_set[train_set_index-1]);
        } else {
            /* Add on the new content to the SVG string */
            eol = train_set[0] + strlen(layover_buffer);
            g_string_append_printf(svg, "%s", layover_buffer);

            sprintf(layover_buffer, "%s", eol);
        }
        g_strfreev(train_set);
    }

    if (err != NULL) {
        fprintf(stderr, "Looping: %s\n", err->message);
        g_error_free(err);
    }

    g_string_free(svg, TRUE);

    destroy_data = g_new(struct _GasketServerConnectionData, 1);
    destroy_data->gasket = gasket;
    destroy_data->connection_index = gasket_socket_conn;
    g_main_context_invoke(NULL, _gasket_server_close_connection, destroy_data);

    return NULL;
}


/**
 * _gasket_server_close_connection
 * data: a struct _GasketServerConnectionData containing a pointer to the #GasketServer and the connection index to be destroyed
 */
gboolean
_gasket_server_close_connection(gpointer data)
{
    struct _GasketServerConnectionData* conn_data = (struct _GasketServerConnectionData*)data;
    GasketServerPrivate *priv = conn_data->gasket->priv;
    gint connection_index = conn_data->connection_index;

    GasketServerTrain *train = g_hash_table_lookup(priv->train_hash, &connection_index);

    if (train == NULL)
        return FALSE;

    close(connection_index);

    g_string_free(train->svg, TRUE);

    if (train->rsvg_handle != NULL)
        g_object_unref(train->rsvg_handle);

    g_hash_table_remove(priv->train_hash, &connection_index);

    printf("CLOSED %d\n", connection_index);

    priv->invalidation_function(priv->invalidation_data);

    return FALSE;
}

/**
 * gasket_server_listen:
 * @gasket: a #GasketServer
 *
 * Accept connections from the socket.
 */
gpointer
gasket_server_listen(GasketServer *gasket)
{
    int socket_ret;
    unsigned int local_len, gasket_socket_conn;
    struct _GasketServerConnectionData *conn_data;
    struct sockaddr_un remote;

    /* Enter acceptance loop */
    for (;;) {
        /* Accept data via socket */
        local_len = sizeof(struct sockaddr_un);
        socket_ret = accept(gasket->priv->socket, (struct sockaddr*)&remote, &local_len);

        if (socket_ret == -1)
        {
            g_warning("Could not accept requests from socket - %s\n",
              g_strerror(errno));
            return NULL;
        }
        gasket_socket_conn = (unsigned int)socket_ret;

        conn_data = (struct _GasketServerConnectionData*)malloc(sizeof(struct _GasketServerConnectionData));
        conn_data->gasket = gasket;
        conn_data->connection_index = gasket_socket_conn;

        g_thread_new("gasket_platform", (GThreadFunc)_gasket_server_handle_new_connection, conn_data);

    }

    return NULL;
}

void
gasket_server_child_setup(GasketServer *gasket)
{
    //FIXME: error tests before dereference
    _gasket_server_setenv(gasket->priv->parent_pid, gasket->priv->uuid);
}

void
gasket_server_update_table(GasketServer *gasket, GHashTable* table)
{
    char uuid_str[300], socket_str[330];
    //FIXME: error tests before dereference
    uuid_t *uuid = gasket->priv->uuid;

    /* Set the environment variables within the PTY that called us */
    if (!_gasket_server_setenv(gasket->priv->parent_pid, uuid)) {
        fprintf(stderr,
            "Could not set environment variables when updating\n");
        return;
    }

    uuid_unparse(*uuid, uuid_str);

    /* Set the Gasket socket path from the UUID */
    sprintf(socket_str, GASKET_SERVER_SOCKET_PRINTF, gasket->priv->parent_pid, uuid_str);

    /* Update the hash table with the relevant environment variables */
    g_hash_table_replace (table, g_strdup (GASKET_SERVER_ENVIRONMENT_SERVER_ID), g_strdup (uuid_str));
    g_hash_table_replace (table, g_strdup (GASKET_SERVER_ENVIRONMENT_SERVER_SOCKET), g_strdup (socket_str));
}

void
gasket_server_set_invalidation_function (GasketServer *gasket, GSourceFunc function, gpointer user_data)
{
    g_return_if_fail( GASKET_IS_SERVER(gasket) );

    gasket->priv->invalidation_function = function;
    gasket->priv->invalidation_data = user_data;
}

void
gasket_server_set_target_extents(GasketServer *gasket, int row, int col, int row_count, int col_count, long width, long height)
{
    GasketServerPrivate *priv = gasket->priv;
    struct _GasketServerTargetExtents *extents = &priv->extents;

    extents->row = row;
    extents->col = col;
    extents->row_count = row_count;
    extents->col_count = col_count;
    extents->width = width;
    extents->height = height;
}

void
gasket_server_close(GasketServer *gasket)
{
    _gasket_server_close_socket(gasket);
}

/**
 * gasket_server_pass_thru
 * @gasket: a #GasketServer
 *
 * Return the combined SVG in storage. This should
 * be freed by the caller.
 */
char*
gasket_server_pass_thru(GasketServer *gasket)
{
    GasketServerPrivate *priv = gasket->priv;

    GHashTableIter iter;
    gpointer k, v;
    gint connection_index;
    GasketServerTrain* train;
    char* combined_svg_string;

    g_hash_table_iter_init(&iter, priv->train_hash);

    GString *svg;
    GString *combined_svg = g_string_new(NULL);

    g_string_append(combined_svg, GASKET_SERVER_CABOOSE);

    while (g_hash_table_iter_next(&iter, &k, &v))
    {
        connection_index = *(int*)k;
        train = (GasketServerTrain*)v;
        svg = train->svg;

        if (svg != NULL && train->invalid && svg->len >0)
            g_string_append(combined_svg, svg->str);
    }

    g_string_append(combined_svg, GASKET_SERVER_CABOOSE);

    combined_svg_string = g_strdup(combined_svg->str);

    g_string_free(combined_svg, TRUE);

    return combined_svg_string;
}
