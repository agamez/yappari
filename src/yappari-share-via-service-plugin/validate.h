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

#ifndef _VALIDATE_H_
#define _VALIDATE_H_

#include <glib.h>
#include <sharing-entry.h>
#include <sharing-plugin-interface.h>
#include <conicconnection.h>

G_BEGIN_DECLS

SharingPluginInterfaceAccountValidateResult validate (SharingAccount* account,
    ConIcConnection* con, gboolean *cont, gboolean* dead_mans_switch);

G_END_DECLS

#endif // _VALIDATE_H_

