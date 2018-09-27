scratch_execdir = $(datadir)/gooroom/toolkit/modules/scratch
scratch_exec_PROGRAMS = scratch-exec

scratch_exec_SOURCES = scratch-exec.c 

scratch_exec_CFLAGS = \
	-DLOCALEDIR=\"$(localedir)\"	\
	$(GLIB_CFLAGS)	\
	$(GTK_CFLAGS)	\
	$(GOBJECT_CFLAGS)

scratch_exec_LDADD = \
	$(GLIB_LIBS)	\
	$(GTK_LIBS)		\
	$(GOBJECT_LIBS)

@INTLTOOL_DESKTOP_RULE@
desktopdir = $(datadir)/applications
desktop_in_files = scratch.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)

@INTLTOOL_DESKTOP_RULE@
autostart_desktopdir = $(sysconfdir)/xdg/autostart
autostart_desktop_in_files = npm-start.desktop.in.in
autostart_desktop_DATA = $(autostart_desktop_in_files:.desktop.in.in=.desktop)

@INTLTOOL_POLICY_RULE@
polkitdir = $(datadir)/polkit-1/actions
polkit_in_files = kr.gooroom.pkexec.npm.policy.in.in
polkit_DATA = $(polkit_in_files:.policy.in.in=.policy)

EXTRA_DIST = $(desktop_in_files) $(autostart_desktop_in_files)
DISTCLEANFILES = $(desktop_DATA) $(autostart_desktop_DATA) $(polkit_DATA)
