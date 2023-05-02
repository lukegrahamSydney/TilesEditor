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


#ifndef __KGL_XMLHELPER_HPP__
#define __KGL_XMLHELPER_HPP__


//
//  Included headers
//
#include <QXmlStreamReader>
#include <QMargins>


namespace kgl {

    ///
    ///  @file      XmlHelper.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 18th, 2016
    ///  @brief     Declares several functions for parsing XML elements.
    ///

    ///
    ///  @fn      readBool
    ///  @brief   Reads an XML element and converts it to a boolean.
    ///  @param   reader Current XML reader
    ///  @returns true if element is string "true", otherwise false.
    ///
    extern bool readBool(QXmlStreamReader *reader);

    ///
    ///  @fn      readColor
    ///  @brief   Reads an XML element and converts it to a color.
    ///  @param   reader Current XML reader
    ///  @returns the color representing the inline text.
    ///
    /// Supported:
    /// Hexadecimal notation (e.g. '#abcdef')
    /// Color strings (e.g. 'red')
    /// RGBA notation (e.g. 'rgba(r, g, b, a)
    ///
    extern QColor readColor(QXmlStreamReader *reader);

    ///
    ///  @fn      readKeywords
    ///  @brief   Reads an XML element and converts it to a list of strings.
    ///  @param   reader Current XML reader
    ///  @returns all the words separated by whitespace
    ///
    extern QStringList readKeywords(QXmlStreamReader *reader);

    ///
    ///  @fn      readMargin
    ///  @brief   Reads a margin with 4 values.
    ///  @param   reader Current XML reader
    ///  @returns a QMargins structure.
    ///
    extern QMargins readMargin(QXmlStreamReader *reader);

    ///
    ///  @fn      readSize
    ///  @brief   Reads a size with 2 values.
    ///  @param   reader Current XML reader
    ///  @returns a QSize structure.
    extern QSize readSize(QXmlStreamReader *reader);

    ///
    ///  @fn      readFont
    ///  @brief   Reads a font structure from the XML reader.
    ///  @param   reader Current XML reader
    ///  @param   def Default font, in case no family specified
    ///  @returns a QFont structure.
    ///
    extern QFont readFont(QXmlStreamReader *reader, const QFont &def);
}


#endif  // __KGL_XMLHELPER_HPP__
