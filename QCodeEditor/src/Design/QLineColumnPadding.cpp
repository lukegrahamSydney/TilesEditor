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
#include <KGL/Design/QLineColumnPadding.hpp>


namespace kgl {

    ///
    ///  @fn        Default constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    QLineColumnPadding::QLineColumnPadding()
        : m_Left(16),
          m_Right(16) {
    }

    ///
    ///  @fn        Default constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 19th, 2016
    ///
    QLineColumnPadding::QLineColumnPadding(const QSize &size)
        : m_Left(static_cast<quint32>(size.width())),
          m_Right(static_cast<quint32>(size.height())) {

    }

    ///
    ///  @fn        Constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    QLineColumnPadding::QLineColumnPadding(quint32 left, quint32 right)
        : m_Left(left),
          m_Right(right) {
    }

    ///
    ///  @fn        Destructor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    QLineColumnPadding::~QLineColumnPadding() {
    }


    ///
    ///  @fn        left
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    quint32 QLineColumnPadding::left() const {
        return m_Left;
    }

    ///
    ///  @fn        right
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    quint32 QLineColumnPadding::right() const {
        return m_Right;
    }


    ///
    ///  @fn        setLeft
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QLineColumnPadding::setLeft(quint32 left) {
        m_Left = left;
    }

    ///
    ///  @fn        setRight
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QLineColumnPadding::setRight(quint32 right) {
        m_Right = right;
    }
}
