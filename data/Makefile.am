gooroom_toolkit_confdir = $(sysconfdir)/gooroom/gooroom-toolkit
gooroom_toolkit_conf_DATA = toolpackages.json

@INTLTOOL_DESKTOP_RULE@
desktopdir = $(datadir)/applications
desktop_in_files = gooroom-toolkit.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)

EXTRA_DIST = $(desktop_in_files)
DISTCLEANFILES = $(desktop_DATA)
