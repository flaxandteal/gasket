
#ifndef GASKET_SERVER_H
#define GASKET_SERVER_H

#include <glib-object.h>
#include <cairo.h>

G_BEGIN_DECLS

/**
 * GasketServerError:
 * @GASKET_SERVER_ERROR_ENVIRONMENT_SETUP_FAILED: could not set environment variables
 */
typedef enum {
  GASKET_SERVER_ERROR_ENVIRONMENT_SETUP_FAILED = 0
} GasketServerError;

GQuark gasket_server_error_quark (void);

#define GASKET_SERVER_ERROR ( gasket_server_error_quark() )

#define GASKET_TYPE_SERVER           (gasket_server_get_type())
#define GASKET_SERVER(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GASKET_TYPE_SERVER, GasketServer))
#define GASKET_SERVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),  GASKET_TYPE_SERVER, GasketServerClass))
#define GASKET_IS_SERVER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GASKET_TYPE_SERVER))
#define GASKET_IS_SERVER_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GASKET_TYPE_SERVER))
#define GASKET_SERVER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),  GASKET_TYPE_SERVER, GasketServerClass))

typedef struct _GasketServer      GasketServer;
typedef struct _GasketServerClass GasketServerClass;
typedef struct _GasketServerPrivate      GasketServerPrivate;

struct _GasketServer {
        GObject parent_instance;

        /* <private> */
        GasketServerPrivate *priv;
};

struct _GasketServerClass {
        GObjectClass parent_class;
};

GType gasket_server_get_type(void);

GasketServer *gasket_server_new (GError **error);
GasketServer *gasket_server_new_with_uuid(const gchar* uuid, GError **error);

gboolean gasket_server_set_uuid (GasketServer *gasket, const gchar* uuid_str);
gboolean gasket_server_make_socket (GasketServer *gasket);
void gasket_server_child_setup (GasketServer *gasket);
void gasket_server_update_table (GasketServer *gasket, GHashTable* table);
void gasket_server_launch_listen (GasketServer *gasket);
void gasket_server_paint_overlay (GasketServer *gasket, cairo_t *cr);
void gasket_server_set_invalidation_function (GasketServer *gasket, GSourceFunc function, gpointer user_data);
void gasket_server_set_target_extents(GasketServer *gasket, int row, int col, int row_count, int column_count, long width, long height);
void gasket_server_close(GasketServer *gasket);
gpointer gasket_server_listen (GasketServer *gasket);

G_END_DECLS

#endif

