/* grinstallprogress.cc
 * 
 * Copyright (C) 2015-2017 Gooroom <gooroom@gooroom.kr>
 * Copyright (c) 2004 Canonical
 *
 * Author: Michael Vogt <mvo@debian.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */
#include "config.h"

#include <pty.h>
#include <math.h>
#include <errno.h>

#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <glib.h>
#include <gio/gio.h>

#include <glib/gi18n.h>

#include "grutils.h"
#include "gruserdialog.h"

#include "grmainwindow.h"
#include "grinstallprogress.h"

ssize_t
write_fd(int fd, void *ptr, size_t nbytes, int sendfd)
{
    struct msghdr msg;
    struct iovec iov[1];
    
    union {
            struct cmsghdr cm;
            char control[CMSG_SPACE(sizeof(int))];
    } control_un;
    struct cmsghdr *cmptr;
    
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    
    cmptr = CMSG_FIRSTHDR(&msg);
    cmptr->cmsg_len = CMSG_LEN(sizeof(int));
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    *((int *)CMSG_DATA(cmptr)) = sendfd;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    return (sendmsg(fd, &msg, 0));
}

ssize_t
read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
    struct msghdr msg;
    struct iovec iov[1];
    ssize_t n;
    int newfd;
    
    union {
        struct cmsghdr cm;
	char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    struct cmsghdr *cmptr;
    
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    if ((n = recvmsg(fd, &msg, MSG_WAITALL)) <= 0)
        return (n);
    if ((cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
        cmptr->cmsg_len == CMSG_LEN(sizeof(int)))
    {
        if (cmptr->cmsg_level != SOL_SOCKET)
        {
	    perror("control level != SOL_SOCKET");
	    exit(1);
        }
        if (cmptr->cmsg_type != SCM_RIGHTS)
        {
   	    perror("control type != SCM_RIGHTS");
	    exit(1);
        }
        *recvfd = *((int *)CMSG_DATA(cmptr));
    }
    else
        *recvfd = -1; /* descriptor was not passed */

    return (n);
}
/* end read_fd */

#define UNIXSTR_PATH "/var/run/synaptic.socket"

int ipc_send_fd(int fd)
{
    // open connection to server
    struct sockaddr_un servaddr;
    int serverfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, UNIXSTR_PATH);
    
    // wait max 5s (5000 * 1000/1000000) for the server
    for (int i = 0; i < 5000; i++)
    {
        if (connect(serverfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0)
            break;
	usleep(1000);
    }
    // send fd to server
    write_fd(serverfd, (void *)"", 1, fd);
    close(serverfd);

    return 0;
}

int ipc_recv_fd()
{
    int ret;
    
    // setup socket
    struct sockaddr_un servaddr, cliaddr;
    char c;
    int connfd = -1, fd;
    
    int listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    fcntl(listenfd, F_SETFL, O_NONBLOCK);
    
    unlink(UNIXSTR_PATH);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, UNIXSTR_PATH);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 1);
    
    // wait for connections
    socklen_t clilen = sizeof(cliaddr);
    
    // wait max 5s (5000 * 1000/1000000) for the client
    for (int i = 0; i < 5000 || connfd > 0; i++)
    {
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd > 0)
            break;

	usleep(1000);
	RGFlushInterface();
    }
    // read_fd
    read_fd(connfd, &c, 1, &fd);
    
    close(connfd);
    close(listenfd);
    
    return fd;
}

GRInstallProgress::GRInstallProgress(GRMainWindow *main, bool autoClose)
    : GRWindow(main, "toolinstall_progress"),
        _totalActions(0), _progress(0), _sock(0), _userDialog(0), _autoClose(autoClose)
{
    _terminalTimeout = 120;
    _systemRes = OrderResult::Failed;
    
    setTitle(_("Install"));
    // make sure we try to get a graphical debconf
    setenv("DEBIAN_FRONTEND", "gnome", FALSE);
    setenv("APT_LISTCHANGES_FRONTEND", "gtk", FALSE);
    
    _startCounting = false;
    _label_status = GTK_WIDGET(gtk_builder_get_object(_builder, "label_status"));
    _pbarTotal = GTK_WIDGET(gtk_builder_get_object(_builder, "progress_total"));
    gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(_pbarTotal), 0.025);
    
    // work around for kdesudo blocking our SIGCHLD (LP: #156041)
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &sset, NULL);
    
    _term = vte_terminal_new();
    vte_terminal_set_size(VTE_TERMINAL(_term), 80, 23);
    GtkWidget *scrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,
                                             gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(VTE_TERMINAL(_term))));
    gtk_widget_set_can_focus(scrollbar, FALSE);
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(_term), 10000);
    
    //const char *s = "monospace 8";
    // PangoFontDescription *fontdesc = pango_font_description_from_string(s);
    // vte_terminal_set_font(VTE_TERMINAL(_term), fontdesc);
    // pango_font_description_free(fontdesc);
    
    gtk_box_pack_start(GTK_BOX(GTK_WIDGET(gtk_builder_get_object(_builder, "hbox_vte"))), _term, TRUE, TRUE, 0);        
    g_signal_connect(G_OBJECT(_term), "button-press-event", (GCallback)cbTerminalClicked, this);
    
    gtk_widget_show(_term);
    
    gtk_box_pack_end(GTK_BOX(GTK_WIDGET(gtk_builder_get_object(_builder, "hbox_vte"))), scrollbar, FALSE, FALSE, 0);

    // Terminal contextual menu
    GtkWidget *img, *menuitem;
    _popupMenu = gtk_menu_new();
    menuitem = gtk_menu_item_new_with_label(_("Copy"));
    g_object_set_data(G_OBJECT(menuitem), "me", this);
    g_signal_connect(menuitem, "activate", (GCallback)cbMenuitemClicked, (void *)EDIT_COPY);
    gtk_menu_shell_append(GTK_MENU_SHELL(_popupMenu), menuitem);
    gtk_widget_show(menuitem);
    
    menuitem = gtk_menu_item_new_with_label(_("Select All"));
    g_object_set_data(G_OBJECT(menuitem), "me", this);
    g_signal_connect(menuitem, "activate", (GCallback)cbMenuitemClicked, (void *)EDIT_SELECT_ALL);
    gtk_menu_shell_append(GTK_MENU_SHELL(_popupMenu), menuitem);
    gtk_widget_show(menuitem);
    
    gtk_widget_show(scrollbar);
    
    gtk_window_set_default_size(GTK_WINDOW(_win), 500, -1);
    
    g_signal_connect(_term, "contents-changed", G_CALLBACK(content_changed), this);
    
    g_signal_connect(gtk_builder_get_object(_builder, "button_cancel"), "clicked", G_CALLBACK(cbCancel), this);
    g_signal_connect(gtk_builder_get_object(_builder, "button_close"), "clicked", G_CALLBACK(cbClose), this);
    g_signal_connect(VTE_TERMINAL(_term), "child-exited", G_CALLBACK(child_exited), this);
    
    if (_userDialog == NULL)
        _userDialog = new GRUserDialog(this);
    
    gtk_window_set_urgency_hint(GTK_WINDOW(_win), FALSE);
    
    // init the timer
    last_term_action = time(NULL);
    
    _cssProvider = gtk_css_provider_new();
}
    
GRInstallProgress::~GRInstallProgress()
{
#ifdef DEBUG_MSG
    cout << "GRInstallProgress::~GRInstallProgress()" << endl;
#endif           
    delete _userDialog;
    g_object_unref(_cssProvider);
}

void 
GRInstallProgress::start(string strSrc, string strFormat, string strDownloadSrc, string strInstallSrc)
{   
    string execPath = strDownloadSrc + "/" + strSrc;
    cout << execPath << endl;

    if (!FileExists(execPath))
    {
        string strConfigDir = SCRATCH_HELPER_SCRIPT;
        execPath = strConfigDir + strSrc;
    }

    char szCommand[512];
    snprintf(szCommand, sizeof(szCommand), "sh %s %s %s", execPath.c_str(), strDownloadSrc.c_str(), strInstallSrc.c_str());
    start(szCommand, strFormat);
}

void
GRInstallProgress::start(string execCommand, string strFormat)
{
    int master;
    _child_id = forkpty(&master, NULL, NULL, NULL);
    
    if(_child_id < 0) 
    {
        //cerr << "vte_terminal_forkpty() failed. " << strerror(errno) << endl;
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (GTK_WINDOW(window()),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        NULL);
        gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), 
                                        _("Error failed to fork pty"));
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    } 
    else if (_child_id == 0) 
    {      
        int fd[2];
        pipe(fd);
        ipc_send_fd(fd[0]); // send the read part of the pipe to the parent
        
        if (strFormat.compare("shell") == 0)
        {
            int res = system(execCommand.c_str());
	    if (res == 0)
	    {
                _systemRes = OrderResult::Completed;
	    }
        }
        
        // dump errors into cerr (pass it to the parent process)	
        _error->DumpErrors();
        
        ::close(fd[0]);
        ::close(fd[1]);
        
        _exit(0);
    }

    // parent: assign pty to the vte terminal
    GError *err = NULL;
    VtePty *pty = vte_pty_new_foreign_sync(master, NULL, &err);
    
    if (err != NULL) 
    {
        std::cerr << "failed to create new pty: " << err->message << std::endl;
        g_error_free (err);
        return;
    }
    vte_terminal_set_pty(VTE_TERMINAL(_term), pty);
    // FIXME: is there a race here? i.e. what if the child is dead before
    //        we can set it?
    vte_terminal_watch_child(VTE_TERMINAL(_term), _child_id);
    
    _childin = ipc_recv_fd();
    if(_childin < 0) 
    {
        // something _bad_ happend. so the terminal window and hope for the best
        GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(_builder, "expander_terminal"));
        gtk_expander_set_expanded(GTK_EXPANDER(w), TRUE);
        gtk_widget_hide(_pbarTotal);
    }
    
    // make it nonblocking
    fcntl(_childin, F_SETFL, O_NONBLOCK);
    
    startUpdate();
    while(!child_has_exited)
        updateInterface();
    
    finishUpdate();
    
    ::close(_childin);
    ::close(master);
}


void 
GRInstallProgress::child_exited(VteTerminal *vteterminal,
                                        gint ret,
                                        gpointer data)
{
    GRInstallProgress *me = (GRInstallProgress *)data;
    
    if (ret == 0)
        me->_systemRes = OrderResult::Completed;
    else
        me->_systemRes = OrderResult::Failed;

    me->child_has_exited = true;
}

void 
GRInstallProgress::cbCancel(GtkWidget *self, void *data)
{
    //FIXME: we can't activate this yet, it's way to heavy (sending KILL)
    //cout << "cbCancel: sending SIGKILL to child" << endl;
    GRInstallProgress *me = (GRInstallProgress *)data;
    kill(me->_child_id, SIGTERM);
    
    me->_systemRes = OrderResult::Cancel;
}

void 
GRInstallProgress::cbClose(GtkWidget *self, void *data)
{
    //cout << "cbCancel: sending SIGKILL to child" << endl;
    GRInstallProgress *me = (GRInstallProgress *)data;
    me->_updateFinished = true;
}

bool 
GRInstallProgress::close()
{
    if (child_has_exited)
        cbClose(NULL, this);
    
    return TRUE;
}

void 
GRInstallProgress::content_changed(GObject *object, gpointer data)
{
    GRInstallProgress *me = (GRInstallProgress *)data;
    me->last_term_action = time(NULL);
}

gboolean 
GRInstallProgress::cbTerminalClicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    if (event->button == 3)
    {
        GRInstallProgress *me = (GRInstallProgress *)user_data;
	gtk_menu_popup(GTK_MENU(me->_popupMenu), NULL, NULL, NULL, NULL,
			event->button,
			gdk_event_get_time((GdkEvent *)event));
	return true;
    }

    return false;
}

void 
GRInstallProgress::cbMenuitemClicked(GtkMenuItem *menuitem, gpointer user_data)
{
    GRInstallProgress *me = (GRInstallProgress *)g_object_get_data(G_OBJECT(menuitem), "me");
    me->terminalAction(me->_term, (TermAction)GPOINTER_TO_INT(user_data));
}

void 
GRInstallProgress::terminalAction(GtkWidget *terminal, TermAction action)
{
    switch (action)
    {
        case EDIT_COPY:
            vte_terminal_copy_clipboard(VTE_TERMINAL(terminal));
            break;
        case EDIT_SELECT_ALL:
            vte_terminal_select_all(VTE_TERMINAL(terminal));
            break;
        case EDIT_SELECT_NONE:
            vte_terminal_unselect_all(VTE_TERMINAL(terminal));
            break;
    }
}

void 
GRInstallProgress::startUpdate()
{
    child_has_exited = false;
    
    show();
    
    RGFlushInterface();
}

void 
GRInstallProgress::updateInterface()
{
    char buf[2];
    static char line[1024] = "";
    int i = 0;
    
    while (1)
    {
        // This algorithm should be improved (it's the same as the rpm one ;)
        int len = read(_childin, buf, 1);
        
        // nothing was read
        if (len < 1)
            break;
        
        // update the time we last saw some action
        last_term_action = time(NULL);
        
        if (buf[0] == '\n')
        {
            //cout << "got line: " << line << endl;
	    gchar **split = g_strsplit(line, ":", 4);
            gchar *status = g_strstrip(split[0]);
            gchar *pkg = g_strstrip(split[1]);
            gchar *percent = g_strstrip(split[2]);
            gchar *str = g_strdup(g_strstrip(split[3]));
            
            // major problem here, we got unexpected input. should _never_ happen
            if (!(pkg && status))
                continue;
                                                    
            _startCounting = true;
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(_pbarTotal), 0);
            
            // reset the urgency hint, something changed on the terminal
            if (gtk_window_get_urgency_hint(GTK_WINDOW(_win)))
                gtk_window_set_urgency_hint(GTK_WINDOW(_win), FALSE);
            
            float val = atof(percent) / 100.0;
            
            if (fabs(val - gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(_pbarTotal))) > 0.1)
                gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(_pbarTotal), val);
            
            if (str != NULL)
                gtk_label_set_text(GTK_LABEL(_label_status), utf8(str));
            
            // clean-up
            g_strfreev(split);
            g_free(str);
            line[0] = 0;
        }
        else
        {
            buf[1] = 0;
            strcat(line, buf);
        }
    }
    
    time_t now = time(NULL);
    
    if (!_startCounting)
    {
        usleep(100000);
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR(_pbarTotal));
        // wait until we get the first message from apt
        last_term_action = now;
    }
    
    if ((now - last_term_action) > _terminalTimeout)
    {
        // get some debug info
        const gchar *s = gtk_label_get_text(GTK_LABEL(_label_status));
        g_warning("no statusfd changes/content updates in terminal for %i" " seconds", _terminalTimeout);
        g_warning("TerminalTimeout in step: %s", s);
        // now expand the terminal
        GtkWidget *w;
        w = GTK_WIDGET(gtk_builder_get_object(_builder, "expander_terminal"));
        gtk_expander_set_expanded(GTK_EXPANDER(w), TRUE);
        last_term_action = time(NULL);
        // try to get the attention of the user
        gtk_window_set_urgency_hint(GTK_WINDOW(_win), TRUE);
    }
    
    if (gtk_events_pending())
    {
        while (gtk_events_pending())
            gtk_main_iteration();
    }
    else
    {
        // 25fps
        usleep(1000000 / 25);
    }
}


void 
GRInstallProgress::finishUpdate()
{
    if (_startCounting)
    {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(_pbarTotal), 1.0);
    }

    RGFlushInterface();
    
    GtkWidget *_closeB = GTK_WIDGET(gtk_builder_get_object(_builder, "button_close"));
    gtk_widget_set_sensitive(_closeB, TRUE);
    
    if (_systemRes == OrderResult::Completed)
    {
        gtk_widget_grab_focus(_closeB);
        if (!_autoClose)
        {
            _userDialog->message_ex(_("The package has been installed successfully."));        
            _updateFinished = true;
        }
    }
    
    gchar *msg = g_strdup_printf("<big><b>Installed</b></big>\n%s", "The package has been installed successfully.");
    setTitle(_("Reuslt"));
    GtkWidget *l = GTK_WIDGET(gtk_builder_get_object(_builder, "label_action"));
    gtk_label_set_markup(GTK_LABEL(l), msg);
    g_free(msg);
    
    // hide progress and label
    gtk_widget_hide(_pbarTotal);
    gtk_widget_hide(_label_status);
    
    // wait for user action
    while (true)
    {
        // events
        while (gtk_events_pending())
            gtk_main_iteration();
    
        // user clicked "close" button
        if (_updateFinished)
            break;
    
        if (_autoClose)                
            break;
    
        // wait a bit
        g_usleep(100000);
    }
    
    // hide and finish
    if (_sock != NULL)
    {
        gtk_widget_destroy(_sock);
    }
    else
    {           
        hide();
    }
    
}
