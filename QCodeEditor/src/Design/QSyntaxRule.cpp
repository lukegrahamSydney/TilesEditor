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
#include <KGL/Design/QSyntaxRule.hpp>
#include <KGL/Design/XmlHelper.hpp>
#include <QtXml/QtXml>
#include <QFont>


namespace kgl {

    ///
    ///  @fn        Default constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    QSyntaxRule::QSyntaxRule()
        : m_Regex(""),
          m_BackColor(Qt::transparent),
          m_ForeColor(Qt::transparent),
          m_Id(""),
          m_StartReg(""),
          m_EndReg(""),
          m_IsGlobal(true),
          m_UseFont(false) {
    }

    ///
    ///  @fn        Copy constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QSyntaxRule::QSyntaxRule(const QSyntaxRule &rule)
        : m_Regex(rule.regex()),
          m_Font(rule.m_Font),
          m_BackColor(rule.backColor()),
          m_ForeColor(rule.foreColor()),
          m_Id(rule.id()),
          m_StartReg(rule.startRegex()),
          m_EndReg(rule.closeRegex()),
          m_IsGlobal(rule.isGlobal()),
          m_UseFont(rule.useFont()) {
    }

    ///
    ///  @fn        Destructor
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    QSyntaxRule::~QSyntaxRule() {
    }


    ///
    ///  @fn        regex
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    const QString &QSyntaxRule::regex() const {
        return m_Regex;
    }

    ///
    ///  @fn        font
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    const QFont &QSyntaxRule::font() const {
        return m_Font;
    }

    ///
    ///  @fn        backColor
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    const QColor &QSyntaxRule::backColor() const {
        return m_BackColor;
    }

    ///
    ///  @fn        backColor
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    const QColor &QSyntaxRule::foreColor() const {
        return m_ForeColor;
    }

    ///
    ///  @fn        id
    ///  @author    Nicolas Kogler
    ///  @date      October 8th, 2016
    ///
    const QString &QSyntaxRule::id() const {
        return m_Id;
    }

    ///
    ///  @fn        startRegex
    ///  @author    Nicolas Kogler
    ///  @date      October 9th, 2016
    ///
    const QString &QSyntaxRule::startRegex() const {
        return m_StartReg;
    }

    ///
    ///  @fn        closeRegex
    ///  @author    Nicolas Kogler
    ///  @date      October 9th, 2016
    ///
    const QString &QSyntaxRule::closeRegex() const {
        return m_EndReg;
    }

    ///
    ///  @fn        isGlobal
    ///  @author    Nicolas Kogler
    ///  @date      October 9th, 2016
    ///
    bool QSyntaxRule::isGlobal() const {
        return m_IsGlobal;
    }

    ///
    ///  @fn        useFont
    ///  @author    Nicolas Kogler
    ///  @date      October 7th, 2016
    ///
    bool QSyntaxRule::useFont() const {
        return m_UseFont;
    }


    ///
    ///  @fn        setRegex
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    void QSyntaxRule::setRegex(const QString &regex) {
        m_Regex = regex;
    }

    ///
    ///  @fn        setKeywords
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    void QSyntaxRule::setKeywords(const QStringList &keywords, bool lineBegin) {
         m_Regex.clear();

        // Appends a regex that requires the given keywords either to be at the
        // start of the line or to be the first non-whitespace on the line.
        if (lineBegin) {
            m_Regex.append("(^|\\s.)(");
        } else {
            m_Regex.append("\\b(");
        }

        // Appends all the keywords as OR'ed expressions
        foreach (const QString &k, keywords) {
            m_Regex.append(k + "|");
        }

        // Adds the closing braces and a word boundary attribute
        m_Regex.append("(?!))\\b");
    }

    ///
    ///  @fn        setFont
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    void QSyntaxRule::setFont(const QFont &font) {
        m_Font = font;
        m_UseFont = true;
    }

    ///
    ///  @fn        setBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    void QSyntaxRule::setBackColor(const QColor &backColor) {
        m_BackColor = backColor;
    }

    ///
    ///  @fn        setForeColor
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///
    void QSyntaxRule::setForeColor(const QColor &foreColor) {
        m_ForeColor = foreColor;
    }

    ///
    ///  @fn        setId
    ///  @author    Nicolas Kogler
    ///  @date      October 8th, 2016
    ///
    void QSyntaxRule::setId(const QString &id) {
        m_Id = id;
    }

    ///
    ///  @fn        setStartRegex
    ///  @author    Nicolas Kogler
    ///  @date      October 9th, 2016
    ///
    void QSyntaxRule::setStartRegex(const QString &regex) {
        m_StartReg = regex;
        m_Regex = regex;
    }

    ///
    ///  @fn        setCloseRegex
    ///  @author    Nicolas Kogler
    ///  @date      October 9th, 2016
    ///
    void QSyntaxRule::setCloseRegex(const QString &regex) {
        m_EndReg = regex;
    }

    ///
    ///  @fn        setGlobal
    ///  @author    Nicolas Kogler
    ///  @date      October 9th, 2016
    ///
    void QSyntaxRule::setGlobal(bool global) {
        m_IsGlobal = global;
    }


    ///
    ///  @fn        loadFromFile
    ///  @author    Nicolas Kogler
    ///  @date      October 7th, 2016
    ///
    QList<QSyntaxRule> QSyntaxRules::loadFromFile(const char *path, const QCodeEditorDesign &design) {
        QList<QSyntaxRule> rules;

        // Attempts to open the XML file
        QFile xmlFile(path);
        if (!xmlFile.open(QIODevice::ReadOnly)) {
            qDebug("kgl::QSyntaxRules: Cannot open XML file.");
            return rules; // check against <retval>.isEmpty();
        }

        // Loads the file into a XML reader and tries
        // to find the <rules> tag.
        QXmlStreamReader xmlReader(&xmlFile);
        if (!xmlReader.readNextStartElement() || xmlReader.name().toString() != "rules") {
            qDebug("kgl::QSyntaxRules: There is no <rules> tag.");
            return rules;
        }

        // Iterates through every rule until reader finished
        while (!xmlReader.hasError()) {
            if (!xmlReader.readNextStartElement()) {
                break; // no more elements
            } else if (xmlReader.name().toString() != "rule") {
                break; // no more rule element
            } else if (xmlReader.isComment()) {
                continue; // comments are ignored
            }

            QSyntaxRule rule;
            QXmlStreamAttributes a = xmlReader.attributes();
            if (!a.isEmpty()) {
                auto first = a.at(0);
                if (first.name().toString() == "id") {
                    rule.setId(first.value().toString());
                } else if (first.name().toString() == "global") {
                    rule.setGlobal((first.value().toString() == "true") ? true : false);
                } if (a.size() > 1) {
                    auto second = a.at(1);
                    if (second.name().toString() == "id") {
                        rule.setId(second.value().toString());
                    } else if (second.name().toString() == "global") {
                        rule.setGlobal((second.value().toString() == "true") ? true : false);
                    }
                }
            }

            // Extracts the information out of the rule child
            while (!(xmlReader.isEndElement() && xmlReader.name().toString() == "rule")) {
                if (!xmlReader.readNextStartElement()) break;
                if (xmlReader.isComment()) break;

                // Attempts to parse the rule
                auto name = xmlReader.name().toString().toLower();
                if (name == "regex") {
                    rule.setRegex(xmlReader.readElementText());
                } else if (name == "startregex") {
                    rule.setStartRegex(xmlReader.readElementText());
                } else if (name == "closeregex") {
                    rule.setCloseRegex(xmlReader.readElementText());
                } else if (name == "keywords") {

                    // Attempts to read the attribute
                    bool firstWord = false;
                    auto attr = xmlReader.attributes();
                    if (!attr.isEmpty() && attr.at(0).name().toString() == "start") {
                        if (attr.at(0).value().toString() == "true") {
                            firstWord = true;
                        }
                    }

                    rule.setKeywords(readKeywords(&xmlReader), firstWord);
                } else if (name == "backcolor") {
                    rule.setBackColor(readColor(&xmlReader));
                } else if (name == "forecolor") {
                    rule.setForeColor(readColor(&xmlReader));
                } else if (name == "font") {
                    rule.setFont(readFont(&xmlReader, design.editorFont()));
                } else {
                    QString s("kgl::QSyntaxRule: Element '%0' is unknown.");
                    qDebug(s.arg(name).toStdString().c_str());
                }
            }

            // Determines if the required element (regex||keywords) was found
            if (rule.regex().isEmpty() && (rule.startRegex().isEmpty() && rule.closeRegex().isEmpty())) {
                qDebug("kgl::QSyntaxRule: Required element 'regex' or 'keywords' not found.");
                return rules;
            } else {
                rules.push_back(rule);
            }
        }

        xmlFile.close();
        return rules;
    }
}
