#include "gxmlsettings.h"

#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "gstring.h"


const KXMLConfig::Data KXMLConfig::DATA[Nb_ConfigItemType] = {
    { KSimpleConfigItem::NB_TYPES, KSimpleConfigItem::DATA },
    { KRangedConfigItem::NB_TYPES, KRangedConfigItem::DATA },
    { KMultiConfigItem::NB_TYPES, KMultiConfigItem::DATA }
};

QDomDocument *KXMLConfig::_xml = 0;

void KXMLConfig::readConfigFile()
{
    _xml = new QDomDocument;

    QString name = KGlobal::instance()->instanceName();
    QString xml_file = locate("data", name + '/' + name + "config.rc");
    QFile file(xml_file);
    if ( !file.open( IO_ReadOnly ) )
        kdError(1000) << "No XML config file " << xml_file << endl;
    else {
        QByteArray buffer(file.readAll());
        _xml->setContent( QString::fromUtf8(buffer.data(), buffer.size()) );
    }
}

#define ENTRY_NAME "entry named \"" << name << "\")"

KConfigItem *KXMLConfig::createConfigItem(const QString &name, QObject *object,
                                          KConfigCollection *col)
{
    if ( _xml==0 ) readConfigFile();
    QDomElement root = _xml->namedItem("kconfig").toElement();

    // find entry and group
    bool several = false;
    QDomElement entry, grp;
    QDomNodeList groups = root.elementsByTagName("Group");
    for (uint i=0; i<groups.count(); i++) {
        grp = groups.item(i).toElement();
        if ( grp.isNull() ) continue;
        QDomNodeList list = grp.elementsByTagName("Entry");
        for (uint j=0; j<list.count(); j++) {
            QDomElement elt = list.item(j).toElement();
            if ( elt.isNull() || elt.attribute("name")!=name ) continue;
            if ( !entry.isNull() ) several = true;
            entry = elt;
        }
    }
    if ( entry.isNull() ) {
        kdError() << "no " << ENTRY_NAME << endl;
        return 0;
    }
    if (several) kdWarning() << "several " << ENTRY_NAME << endl;

    // read common attributes
    QString group = grp.attribute("group");
    QString key = entry.attribute("key");
    if ( key.isEmpty() ) kdWarning() << ENTRY_NAME << " has empty key" << endl;
    QString stype = entry.attribute("type");
    QString text = entry.namedItem("text").toElement().text();
    QString whatsthis = entry.namedItem("whatsthis").toElement().text();
    QString tooltip = entry.namedItem("tooltip").toElement().text();

    // recognize type
    ConfigItemType type;
    uint specificType;
    const KConfigItem::Data *data = 0;
    for (uint i = 0; i<Nb_ConfigItemType; i++)
        for (uint k = 0; k<DATA[i].nb; k++)
            if ( stype==DATA[i].data[k].typeName ) {
                data = &DATA[i].data[k];
                type = (ConfigItemType)i;
                specificType = k;
                break;
            }
    if ( data==0 ) {
        kdError() << "type not recognized for " << ENTRY_NAME << endl;
        return 0;
    }

    // check default value
    QVariant def = KConfigString::toVariant(entry.attribute("defaultValue"),
                                            data->type);

    // specific
    if ( type==Simple )
        return new KSimpleConfigItem((KSimpleConfigItem::Type)specificType,
                                     object, group, key, def, col, text);
    else if ( type==Ranged ) {
        QVariant min = KConfigString::toVariant(entry.attribute("minValue"),
                                                data->type);
        QVariant max = KConfigString::toVariant(entry.attribute("maxValue"),
                                                data->type);
        return new KRangedConfigItem((KRangedConfigItem::Type)specificType,
                                     object, group, key, def, min, max, col,
                                     text);
    } else if ( type==Multi ) {
        QDomNodeList list = entry.elementsByTagName("Item");
        QStringList names, texts;
        for (uint i=0; i<list.count(); i++) {
            QDomElement item = list.item(i).toElement();
            if ( item.isNull() ) continue;
            names.append(item.attribute("name"));
            texts.append(item.text());
        }
        KMultiConfigItem *mci =
            new KMultiConfigItem((KMultiConfigItem::Type)specificType, object,
                                 names.count(), group, key, def, col, text);
        for (uint i=0; i<names.count(); i++)
            mci->map(i, names[i].utf8(), i18n(texts[i].utf8()));
        return mci;
    }
    return 0;
}
