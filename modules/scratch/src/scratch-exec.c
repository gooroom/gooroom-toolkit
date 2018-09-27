/*
 * Copyright (c) 2015 - 2017 gooroom <gooroom@gooroom.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include <libintl.h>

#include <gtk/gtk.h>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <glib/gi18n.h>

#define	SCRATCH_GUI_WORKING_DIR	"/opt/scratch/scratch-gui"
#define	SCRATCH_GUI_URL			"0.0.0.0:8601"



static void
process_done_cb (GPid     pid,
                 gint     status,
                 gpointer data)
{
	g_spawn_close_pid (pid);

	gtk_main_quit ();
}

int
main (int argc, char **argv)
{
	gint ret = -1;

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);

	gchar *pidof, *npm, *cmdline, *output = NULL;

	pidof = g_find_program_in_path ("pidof");
	npm = g_find_program_in_path ("npm");
	cmdline = g_strdup_printf ("%s %s", pidof, npm);

	if (g_spawn_command_line_sync (cmdline, &output, NULL, NULL, NULL)) {
		if (output && strlen (output) > 0) {
			GPid pid;
			gchar **argv = NULL;
			gchar *cmd = NULL, *cmdline = NULL;

			cmd = g_find_program_in_path ("gooroom-browser");
			cmdline = g_strdup_printf ("%s %s", cmd, SCRATCH_GUI_URL);

			argv = g_strsplit (cmdline, " ", -1);

			/* try to spawn the new process */
			if (g_spawn_async (NULL, argv, NULL,
						G_SPAWN_DO_NOT_REAP_CHILD,
						NULL, NULL, &pid, NULL))
			{
				g_child_watch_add (pid, (GChildWatchFunc) process_done_cb, NULL);
			}

			g_free (cmd);
			g_free (cmdline);
			g_strfreev (argv);

			ret = 0;
		}
	}

	g_free (pidof);
	g_free (npm);
	g_free (cmdline);
	g_free (output);

	if (ret != 0) {
		GtkWidget *message = gtk_message_dialog_new (NULL,
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				_("Failed to launch scratch"));

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (message),
				_("You can't launch scratch because the NPM service is not running.\nPlease try again later."));

		gtk_window_set_title (GTK_WINDOW (message), _("Notification"));

		gtk_dialog_run (GTK_DIALOG (message));
		gtk_widget_destroy (message);
		return -1;
	}

	gtk_main ();
	return 0;
}
