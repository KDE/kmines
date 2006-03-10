kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kmines.desktop )
install_files( ${DATA_INSTALL_DIR}/kmines FILES eventsrc )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#KDE_ICON  = kmines
#
#xdg_apps_DATA = kmines.desktop
#
#appdatadir = $(kde_datadir)/kmines
#appdata_DATA = eventsrc
#
