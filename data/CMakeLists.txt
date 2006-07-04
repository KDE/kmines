



########### install files ###############

install( FILES kmines.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES eventsrc  DESTINATION  ${DATA_INSTALL_DIR}/kmines )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#KDE_ICON  = kmines
#
#xdg_apps_DATA = kmines.desktop
#
#appdatadir = $(kde_datadir)/kmines
#appdata_DATA = eventsrc
#
