
install( PROGRAMS kmines.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmines.notifyrc  DESTINATION  ${KNOTIFYRC_INSTALL_DIR} )

ecm_install_icons(ICONS
    16-apps-kmines.png
    22-apps-kmines.png
    32-apps-kmines.png
    48-apps-kmines.png
    64-apps-kmines.png
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
)

