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

#ifndef __GASKET_TRAIN_H__
#define __GASKET_TRAIN_H__

#include <glib.h>
#include <glib-object.h>

#define GASKET_TRAIN_TYPE		\
    (gasket_train_get_type())
#define GASKET_TRAIN(o)			\
    (G_TYPE_CHECK_INSTANCE_CAST ((o), GASKET_TRAIN_TYPE, GasketTrain))
#define GASKET_TRAIN_CLASS(c)		\
    (G_TYPE_CHECK_CLASS_CAST ((c), GASKET_TRAIN_TYPE, GasketTrainClass))
#define TUT_IS_GREETER(o)		\
    (G_TYPE_CHECK_INSTANCE_TYPE ((o), GASKET_TRAIN_TYPE))
#define TUT_IS_GREETER_CLASS(c)		\
    (G_TYPE_CHECK_CLASS_TYPE ((c),  GASKET_TRAIN_TYPE))
#define GASKET_TRAIN_GET_CLASS(o)	\
    (G_TYPE_INSTANCE_GET_CLASS ((o), GASKET_TRAIN_TYPE, GasketTrainClass))

typedef struct _GasketTrain		GasketTrain;
typedef struct _GasketTrainPrivate	GasketTrainPrivate;
typedef struct _GasketTrainClass	GasketTrainClass;

struct _GasketTrainPending;

struct _GasketTrain {
    GObject parent;
};

struct _GasketTrainClass {
    GObjectClass parent;
};

GType		gasket_train_get_type	        () G_GNUC_CONST;

GasketTrain*	gasket_train_new		(void);

void		gasket_train_clear              (GasketTrain *train);

void		gasket_train_station_reset      (GasketTrain *train);

void            gasket_train_station_send       (GasketTrain *train, const gchar* gasket_content);

const gchar*    gasket_train_add_carriage       (GasketTrain* train, const gchar* name);

void            gasket_train_update_carriage    (GasketTrain* train, const gchar* carriage_id, const gchar* svg);

void            gasket_train_remove_carriage    (GasketTrain* train, const gchar* carriage_id);

gint            gasket_train_station_connect    (GasketTrain *train);

void            gasket_train_redisplay          (GasketTrain* train);

void            gasket_train_flush              (GasketTrain* train);

void            gasket_train_shutdown           (GasketTrain* train);
#endif /* __TUT_GREETER_H__ */
