## ##########################
## # Installation definitions

kde_icon_DATA = kmines.xpm
minidir = $(kde_minidir)
appsdir = $(kde_appsdir)/Games
apps_DATA = kmines.kdelnk
htmldir = $(kde_htmldir)/en/kmines
html_DATA = index.html index-1.html index-2.html index-3.html logotp3.gif

EXTRA_DIST = $(kde_icon_DATA) $(apps_DATA) $(html_DATA) mini-kmines.xpm kmines.xpm

install-data-local:
	$(mkinstalldirs) $(minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kmines.xpm $(minidir)/kmines.xpm
	$(mkinstalldirs) $(minidir)/../large
	$(INSTALL_DATA) $(srcdir)/large-kmines.xpm $(minidir)/../large/kmines.xpm

uninstall-local:
	rm -f $(minidir)/kmines.xpm
	rm -f $(minidir)/../large-kmines.xpm
