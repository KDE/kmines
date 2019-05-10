
install( PROGRAMS org.kde.kmines.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES org.kde.kmines.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

ecm_install_icons(ICONS
    16-apps-kmines.png
    22-apps-kmines.png
    32-apps-kmines.png
    48-apps-kmines.png
    64-apps-kmines.png
    128-apps-kmines.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)

