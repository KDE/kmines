#include "gstring.h"


//-----------------------------------------------------------------------------
bool KConfigString::isUtf8(const char *buf)
{
    int i, n;
    register char c;
    bool gotone = false;

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

  static const char text_chars[256] = {
  /*                  BEL BS HT LF    FF CR    */
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
        /*                              ESC          */
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
        /*            NEL                            */
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
  };

  /* *ulen = 0; */
  for (i = 0; (c = buf[i]); i++) {
    if ((c & 0x80) == 0) {        /* 0xxxxxxx is plain ASCII */
      /*
       * Even if the whole file is valid UTF-8 sequences,
       * still reject it if it uses weird control characters.
       */

      if (text_chars[c] != T)
        return false;

    } else if ((c & 0x40) == 0) { /* 10xxxxxx never 1st byte */
      return false;
    } else {                           /* 11xxxxxx begins UTF-8 */
      int following;

    if ((c & 0x20) == 0) {             /* 110xxxxx */
      following = 1;
    } else if ((c & 0x10) == 0) {      /* 1110xxxx */
      following = 2;
    } else if ((c & 0x08) == 0) {      /* 11110xxx */
      following = 3;
    } else if ((c & 0x04) == 0) {      /* 111110xx */
      following = 4;
    } else if ((c & 0x02) == 0) {      /* 1111110x */
      following = 5;
    } else
      return false;

      for (n = 0; n < following; n++) {
        i++;
        if (!(c = buf[i]))
          goto done;

        if ((c & 0x80) == 0 || (c & 0x40))
          return false;
      }
      gotone = true;
    }
  }
done:
  return gotone;   /* don't claim it's UTF-8 if it's all 7-bit */
}

#undef F
#undef T
#undef I
#undef X

//-----------------------------------------------------------------------------
int KConfigString::toInt(const QCString &s, int def)
{
    if( s.isNull() ) return def;
    else if( s == "true" || s=="on" || s=="yes" ) return 1;
    else {
        bool ok;
        int v = s.toInt( &ok );
        return ( ok ? v : def );
    }
}

uint KConfigString::toUnsignedInt(const QCString &s, uint def)
{
    if( s.isNull() ) return def;
    else {
        bool ok;
        uint v = s.toUInt( &ok );
        return ( ok ? v : def );
    }
}

long KConfigString::toLong(const QCString &s, long def)
{
    if( s.isNull() ) return def;
    else {
        bool ok;
        long v = s.toLong( &ok );
        return( ok ? v : def );
    }
}

unsigned long KConfigString::toUnsignedLong(const QCString &s, unsigned long def)
{
    if( s.isNull() ) return def;
    else {
        bool ok;
        unsigned long v = s.toULong( &ok );
        return ( ok ? v : def );
    }
}

double KConfigString::toDouble(const QCString &s, double def)
{
    if( s.isNull() ) return def;
    else {
        bool ok;
        double v = s.toDouble( &ok );
        return ( ok ? v : def );
    }
}

bool KConfigString::toBool(const QCString &s, bool def)
{
    if( s.isNull() ) return def;
    else {
        if( s=="true" || s=="on" || s=="yes" || s=="1" ) return true;
        else {
            bool ok;
            int v = s.toInt( &ok );
            return ( ok && v!=0 );
        }
    }
}

uint KConfigString::toList(const QCString &s, QStrList &list, char sep)
{
    if ( s.isEmpty() ) return 0;
    list.clear();
    QCString value = "";
    int len = s.length();
    for (int i = 0; i < len; i++) {
        if ( s[i]!=sep && s[i]!='\\' ) {
            value += s[i];
            continue;
        }
        if ( s[i]=='\\' ) {
            i++;
            value += s[i];
            continue;
        }
        list.append(value);
        value.truncate(0);
    }

    if ( s[len-1]!=sep ) list.append(value);
    return (uint)list.count();
}

QRect KConfigString::toRect(const QCString &s, const QRect *def)
{
    if ( !s.isEmpty() ) {
        int left, top, width, height;
        if ( sscanf(s.data(), "%d,%d,%d,%d", &left, &top, &width, &height)==4 )
            return QRect(left, top, width, height);
    }
    if (def) return *def;
    return QRect();
}

QPoint KConfigString::toPoint(const QCString &s, const QPoint *def)
{
    if ( !s.isEmpty() ) {
        int x,y;
        if ( sscanf(s.data(), "%d,%d", &x, &y)==2 )
            return QPoint(x, y);
    }
    if (def) return *def;
    return QPoint();
}

QSize KConfigString::toSize(const QCString &s, const QSize *def)
{
    if ( !s.isEmpty() ) {
        int width, height;
        if ( sscanf(s.data(), "%d,%d", &width, &height)==2 )
            return QSize(width, height);
    }
    if (def) return *def;
    return QSize();
}

QDateTime KConfigString::toDateTime(const QCString &s, const QDateTime *def)
{
    QDateTime aRetDateTime = QDateTime::currentDateTime();

    QStrList list;
    toList(s, list);
    if( list.count()==6 ) {
        QTime time;
        QDate date;
        date.setYMD( QString::fromLatin1( list.at( 0 ) ).toInt(),
                     QString::fromLatin1( list.at( 1 ) ).toInt(),
                     QString::fromLatin1( list.at( 2 ) ).toInt() );
        time.setHMS( QString::fromLatin1( list.at( 3 ) ).toInt(),
                     QString::fromLatin1( list.at( 4 ) ).toInt(),
                     QString::fromLatin1( list.at( 5 ) ).toInt() );
        aRetDateTime.setTime( time );
        aRetDateTime.setDate( date );
    } else if (def) return *def;

    return aRetDateTime;
}

//-----------------------------------------------------------------------------
QStringList KConfigString::toList(const QString &s, char sep)
{
    QStringList list;
    if( s.isEmpty() ) return list;

    QString value = "";
    int len = s.length();
    for(int i = 0; i < len; i++ ) {
        if( s[i]!=sep && s[i]!='\\' ) {
            value += s[i];
            continue;
        }
        if( s[i]=='\\' ) {
            i++;
            value += s[i];
            continue;
        }
        list.append(value);
        value.truncate(0);
    }
    if ( s[len-1]!=sep ) list.append(value);
    return list;
}

QValueList<int> KConfigString::toIntList(const QString &s)
{
    QStringList strlist = toList(s);
    QValueList<int> list;
    for (QStringList::ConstIterator it = strlist.begin(); it != strlist.end();
         it++)
        // I do not check if the toInt failed because I consider the number of
        // items more important than their value
        list << (*it).toInt();
    return list;
}

QFont KConfigString::toFont(const QString &s, const QFont *def)
{
    QFont aRetFont;

    if ( !s.isNull() ) {
        if ( s.contains( ',' ) > 5 ) {
            // KDE3 and upwards entry
            if ( !aRetFont.fromString( s ) && def ) aRetFont = *def;
        } else {
            // backward compatibility with older font formats
            // ### remove KDE 3.1 ?
            // find first part (font family)
            int nIndex = s.find( ',' );
            if ( nIndex == -1 ) {
                if (def) aRetFont = *def;
                return aRetFont;
            }
            aRetFont.setFamily( s.left( nIndex ) );

            // find second part (point size)
            int nOldIndex = nIndex;
            nIndex = s.find( ',', nOldIndex+1 );
            if ( nIndex == -1 ) {
                if (def) aRetFont = *def;
                return aRetFont;
            }

            aRetFont.setPointSize( s.mid( nOldIndex+1,
                                          nIndex-nOldIndex-1 ).toInt() );

            // find third part (style hint)
            nOldIndex = nIndex;
            nIndex = s.find( ',', nOldIndex+1 );

            if ( nIndex == -1 ) {
                if (def) aRetFont = *def;
                return aRetFont;
            }

            aRetFont.setStyleHint( (QFont::StyleHint)s.mid( nOldIndex+1,
                                               nIndex-nOldIndex-1 ).toUInt() );

            // find fourth part (char set)
            nOldIndex = nIndex;
            nIndex = s.find( ',', nOldIndex+1 );

            if ( nIndex == -1 ) {
                if (def) aRetFont = *def;
                return aRetFont;
            }

            QString chStr = s.mid( nOldIndex+1, nIndex-nOldIndex-1 );
            // find fifth part (weight)
            nOldIndex = nIndex;
            nIndex = s.find( ',', nOldIndex+1 );

            if ( nIndex == -1 ) {
                if (def) aRetFont = *def;
                return aRetFont;
            }

            aRetFont.setWeight( s.mid( nOldIndex+1,
                                       nIndex-nOldIndex-1 ).toUInt() );

            // find sixth part (font bits)
            uint nFontBits = s.right( s.length()-nIndex-1 ).toUInt();

            aRetFont.setItalic( nFontBits & 0x01 );
            aRetFont.setUnderline( nFontBits & 0x02 );
            aRetFont.setStrikeOut( nFontBits & 0x04 );
            aRetFont.setFixedPitch( nFontBits & 0x08 );
            aRetFont.setRawMode( nFontBits & 0x20 );
        }
    }
    else if (def) aRetFont = *def;
    return aRetFont;
}

QColor KConfigString::toColor(const QString &s, const QColor *def)
{
    QColor aRetColor;

    if( !s.isEmpty() ) {
      if ( s.at(0)=='#' ) aRetColor.setNamedColor(s);
      else {
          // find first part (red)
          int nIndex = s.find( ',' );
          if ( nIndex==-1 ) {
            if (def) aRetColor = *def;
            return aRetColor;
          }
          bool rok;
          int red = s.left( nIndex ).toInt( &rok );

          // find second part (green)
          int nOldIndex = nIndex;
          nIndex = s.find( ',', nOldIndex+1 );
          if ( nIndex==-1 ) {
            if (def) aRetColor = *def;
            return aRetColor;
          }
          bool gok;
          int green = s.mid( nOldIndex+1, nIndex-nOldIndex-1 ).toInt( &gok );

          // find third part (blue)
          bool bok;
          int blue = s.right( s.length()-nIndex-1 ).toInt( &bok );

          if ( !rok || !gok || !bok ) {
              if (def) aRetColor = *def;
              return aRetColor;
          }
          aRetColor.setRgb( red, green, blue );
      }
    } else if (def) aRetColor = *def;

    return aRetColor;
}

QVariant KConfigString::toVariant(const QString &s, QVariant::Type type)
{
    QVariant va;
    (void)va.cast(type);
    return toVariant(s, va);
}

QVariant KConfigString::toVariant(const QString &s, const QVariant &def)
{
    QValueList<QVariant> list;
    QStringList strList;
    QStringList::ConstIterator it;
    QStringList::ConstIterator end;
    QVariant tmp = def;

    switch( def.type() ) {
    case QVariant::Invalid:
        return QVariant();
    case QVariant::String:
        return QVariant(s);
    case QVariant::StringList:
        return QVariant( toList(s) );
    case QVariant::List:
        strList = toList(s);
        it = strList.begin();
        end = strList.end();
        for (; it != end; ++it ) {
            tmp = *it;
            list.append(tmp);
        }
        return QVariant(list);
    case QVariant::Font:
        return QVariant( toFont(s, &tmp.asFont()) );
    case QVariant::Point:
        return QVariant( toPoint(s.utf8(), &tmp.asPoint()) );
    case QVariant::Rect:
        return QVariant( toRect(s.utf8(), &tmp.asRect()) );
    case QVariant::Size:
        return QVariant( toSize(s.utf8(), &tmp.asSize()) );
    case QVariant::Color:
        return QVariant( toColor(s, &tmp.asColor()) );
    case QVariant::Int:
        return QVariant( toInt(s.utf8(), def.toInt()) );
    case QVariant::UInt:
        return QVariant( toUnsignedInt(s.utf8(), def.toUInt()) );
    case QVariant::Bool:
        return QVariant( toBool(s.utf8(), def.toBool()), 0 );
    case QVariant::Double:
        return QVariant( toDouble(s.utf8(), def.toDouble()) );
    case QVariant::DateTime:
        return QVariant( toDateTime(s.utf8(), &tmp.asDateTime()) );
    case QVariant::Date:
        return QVariant( toDateTime(s.utf8(), &tmp.asDateTime()).date() );

    case QVariant::Pixmap:
    case QVariant::Image:
    case QVariant::Brush:
    case QVariant::Palette:
    case QVariant::ColorGroup:
    case QVariant::Map:
    case QVariant::IconSet:
    case QVariant::PointArray:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
    case QVariant::Time:
    case QVariant::CString:
    case QVariant::ByteArray:
    case QVariant::BitArray:
    case QVariant::KeySequence:
    case QVariant::Pen:
        break;
    }

    Q_ASSERT(false);
    return QVariant();
}

//-----------------------------------------------------------------------------
QString KConfigString::from(const QStrList &list, char sep)
{
    if ( list.isEmpty() ) return QString::fromLatin1("");

    QString str_list;
    QStrListIterator it( list );
    for( ; it.current(); ++it ) {
        uint i;
        QString value;
        // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
        // A QStrList may contain values in 8bit locale cpecified
        // encoding or in UTF8 encoding.
        if (isUtf8(it.current()))
            value = QString::fromUtf8(it.current());
        else value = QString::fromLocal8Bit(it.current());
        for( i = 0; i < value.length(); i++ ) {
            if( value[i]==sep || value[i]=='\\' )
                str_list += '\\';
            str_list += value[i];
        }
        str_list += sep;
    }
    if( str_list.at(str_list.length() - 1)==sep )
        str_list.truncate( str_list.length() -1 );
    return str_list;
}

QString KConfigString::from(const QStringList &list, char sep)
{
    if ( list.isEmpty() ) return QString::fromLatin1("");

    QString str_list;
    QStringList::ConstIterator it = list.begin();
    for( ; it != list.end(); ++it ) {
        QString value = *it;
        for (uint i = 0; i < value.length(); i++ ) {
            if( value[i]==sep || value[i]=='\\' )
                str_list += '\\';
            str_list += value[i];
        }
        str_list += sep;
    }
    if( str_list.at(str_list.length() - 1)==sep )
        str_list.truncate( str_list.length() -1 );
    return str_list;
}

QString KConfigString::from(const QValueList<int> &list)
{
    QStringList strlist;
    QValueList<int>::ConstIterator end = list.end();
    for (QValueList<int>::ConstIterator it = list.begin(); it != end; it++)
        strlist << QString::number(*it);
    return from(strlist);
}

QString KConfigString::from(const QRect &rect)
{
    QStrList list;
    QCString tmp;
    list.insert( 0, tmp.setNum( rect.left() ) );
    list.insert( 1, tmp.setNum( rect.top() ) );
    list.insert( 2, tmp.setNum( rect.width() ) );
    list.insert( 3, tmp.setNum( rect.height() ) );
    return from(list);
}

QString KConfigString::from(const QPoint &point)
{
    QStrList list;
    QCString tmp;
    list.insert( 0, tmp.setNum( point.x() ) );
    list.insert( 1, tmp.setNum( point.y() ) );
    return from(list);
}

QString KConfigString::from(const QSize &size)
{
    QStrList list;
    QCString tmp;
    list.insert( 0, tmp.setNum( size.width() ) );
    list.insert( 1, tmp.setNum( size.height() ) );
    return from(list);
}

QString KConfigString::from(const QColor &color)
{
    if ( color.isValid() ) {
        QString s;
        s.sprintf( "%d,%d,%d", color.red(), color.green(), color.blue() );
        return s;
    } else return "invalid";
}

QString KConfigString::from(const QDateTime &dateTime)
{
     QStrList list;
     QCString tmp;
     QTime time = dateTime.time();
     QDate date = dateTime.date();

     list.insert( 0, tmp.setNum( date.year() ) );
     list.insert( 1, tmp.setNum( date.month() ) );
     list.insert( 2, tmp.setNum( date.day() ) );

     list.insert( 3, tmp.setNum( time.hour() ) );
     list.insert( 4, tmp.setNum( time.minute() ) );
     list.insert( 5, tmp.setNum( time.second() ) );

     return from(list);
}

QString KConfigString::from(const QVariant &v)
{
    QValueList<QVariant> list;
    QValueList<QVariant>::ConstIterator it;
    QValueList<QVariant>::ConstIterator end;
    QStringList strList;

    switch ( v.type() ) {
    case QVariant::Invalid:
        return QString::null;
    case QVariant::String:
        return v.toString();
    case QVariant::StringList:
        return from( v.toStringList() );
    case QVariant::List:
        list = v.toList();
        it = list.begin();
        end = list.end();
        for (; it != end; ++it )
            strList.append( (*it).toString() );
        return from(strList);
    case QVariant::Font:
        return from( v.toFont() );
    case QVariant::Point:
        return from( v.toPoint() );
    case QVariant::Rect:
        return from( v.toRect() );
    case QVariant::Size:
        return from( v.toSize() );
    case QVariant::Color:
        return from( v.toColor() );
    case QVariant::Int:
        return from( v.toInt() );
    case QVariant::UInt:
        return from( v.toUInt() );
    case QVariant::Bool:
        return from( v.toBool() );
    case QVariant::Double:
        return from( v.toDouble() );
    case QVariant::DateTime:
        return from( v.toDateTime() );
    case QVariant::Date:
        return from( QDateTime(v.toDate()) );

    case QVariant::Pixmap:
    case QVariant::Image:
    case QVariant::Brush:
    case QVariant::Palette:
    case QVariant::ColorGroup:
    case QVariant::Map:
    case QVariant::IconSet:
    case QVariant::CString:
    case QVariant::PointArray:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
    case QVariant::Time:
    case QVariant::ByteArray:
    case QVariant::BitArray:
    case QVariant::KeySequence:
    case QVariant::Pen:
        break;
    }

    Q_ASSERT( 0 );
    return QString::null;
}

