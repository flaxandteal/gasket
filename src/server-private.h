G_BEGIN_DECLS

struct _GasketServerConnectionData;
gboolean _gasket_server_close_socket(GasketServer *gasket);
gboolean _gasket_server_setenv(int parent_pid, uuid_t *uuid_ptr);
gboolean _gasket_server_make_tmpdir(GasketServer* gasket);
gboolean _gasket_server_update_svg(gpointer user_data);
gpointer _gasket_server_handle_new_connection(struct _GasketServerConnectionData *data);
gboolean _gasket_server_close_connection(gpointer data);

G_END_DECLS
