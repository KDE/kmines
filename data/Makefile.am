## ##########################
## # Installation definitions

icondir = $(kde_icondir)
icon_DATA = kmines.xpm
minidir = $(kde_minidir)
appsdir = $(kde_appsdir)/Games
apps_DATA = kmines.kdelnk
htmldir = $(kde_htmldir)/en/kmines
html_DATA = index.html index-1.html index-2.html index-3.html logotp3.gif

install-data-local:
	$(mkinstalldirs) $(minidir)
	$(INSTALL_DATA) mini-kmines.xpm $(minidir)/kmines.xpm

uninstall-local:
	rm -f $(minidir)/kmines.xpm
	rm -rf $(htmldir)
