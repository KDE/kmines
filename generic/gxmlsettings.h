#ifndef KXMLCONFIG_h
#define KXMLCONFIG_h

#include "gsettings.h"


class QDomDocument;

class KXMLConfig
{
 public:
    static KConfigItem *createConfigItem(const QString &name, QObject *object,
                                         KConfigCollection *col);

 private:
    static QDomDocument *_xml;

    enum ConfigItemType { Simple = 0, Ranged, Multi, Nb_ConfigItemType };
    struct Data {
        uint nb;
        const KConfigItem::Data *data;
    };
    static const Data DATA[Nb_ConfigItemType];

    static void readConfigFile();
};

#endif
