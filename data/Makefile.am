## ##########################
## # Installation definitions

kde_icon_DATA = kmines.xpm
minidir = $(kde_minidir)
appsdir = $(kde_appsdir)/Games
apps_DATA = kmines.desktop

EXTRA_DIST = $(kde_icon_DATA) $(apps_DATA) mini-kmines.xpm kmines.xpm

install-data-local:
	$(mkinstalldirs) $(minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kmines.xpm $(minidir)/kmines.xpm

uninstall-local:
	rm -f $(minidir)/kmines.xpm
