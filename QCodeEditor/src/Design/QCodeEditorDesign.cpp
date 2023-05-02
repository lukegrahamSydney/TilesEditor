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
#include <KGL/Design/QCodeEditorDesign.hpp>
#include <KGL/Design/XmlHelper.hpp>
#include <qrgb.h>


namespace kgl {

    ///
    ///  @fn        Default constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    QCodeEditorDesign::QCodeEditorDesign()
        : m_EditorBackColor(0xffffffff),
          m_EditorTextColor(0xff333333),
          m_EditorBorderColor(0xffb9b9b9),
          m_LineColumnBackColor(0xffe9e9e9),
          m_LineColumnTextColor(0xff6a9fc6),
          m_LineColumnSeparatorColor(0xffb9b9b9),
          m_ActiveLineColor(0xfffc9100),
          m_IntelliBoxBackColor(0xfffafafa),
          m_IntelliBoxTextColor(0xff333333),
          m_IntelliBoxBorderColor(0xffb9b9b9),
          m_IntelliBoxSelectionBackColor(0xffc0dcf3),
          m_IntelliBoxSelectionBorderColor(0xff90c8f6),
          m_IntelliBoxPressBackColor(0xff90c8f6),
          m_IntelliBoxPressBorderColor(0xff60b0f9),
          m_EditorBorder(QMargins(0,0,0,0)),
          m_IntelliBoxBorder(QMargins(1,1,1,1)),
          m_PopupSize(200, 200),
          m_HasLineColumn(true),
          m_ShowFocusRect(false),
          m_FirstLineOne(true) {

        // Tries to find a monospaced font
    #ifndef Q_OS_WIN32
        m_EditorFont.setFamily("Monospace");
        m_IntelliBoxFont.setFamily("Monospace");
        m_EditorFont.setStyleHint(QFont::TypeWriter);
        m_IntelliBoxFont.setStyleHint(QFont::TypeWriter);
    #else
        m_EditorFont.setFamily("Consolas");
        m_IntelliBoxFont.setFamily("Consolas");
    #endif
        m_EditorFont.setPointSize(9);
        m_IntelliBoxFont.setPointSize(9);
    }

    ///
    ///  @fn        Copy constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QCodeEditorDesign::QCodeEditorDesign(const QCodeEditorDesign &design)
        : m_EditorBackColor(design.editorBackColor()),
          m_EditorTextColor(design.editorTextColor()),
          m_EditorBorderColor(design.editorBorderColor()),
          m_LineColumnBackColor(design.lineColumnBackColor()),
          m_LineColumnTextColor(design.lineColumnTextColor()),
          m_LineColumnSeparatorColor(design.lineColumnSeparatorColor()),
          m_ActiveLineColor(design.activeLineColor()),
          m_IntelliBoxBackColor(design.intelliBoxBackColor()),
          m_IntelliBoxTextColor(design.intelliBoxTextColor()),
          m_IntelliBoxBorderColor(design.intelliBoxBorderColor()),
          m_IntelliBoxSelectionBackColor(design.intelliBoxSelectionBackColor()),
          m_IntelliBoxSelectionBorderColor(design.intelliBoxSelectionBorderColor()),
          m_IntelliBoxPressBackColor(design.intelliBoxPressBackColor()),
          m_IntelliBoxPressBorderColor(design.intelliBoxPressBorderColor()),
          m_EditorFont(design.editorFont()),
          m_IntelliBoxFont(design.intelliBoxFont()),
          m_EditorBorder(design.editorBorder()),
          m_IntelliBoxBorder(design.intelliBoxBorder()),
          m_PopupSize(design.popupSize()),
          m_HasLineColumn(design.isLineColumnVisible()),
          m_ShowFocusRect(design.hasFocusRect()) {
    }

    ///
    ///  @fn        Constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 18th, 2016
    ///
    QCodeEditorDesign::QCodeEditorDesign(const QString &path)
        : QCodeEditorDesign() {
        QFile file(path);

        // Determines whether file can be opened
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug("kgl::QCodeEditorDesign: Cannot open XML file.");
            return;
        }

        // Attempts to read top element
        QXmlStreamReader xmlReader(&file);
        if (!xmlReader.readNextStartElement() && xmlReader.name().toString() != "design") {
            qDebug("kgl::QCodeEditorDesign: Top tag is not <design>.");
            return;
        }


        while (!xmlReader.hasError() && !xmlReader.atEnd()) {
            if (!xmlReader.readNextStartElement()) {
                continue;
            }

            QString name = xmlReader.name().toString().toLower();

            // Order and case of elements does not matter
            if (name == "editorbackcolor") {
                m_EditorBackColor = readColor(&xmlReader);
            } else if (name == "editortextcolor") {
                m_EditorTextColor = readColor(&xmlReader);
            } else if (name == "editorbordercolor") {
                m_EditorBorderColor = readColor(&xmlReader);
            } else if (name == "linecolumnbackcolor") {
                m_LineColumnBackColor = readColor(&xmlReader);
            } else if (name == "linecolumntextcolor") {
                m_LineColumnTextColor = readColor(&xmlReader);
            } else if (name == "linecolumnseparatorcolor") {
                m_LineColumnSeparatorColor = readColor(&xmlReader);
            } else if (name == "activelinecolor") {
                m_ActiveLineColor = readColor(&xmlReader);
            } else if (name == "intelliboxbackcolor") {
                m_IntelliBoxBackColor = readColor(&xmlReader);
            } else if (name == "intelliboxtextcolor") {
                m_IntelliBoxTextColor = readColor(&xmlReader);
            } else if (name == "intelliboxbordercolor") {
                m_IntelliBoxBorderColor = readColor(&xmlReader);
            } else if (name == "intelliboxselectionbackcolor") {
                m_IntelliBoxSelectionBackColor = readColor(&xmlReader);
            } else if (name == "intelliboxselectionbordercolor") {
                m_IntelliBoxSelectionBorderColor = readColor(&xmlReader);
            } else if (name == "intelliboxpressbackcolor") {
                m_IntelliBoxPressBackColor = readColor(&xmlReader);
            } else if (name == "intelliboxpressbordercolor") {
                m_IntelliBoxPressBorderColor = readColor(&xmlReader);
            } else if (name == "editorborder") {
                m_EditorBorder = readMargin(&xmlReader);
            } else if (name == "intelliboxborder") {
                m_IntelliBoxBorder = readMargin(&xmlReader);
            } else if (name == "linecolumnpadding") {
                m_LineColumnPadding = QLineColumnPadding(readSize(&xmlReader));
            } else if (name == "popupsize") {
                m_PopupSize = readSize(&xmlReader);
            } else if (name == "haslinecolumn") {
                m_HasLineColumn = readBool(&xmlReader);
            } else if (name == "showfocusrect") {
                m_ShowFocusRect = readBool(&xmlReader);
            } else if (name == "firstlineone") {
                m_FirstLineOne = readBool(&xmlReader);
            } else if (name == "editorfont") {
                m_EditorFont = readFont(&xmlReader, QFont("Monospace"));
            } else if (name == "intelliboxfont") {
                m_IntelliBoxFont = readFont(&xmlReader, QFont("Monospace"));
            } else {
                QString s("kgl::QCodeEditorDesign: Element '%0' is unknown.");
                qDebug(s.arg(name).toStdString().c_str());
            }
        }

        file.close();
    }

    ///
    ///  @fn        Destructor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    QCodeEditorDesign::~QCodeEditorDesign() {
    }


    ///
    ///  @fn        editorBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::editorBackColor() const {
        return m_EditorBackColor;
    }

    ///
    ///  @fn        editorTextColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::editorTextColor() const {
        return m_EditorTextColor;
    }

    ///
    ///  @fn        editorBorderColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::editorBorderColor() const {
        return m_EditorBorderColor;
    }

    ///
    ///  @fn        lineColumnBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::lineColumnBackColor() const {
        return m_LineColumnBackColor;
    }

    ///
    ///  @fn        lineColumnTextColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::lineColumnTextColor() const {
        return m_LineColumnTextColor;
    }

    ///
    ///  @fn        lineColumnSeparatorColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::lineColumnSeparatorColor() const {
        return m_LineColumnSeparatorColor;
    }

    ///
    ///  @fn        activeLineColor
    ///  @author    Nicolas Kogler
    ///  @date      October 15th, 2016
    ///
    const QColor &QCodeEditorDesign::activeLineColor() const {
        return m_ActiveLineColor;
    }

    ///
    ///  @fn        intelliBoxBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::intelliBoxBackColor() const {
        return m_IntelliBoxBackColor;
    }

    ///
    ///  @fn        intelliBoxTextColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::intelliBoxTextColor() const {
        return m_IntelliBoxTextColor;
    }

    ///
    ///  @fn        intelliBoxBorderColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::intelliBoxBorderColor() const {
        return m_IntelliBoxBorderColor;
    }

    ///
    ///  @fn        intelliBoxSelectionBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::intelliBoxSelectionBackColor() const {
        return m_IntelliBoxSelectionBackColor;
    }

    ///
    ///  @fn        intelliBoxSelectionBorderColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::intelliBoxSelectionBorderColor() const {
        return m_IntelliBoxSelectionBorderColor;
    }

    ///
    ///  @fn        intelliBoxPressBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::intelliBoxPressBackColor() const {
        return m_IntelliBoxPressBackColor;
    }

    ///
    ///  @fn        intelliBoxPressBorderColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QColor &QCodeEditorDesign::intelliBoxPressBorderColor() const {
        return m_IntelliBoxPressBorderColor;
    }

    ///
    ///  @fn        editorFont
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QFont &QCodeEditorDesign::editorFont() const {
        return m_EditorFont;
    }

    ///
    ///  @fn        intelliBoxFont
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QFont &QCodeEditorDesign::intelliBoxFont() const {
        return m_IntelliBoxFont;
    }

    ///
    ///  @fn        editorBorder
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QMargins &QCodeEditorDesign::editorBorder() const {
        return m_EditorBorder;
    }

    ///
    ///  @fn        intelliBoxBorder
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QMargins &QCodeEditorDesign::intelliBoxBorder() const {
        return m_IntelliBoxBorder;
    }

    ///
    ///  @fn        lineColumnPadding
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QLineColumnPadding &QCodeEditorDesign::lineColumnPadding() const {
        return m_LineColumnPadding;
    }

    ///
    ///  @fn        popupSize
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    const QSize &QCodeEditorDesign::popupSize() const {
        return m_PopupSize;
    }

    ///
    ///  @fn        isLineColumnVisible
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    bool QCodeEditorDesign::isLineColumnVisible() const {
        return m_HasLineColumn;
    }

    ///
    ///  @fn        hasFocusRect
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    bool QCodeEditorDesign::hasFocusRect() const {
        return m_ShowFocusRect;
    }

    ///
    ///  @fn        startsWithOne
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    bool QCodeEditorDesign::startsWithOne() const {
        return m_FirstLineOne;
    }


    ///
    ///  @fn        setEditorBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setEditorBackColor(const QColor &color) {
        m_EditorBackColor = color;
    }

    ///
    ///  @fn        setEditorTextColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setEditorTextColor(const QColor &color) {
        m_EditorTextColor = color;
    }

    ///
    ///  @fn        setEditorBorderColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setEditorBorderColor(const QColor &color) {
        m_EditorBorderColor = color;
    }

    ///
    ///  @fn        setLineColumnBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setLineColumnBackColor(const QColor &color) {
        m_LineColumnBackColor = color;
    }

    ///
    ///  @fn        setLineColumnTextColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setLineColumnTextColor(const QColor &color) {
        m_LineColumnTextColor = color;
    }

    ///
    ///  @fn        setActiveLineColor
    ///  @author    Nicolas Kogler
    ///  @date      October 15th, 2016
    ///
    void QCodeEditorDesign::setActiveLineColor(const QColor &color) {
        m_ActiveLineColor = color;
    }

    ///
    ///  @fn        setLineColumnSeparatorColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setLineColumnSeparatorColor(const QColor &color) {
        m_LineColumnSeparatorColor = color;
    }

    ///
    ///  @fn        setIntelliBoxBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxBackColor(const QColor &color) {
        m_IntelliBoxBackColor = color;
    }

    ///
    ///  @fn        setIntelliBoxTextColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxTextColor(const QColor &color) {
        m_IntelliBoxTextColor = color;
    }

    ///
    ///  @fn        setIntelliBoxBorderColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxBorderColor(const QColor &color) {
        m_IntelliBoxBorderColor = color;
    }

    ///
    ///  @fn        setIntelliBoxSelectionBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxSelectionBackColor(const QColor &color) {
        m_IntelliBoxSelectionBackColor = color;
    }

    ///
    ///  @fn        setIntelliBoxSelectionBorderColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxSelectionBorderColor(const QColor &color) {
        m_IntelliBoxSelectionBorderColor = color;
    }

    ///
    ///  @fn        setIntelliBoxPressBackColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxPressBackColor(const QColor &color) {
        m_IntelliBoxPressBackColor = color;
    }

    ///
    ///  @fn        setIntelliBoxPressBorderColor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxPressBorderColor(const QColor &color) {
        m_IntelliBoxPressBorderColor = color;
    }

    ///
    ///  @fn        setEditorFont
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setEditorFont(const QFont &font) {
        m_EditorFont = font;
    }

    ///
    ///  @fn        setIntelliBoxFont
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxFont(const QFont &font) {
        m_IntelliBoxFont = font;
    }

    ///
    ///  @fn        setEditorBorder
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setEditorBorder(const QMargins &border) {
        m_EditorBorder = border;
    }

    ///
    ///  @fn        setIntelliBoxBorder
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setIntelliBoxBorder(const QMargins &border) {
        m_IntelliBoxBorder = border;
    }

    ///
    ///  @fn        setLineColumnPadding
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setLineColumnPadding(const QLineColumnPadding &padding) {
        m_LineColumnPadding = padding;
    }

    ///
    ///  @fn        setLineColumnVisible
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditorDesign::setLineColumnVisible(bool visible) {
        m_HasLineColumn = visible;
    }

    ///
    ///  @fn        showFocusRect
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    void QCodeEditorDesign::showFocusRect(bool show) {
        m_ShowFocusRect = show;
    }

    ///
    ///  @fn        setFirstLineAsOne
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    void QCodeEditorDesign::setFirstLineAsOne(bool one) {
        m_FirstLineOne = one;
    }
}
