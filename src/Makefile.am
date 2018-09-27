sbin_PROGRAMS = gooroom-toolkit 


BUILT_SOURCES = \
	toolkit-resources.c \
	toolkit-resources.h

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DLOCALEDIR=\"$(localedir)\"    \
	-DPACKAGE_DOWNLOADDIR=\""/var/tmp/download/"\" \
	-DPACKAGE_VERDIR=\""$(sysconfdir)/gooroom/gooroom-toolkit/"\" \
	-DMODULE_INSTALL_SCRIPT_DIR=\""$(datadir)/gooroom/toolkit/modules/"\" \
	$(VTE_CFLAGS) \
	$(GLIB_CFLAGS)	\
	$(GTK_CFLAGS)	\
	$(JSONCPP_CFLAGS)
	
gooroom_toolkit_LDADD = \
	$(GTK_LIBS) 	\
	$(GLIB_LIBS) 	\
	$(VTE_LIBS)	\
	$(JSONCPP_LIBS)	\
	-lutil		\
	-lapt-pkg	\
	-lX11

gooroom_toolkit_SOURCES= \
	gruserdialog.cc		\
	grutils.cc		\
	grwindow.cc		\
	gooroomtoolkit.cc	\
	grfetchprogress.cc	\
	grinstallprogress.cc	\
	grmainwindow.cc		\
	grpackage.cc		\
	grpackagemanager.cc	\
	grparser.cc		\
	grmkit-button.c	\
	$(BUILT_SOURCES)

resource_files = $(shell glib-compile-resources --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/gresource.xml)
toolkit-resources.c: gresource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name toolkit $<
toolkit-resources.h: gresource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name toolkit $<
