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

#include <semaphore.h>
#include <stdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>
#include <libxml/parser.h>
#include "gasket-train.h"

/**
 * SECTION: gasket-train
 * @short_description: A Gasket Train - GUI<->CLI
 *
 * The #GasketTrain is a linkage from a CLI program to the eventual GUI Gasket server.
 */

G_DEFINE_TYPE (GasketTrain, gasket_train, G_TYPE_OBJECT)

#define GASKET_TRAIN_GET_PRIVATE(o)	\
    (G_TYPE_INSTANCE_GET_PRIVATE ((o), GASKET_TRAIN_TYPE, GasketTrainPrivate))

/* Downtime between checking the output buffer */
#define GASKET_SLEEP 1

struct _GasketTrainCarriage {
    gchar *name;
    xmlNodePtr node;
};

typedef struct _GasketTrainCarriage GasketTrainCarriage;

struct _GasketTrainPending {
    sem_t sem;
    gchar test;
    gchar* buffer;
    GOutputStream* stream;
};

struct _GasketTrainPrivate {
    gchar *track;		 /* The socket connection between GUI and CLI */
    gboolean active;             /* On/off toggle */
    gboolean connected;          /* Whether the socket has been connected to the server */
    GOutputStream* stream;       /* Socket from here to server */
    GHashTable* carriages;       /* Individual components added to this channel (train) */
    guint carriage_next_free_id; /* An unassigned ID */

    struct _GasketTrainPending* pending; /* Details of pending buffer */
};

enum
{
    PROP_0,

    PROP_ACTIVE,

    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/**
 * Main loop handling
 */
gboolean _gasket_train_watch_pending(struct _GasketTrainPending *pending);
gboolean _gasket_train_main_loop_init(void* data);
gboolean _gasket_train_send_real(struct _GasketTrainPending *pending);
GMainContext* gasket_main_context = NULL;
GMainLoop* gasket_main_loop = NULL;

gboolean
_gasket_train_main_loop_init(void* data)
{
    if (gasket_main_loop != NULL)
        return;

    gasket_main_context = g_main_context_default();
    gasket_main_loop = g_main_loop_new(gasket_main_context, FALSE);
    g_main_loop_run(gasket_main_loop);
}

static void
gasket_train_init (GasketTrain *object)
{
    GasketTrainPrivate *priv = GASKET_TRAIN_GET_PRIVATE (object);
    struct _GasketTrainPending* pending;

    priv->track  = NULL;
    priv->active = FALSE;
    priv->connected = FALSE;
    priv->stream = NULL;
    priv->carriages = g_hash_table_new(g_str_hash, g_str_equal);
    priv->carriage_next_free_id = 0;

    pending = (struct _GasketTrainPending*)malloc(sizeof(struct _GasketTrainPending));
    priv->pending = pending;
    pending->buffer = NULL;
    pending->stream = NULL;
    pending->test = 'T';
    sem_init(&pending->sem, 0, 1);
    g_thread_new("Main Loop", _gasket_train_main_loop_init, NULL);
}

static void
gasket_train_finalize (GObject *object)
{
    GasketTrainPrivate *priv = GASKET_TRAIN_GET_PRIVATE (object);

    g_free (priv->track);
    g_hash_table_destroy (priv->carriages);

    G_OBJECT_CLASS (gasket_train_parent_class)->finalize (object);
}

static void
gasket_train_set_property (GObject      *object,
			   guint         property_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
    GasketTrainPrivate *priv = GASKET_TRAIN_GET_PRIVATE (object);

    switch (property_id) {
    case PROP_ACTIVE:
	priv->active = g_value_get_boolean(value);
        gasket_train_clear (GASKET_TRAIN(object));
	break;

    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	break;
    }
}

static void
gasket_train_get_property (GObject    *object,
			   guint       property_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
    GasketTrainPrivate *priv = GASKET_TRAIN_GET_PRIVATE (object);

    switch (property_id) {
    case PROP_ACTIVE:
	g_value_set_boolean (value, priv->active);
	break;

    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	break;
    }
}

static void
gasket_train_class_init (GasketTrainClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = gasket_train_set_property;
    object_class->get_property = gasket_train_get_property;
    object_class->finalize = gasket_train_finalize;

    /**
     * GasketTrain:active:
     *
     * On/off toggle (clears on change).
     */
    obj_properties[PROP_ACTIVE] = 
	g_param_spec_boolean ("active",
			      "Active",
			      "On/off toggle.",
			      TRUE,
			      G_PARAM_READWRITE |
			      G_PARAM_CONSTRUCT);

    g_object_class_install_properties (object_class,
				       N_PROPERTIES,
				       obj_properties);

    g_type_class_add_private (object_class, sizeof (GasketTrainPrivate));
}


/**
 * gasket_train_new:
 *
 * Allocates a new #GasketTrain.
 *
 * Return value: a new #GasketTrain.
 */
GasketTrain*
gasket_train_new ()
{
    GasketTrain *train;

    train = g_object_new (GASKET_TRAIN_TYPE, NULL);
    return train;
}

/**
 * gasket_train_station_reset:
 * @train: a #GasketTrain
 *
 * Clears all trains in the station, that is, tells the GUI server to reset
 * all input streams, thereby clearing the screeen. NB: this is unique in
 * affecting output from all other processes to the same GUI, so use with
 * appropriate restraint - it is the moral equivalent of the Unix 'clear'
 * command.
 *
 * Return value: nothing.
 */
void
gasket_train_station_reset (GasketTrain *train)
{
    g_return_if_fail (train != NULL);

    gasket_train_station_send(train, "__GASKET_STATION_RESET__");
}

/**
 * gasket_train_station_send:
 * @train: a #GasketTrain
 *
 * Transfer a given string to the Gasket station (GUI-end server)
 *
 * Return value: nothing.
 */
void
gasket_train_station_send (GasketTrain *train, const gchar* gasket_content)
{
    GasketTrainPrivate *priv;
    GError* error = NULL;
    gssize size;
    gsize to_send;
    gchar* buffer = g_new(gchar, 128 + strlen(gasket_content));

    if (buffer == NULL)
        return;

    g_return_if_fail (train != NULL);

    priv = GASKET_TRAIN_GET_PRIVATE (train);
    g_return_if_fail (priv->connected);

    sprintf(buffer, "\n__GASKET_CABOOSE__\n%s\n__GASKET_CABOOSE__\n", gasket_content);

    _gasket_train_set_pending(train, buffer);
}

gboolean
_gasket_train_watch_pending(struct _GasketTrainPending *pending)
{
    if (gasket_main_loop == NULL)
        g_thread_new("Main Loop", _gasket_train_main_loop_init, NULL);

    g_idle_add(_gasket_train_send_real, pending);

    return FALSE;
}

gboolean
_gasket_train_send_real(struct _GasketTrainPending *pending)
{
    GError* error = NULL;
    gssize size;
    gsize to_send;
    gchar* buffer;
    GCancellable* cancellable;
    gboolean posted = FALSE;

    sem_wait(&pending->sem);
    buffer = pending->buffer;
    pending->buffer = NULL;

    if (buffer != NULL) {
        if (pending->stream != NULL)
        {
            posted = TRUE;
            sem_post(&pending->sem);
            to_send = strlen(buffer);

            while (to_send > 0)
            {
                cancellable = g_cancellable_new();
                size = g_output_stream_write(pending->stream, buffer, to_send,
                        cancellable, &error);
                to_send -= size;
            }
        }
        g_free(buffer);
    }

    if (!posted)
        sem_post(&pending->sem);
}

/**
 * gasket_train_clear:
 * @train: a #GasketTrain
 *
 * Wipes the output train.
 *
 * Return value: nothing.
 */
void
gasket_train_clear (GasketTrain *train)
{
    g_return_if_fail (train != NULL);

    GasketTrainPrivate *priv;

    priv = GASKET_TRAIN_GET_PRIVATE (train);

    if (priv->connected)
        gasket_train_station_send(train, "");
}

void
_gasket_train_carriage_iterator(gpointer key, gpointer val, gpointer data)
{
    xmlNodePtr root = (xmlNodePtr)data;
    xmlNodePtr node;
    GasketTrainCarriage* carriage = (GasketTrainCarriage*)val;

    node = xmlCopyNode(carriage->node, 1);
    xmlNewProp (node, "title", (const gchar*)key);
    xmlAddChild (root, node);
}

/**
 * gasket_train_redisplay:
 * @train: a #GasketTrain
 *
 * Refresh the currently set display.
 *
 * Return value: nothing.
 */
void
gasket_train_redisplay(GasketTrain* train)
{
    xmlNodePtr svg_root;
    xmlDocPtr  doc;
    xmlChar   *buffer;
    gint       buffer_size;
    GasketTrainPrivate *priv;

    g_return_if_fail (train != NULL);

    priv = GASKET_TRAIN_GET_PRIVATE (train);

    if (!priv->connected || !priv->active)
        return;

    doc = xmlNewDoc ("1.0");

    svg_root = xmlNewNode(NULL, "svg");
    xmlDocSetRootElement(doc, svg_root);

    xmlNewProp(svg_root, "width", "10");
    xmlNewProp(svg_root, "height", "10");
    xmlNewProp(svg_root, "xmlns", "http://www.w3.org/2000/svg");

    g_hash_table_foreach(priv->carriages, (GHFunc)_gasket_train_carriage_iterator, svg_root);

    xmlDocDumpFormatMemory(doc, &buffer, &buffer_size, 1);
    gasket_train_station_send(train, buffer);

    xmlFreeDoc(doc);
}

/**
 * gasket_train_remove_carriage:
 * @train: #GasketTrain
 * @carriage_id: the ID of an already-added carriage
 *
 * Takes the given carriage out of the train.
 *
 * Return value: none.
 */
void
gasket_train_remove_carriage(GasketTrain* train, const gchar* carriage_id)
{
    GasketTrainPrivate *priv;
    GasketTrainCarriage *carriage;
    xmlDocPtr doc;
    xmlNodePtr node;

    g_return_if_fail(train != NULL);

    priv = GASKET_TRAIN_GET_PRIVATE (train);

    carriage = g_hash_table_lookup(priv->carriages, carriage_id);

    g_return_if_fail(carriage != NULL);

    xmlFreeNode(carriage->node);
    carriage->node = NULL;
    g_free(carriage->name);
    carriage->name = NULL;

    g_hash_table_remove(priv->carriages, carriage_id);
    g_free(carriage);

    gasket_train_redisplay(train);
}

/**
 * gasket_train_add_carriage:
 * @train: a #GasketTrain
 * @name: the name of this carriage to be passed to the station (server), or NULL
 *
 * Appends a new carriage (placeholder for SVG). name defaults to new ID of
 * carriage.
 *
 * Return value: process-unique ID for referring to this carriage
 */
const gchar*
gasket_train_add_carriage(GasketTrain* train, const gchar* name)
{
    GasketTrainPrivate *priv;
    gchar* carriage_id;
    GasketTrainCarriage *carriage;

    g_return_val_if_fail(train != NULL, NULL);

    priv = GASKET_TRAIN_GET_PRIVATE (train);

    carriage_id = g_new(gchar, 20);
    sprintf(carriage_id, "carriage-%d", priv->carriage_next_free_id);
    priv->carriage_next_free_id++;

    if (name == NULL)
        name = carriage_id;

    carriage = g_new(GasketTrainCarriage, 1);
    carriage->name = g_strdup(name);
    carriage->node = xmlNewNode(NULL, "g");
    g_hash_table_insert(priv->carriages, carriage_id, carriage);

    return (const gchar*) carriage_id;
}

void
_gasket_train_set_pending(GasketTrain* train, gchar* buffer)
{
    GasketTrainPrivate *priv;
    gchar* old_buffer;

    g_return_if_fail(train != NULL);

    priv = GASKET_TRAIN_GET_PRIVATE (train);

    sem_wait(&priv->pending->sem);

    old_buffer = priv->pending->buffer;
    if (old_buffer != NULL)
        g_free(old_buffer);

    priv->pending->buffer = buffer;

    sem_post(&priv->pending->sem);
    _gasket_train_watch_pending(priv->pending);
}

/**
 * gasket_train_update_carriage:
 * @train: #GasketTrain
 * @carriage_id: the ID of an already-added carriage
 * @svg: the SVG element to replace the content of the carriage
 *
 * Updates a carriage with new SVG content.
 *
 * Return value: none.
 */
void
gasket_train_update_carriage(GasketTrain* train, const gchar* carriage_id, const gchar* svg)
{
    GasketTrainPrivate *priv;
    GasketTrainCarriage *carriage;
    xmlDocPtr doc;
    xmlNodePtr node;

    g_return_if_fail(train != NULL);

    priv = GASKET_TRAIN_GET_PRIVATE (train);

    carriage = g_hash_table_lookup(priv->carriages, carriage_id);

    g_return_if_fail(carriage != NULL);

    doc = xmlReadMemory(svg, strlen(svg), "noname.xml", NULL, 0);
    node = xmlDocGetRootElement(doc);
    node = xmlCopyNode(node, 1);

    xmlNodePtr child, next;
    if (xmlChildElementCount(carriage->node) > 0 &&
            (child = xmlFirstElementChild(carriage->node)) != NULL)
        while (child != NULL) {
            next = child->next;
            xmlFreeNode(child);
            xmlUnlinkNode(child);
            child = next;
        }

    xmlAddChild(carriage->node, node);

    gasket_train_redisplay(train);
}

/**
 * gasket_train_station_connect:
 * @train: a #GasketTrain
 *
 * Connects to a station (GUI-based server)
 *
 * Return value: 0 if success, otherwise negative error value.
 */
gint
gasket_train_station_connect (GasketTrain *train)
{
    GasketTrainPrivate *priv;
    g_return_val_if_fail(train != NULL, -1);

    priv = GASKET_TRAIN_GET_PRIVATE (train);

    GSocketAddress* address = NULL;
    GSocket* socket = NULL;
    GError* error = NULL;
    const gchar* socket_file = NULL;

    socket_file = g_getenv("GASKET_SOCKET");

    if (socket_file == NULL)
        return (-2);

    address = g_unix_socket_address_new(socket_file);
    socket = g_socket_new(G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &error);

    if (socket == NULL)
        return (-3);

    if (!g_socket_connect(socket, address, FALSE, &error))
        //TODO: decide required diagnostics
        return (-4);

    GSocketConnection* connection = g_socket_connection_factory_create_connection(socket);
    priv->stream = g_io_stream_get_output_stream(connection);

    if (!priv->stream)
        return (-5);

    priv->connected = TRUE;

    sem_wait(&priv->pending->sem);
    priv->pending->stream = priv->stream;
    sem_post(&priv->pending->sem);

    return (0);
}

/**
 * gasket_train_shutdown:
 * @train: a #GasketTrain
 *
 * Close the connection.
 *
 * Return value: Nothing.
 */
void
gasket_train_shutdown(GasketTrain* train)
{
    GasketTrainPrivate *priv;
    GError *error = NULL;
    GCancellable* cancellable;

    g_return_if_fail(train != NULL);

    priv = GASKET_TRAIN_GET_PRIVATE (train);

    cancellable = g_cancellable_new();

    gasket_train_clear(train);

    //FIXME: kill the output thread first, but don't depend on it not waiting
    g_output_stream_close(priv->stream, cancellable, &error);

    priv->connected = FALSE;
}
