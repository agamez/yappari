/*
 * This file is part of sharing-plugin-template
 *
 * Copyright (C) 2008-2009 Nokia Corporation. All rights reserved.
 *
 * This maemo code example is licensed under a MIT-style license,
 * that can be found in the file called "COPYING" in the root
 * directory.
 *
 */

#ifndef _SEND_H_
#define _SEND_H_

#include <glib.h>
#include <sharing-transfer.h>
#include <sharing-entry.h>
#include <conicconnection.h>
#include <sharing-plugin-interface.h>

G_BEGIN_DECLS

SharingPluginInterfaceSendResult send (SharingTransfer* transfer,
    ConIcConnection* con, gboolean* dead_mans_switch);

G_END_DECLS

#endif // _SEND_H_

