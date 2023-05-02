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


#ifndef __KGL_QLINECOLUMNPADDING_HPP__
#define __KGL_QLINECOLUMNPADDING_HPP__


//
//  Included headers
//
#include <KGL/KGLConfig.hpp>


namespace kgl {

    ///
    ///  @file      QLineColumnPadding.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///  @class     QLineColumnPadding
    ///  @brief     Defines a padding in the right- and left direction.
    ///
    class KGL_API QLineColumnPadding {
    public:

        ///
        ///  @fn    Default constructor
        ///  @brief Initializes a new instance of QLineColumnPadding.
        ///
        QLineColumnPadding();

        ///
        ///  @fn    Constructor
        ///  @brief Constructs a padding out of a QSize.
        ///  @param size Size representing left and right padding
        ///
        QLineColumnPadding(const QSize &size);

        ///
        ///  @fn    Constructor
        ///  @brief Initializes a new instance of QLineColumnPadding.
        ///  @param left Initial left padding
        ///  @param right Initial right padding
        ///
        QLineColumnPadding(quint32 left, quint32 right);

        ///
        ///  @fn     Destructor
        ///  @brief  Frees all resources allocated by QLineColumnPadding.
        ///
        ~QLineColumnPadding();


        ///
        ///  @fn      left : const
        ///  @brief   Retrieves the padding in the left direction.
        ///  @returns the left padding.
        ///
        quint32 left() const;

        ///
        ///  @fn      right : const
        ///  @brief   Retrieves the padding in the right direction.
        ///  @returns the right padding.
        ///
        quint32 right() const;


        ///
        ///  @fn    setLeft
        ///  @brief Specifies the left padding
        ///  @param left Left padding, in pixels
        ///
        void setLeft(quint32 left);

        ///
        ///  @fn    setRight
        ///  @brief Specifies the right padding
        ///  @param left Right padding, in pixels
        ///
        void setRight(quint32 right);


    private:

        //
        // Private class members
        //
        quint32 m_Left;
        quint32 m_Right;
    };
}


#endif  // __KGL_QLINECOLUMNPADDING_HPP__
