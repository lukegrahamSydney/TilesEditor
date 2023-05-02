//
//  QCodeEditor - Widget to highlight and auto-complete code.
//  Copyright (C) 2016 Nicolas Kogler (kogler.cml@hotmail.com)
//
//  This file is part of QCodeEditor.
//
//  QCodeEditor is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with QCodeEditor.  If not, see <http://www.gnu.org/licenses/>.
//
//


//
//  Included headers
//
#include <KGL/Design/XmlHelper.hpp>
#include <QColor>
#include <QSize>
#include <QFont>
#include <QRegularExpression>

namespace kgl {

    ///
    ///  @fn        convertNumber
    ///  @author    Nicolas Kogler
    ///  @date      October 18th, 2016
    ///
    inline bool convertNumber(const QString &str, int *out) {
        bool success;
        *out = str.toInt(&success);
        return success;
    }

    ///
    ///  @fn        readFloat
    ///  @author    Nicolas Kogler
    ///  @date      October 19th, 2016
    inline float readFloat(QXmlStreamReader *reader) {
        QString str = reader->readElementText();
        bool success;

        // Attempts to convert float
        float result = str.toFloat(&success);
        if (!success) {
            qDebug("XmlHelper: Invalid float!");
        }

        return result;
    }

    ///
    ///  @fn        readBool
    ///  @author    Nicolas Kogler
    ///  @date      October 7th, 2016
    ///
    bool readBool(QXmlStreamReader *reader) {
        auto string = reader->readElementText();
        if (string.isEmpty() ||
            string != "true") {
            return false;
        } else {
            return true;
        }
    }

    ///
    ///  @fn        readColor
    ///  @author    Nicolas Kogler
    ///  @date      October 7th, 2016
    ///
    QColor readColor(QXmlStreamReader *reader) {
        auto string = reader->readElementText();
        if (string.isEmpty()) {
            return QColor(Qt::transparent);
        }

        // Attempts to replace a hashtag with '0x'
        if (string.startsWith('#')) {
            string.remove(0, 1);
            string.insert(0, "0x");
        }

        // Tries to convert it to a number;
        // if it failed, is a color name or rgba
        bool success = false;
        quint32 rgba = string.toUInt(&success, 16);
        if (success == true) {
            if (rgba <= 0xFFFFFF) {
                // make it format '0xFF<rgb>'
                return QColor((0xffu << 24) | rgba);
            } else {
                return QColor(rgba);
            }
        } else {
            if (QColor::isValidColor(string)) {
                return QColor(string);
            } else {
                if (string.startsWith("rgba")) {
                    string.remove(0,4);
                    string.remove(QRegularExpression("(\\(|\\))"));

                    QStringList s = string.split(',', Qt::SkipEmptyParts);
                    int r, g, b, a;
                    if (convertNumber(s.at(0), &r) &&
                        convertNumber(s.at(1), &g) &&
                        convertNumber(s.at(2), &b) &&
                        convertNumber(s.at(3), &a)) {
                        return QColor(r, g, b, a);
                    } else {
                        return QColor(Qt::transparent);
                    }
                } else if (string.startsWith("rgb")) {
                    string.remove(0,4);
                    
                    string.remove(QRegularExpression("(\\(|\\))"));

                    QStringList s = string.split(',', Qt::SkipEmptyParts);
                    int r, g, b;
                    if (convertNumber(s.at(0), &r) &&
                        convertNumber(s.at(1), &g) &&
                        convertNumber(s.at(2), &b)) {
                        return QColor(r, g, b);
                    } else {
                        QString e("XML: Value of element '%0' is invalid. Value: '%1'.");
                        qDebug(e.arg(reader->name().toString(), string).toStdString().c_str());
                        return QColor(Qt::transparent);
                    }
                }
            }
        }

        return QColor(Qt::transparent);
    }

    ///
    ///  @fn        readKeywords
    ///  @author    Nicolas Kogler
    ///  @date      October 7th, 2016
    ///
    QStringList readKeywords(QXmlStreamReader *reader) {

        // Simplified fits our needs: Removes trailing and leading space
        // and converts multiple spaces to a single space. We have multiple
        // spaces as: "\n     " -> results in "\n ".
        QString string = reader->readElementText().simplified();

        // Now we just have to remove all the newlines
        string.remove("\n");

        // A single whitespace is our separator for the keywords,
        // thus split the string up accordingly.
        return string.split(' '); // safe to not include empty parts
    }

    ///
    ///  @fn        readMargin
    ///  @author    Nicolas Kogler
    ///  @date      October 18th, 2016
    ///
    QMargins readMargin(QXmlStreamReader *reader) {
        QString str = reader->readElementText().simplified();
        QStringList li = str.split(' ');
        if (li.size() != 4) {
            QString e("XML: Value of element '%0' is invalid. Value: '%1'.");
            qDebug(e.arg(reader->name().toString(), str).toStdString().c_str());
            return QMargins();
        }

        int l, r, t, b;
        if (convertNumber(li.at(0), &l) &&
            convertNumber(li.at(1), &r) &&
            convertNumber(li.at(2), &t) &&
            convertNumber(li.at(3), &b)) {
            return QMargins(l, r, t, b);
        } else {
            QString e("XML: Value of element '%0' is invalid. Value: '%1'.");
            qDebug(e.arg(reader->name().toString(), str).toStdString().c_str());
            return QMargins();
        }
    }

    ///
    ///  @fn        readSize
    ///  @author    Nicolas Kogler
    ///  @date      October 19th, 2016
    ///
    QSize readSize(QXmlStreamReader *reader) {
        QString str = reader->readElementText().simplified();
        QStringList li = str.split(' ');
        if (li.size() != 2) {
            QString e("XML: Value of element '%0' is invalid. Value: '%1'.");
            qDebug(e.arg(reader->name().toString(), str).toStdString().c_str());
            return QSize();
        }

        int w, h;
        if (convertNumber(li.at(0), &w) &&
            convertNumber(li.at(1), &h)) {
            return QSize(w, h);
        } else {
            QString e("XML: Value of element '%0' is invalid. Value: '%1'.");
            qDebug(e.arg(reader->name().toString(), str).toStdString().c_str());
            return QSize();
        }
    }

    ///
    ///  @fn        readFont
    ///  @author    Nicolas Kogler
    ///  @date      October 19th, 2016
    ///
    QFont readFont(QXmlStreamReader *reader, const QFont &def) {
        QString parent = reader->name().toString();
        if (!reader->readNextStartElement()) {
            return def;
        }

        // Determines whether default font should be used
        QFont usedFont;
        if (reader->name().toString() == "family") {
            usedFont.setFamily(reader->readElementText());
            usedFont.setStyleHint(QFont::TypeWriter);
            reader->readNextStartElement();
        } else {
            usedFont = def;
        }

        // Reads the font properties
        while (!(reader->isEndElement() && reader->name() == parent)) {
            auto prop = reader->name().toString();
            if (prop == "strikethrough") {
                usedFont.setStrikeOut(readBool(reader));
            } else if (prop == "underline") {
                usedFont.setUnderline(readBool(reader));
            } else if (prop == "italic") {
                usedFont.setItalic(readBool(reader));
            } else if (prop == "bold") {
                usedFont.setBold(readBool(reader));
            } else if (prop == "pointsize") {
                usedFont.setPointSizeF(readFloat(reader));
            } reader->readNextStartElement();
        }

        return usedFont;
    }
}
