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
