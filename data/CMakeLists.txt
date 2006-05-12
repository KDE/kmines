



########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kmines.desktop )
install_files( ${DATA_INSTALL_DIR}/kmines FILES eventsrc )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#KDE_ICON  = kmines
#
#xdg_apps_DATA = kmines.desktop
#
#appdatadir = $(kde_datadir)/kmines
#appdata_DATA = eventsrc
#
