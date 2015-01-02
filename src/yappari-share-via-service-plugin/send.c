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

#include <stdio.h>
#include <glib.h>
#include <osso-log.h>
#include <sharing-http.h>
#include "send.h"
#include "common.h"

/**
 * send:
 * @account: #SharingTransfer to be send
 * @con: Connection used
 * @dead_mans_switch: Turn to %FALSE at least every 30 seconds.
 *
 * Sends #SharingTransfer to service.
 *
 * Returns: #SharingPluginInterfaceSendResult
 */
SharingPluginInterfaceSendResult send (SharingTransfer* transfer,
    ConIcConnection* con, gboolean* dead_mans_switch)
{
    SharingPluginInterfaceSendResult ret = SHARING_SEND_SUCCESS;

    SharingEntry *entry = sharing_transfer_get_entry( transfer );

    gint result = 0;

    for (GSList* p = sharing_entry_get_media (entry); p != NULL; p = g_slist_next(p)) {
      SharingEntryMedia* media = p->data;
      /* Process media */
      if (!sharing_entry_media_get_sent (media)) {
	/* Post media */
	//result = my_send_task_post_function (my_send_task, media);
	/* Process post result */
	if (result == 0 /* EXAMPLE: MY_SEND_RESULT_SUCCESS */) {
	  /* If success mark media as sent */
	  sharing_entry_media_set_sent (media, TRUE);
	  /* And mark process to your internal data structure */
	  //my_send_task->upload_done += sharing_entry_media_get_size (media); 
	} else {
	  /* We have sent the file in last sharing-manager call */
	  //my_send_task->upload_done += sharing_entry_media_get_size (media);
	}
      }
    }

    return ret;
}

