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


#ifndef __KGL_QCODEEDITORSHEETS_HPP__
#define __KGL_QCODEEDITORSHEETS_HPP__


//
//  Included headers
//
#include <KGL/KGLConfig.hpp>
#include <KGL/Design/QCodeEditorDesign.hpp>


namespace kgl {

    ///
    ///  @file      QCodeEditorSheets.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 7th, 2016
    ///  @def       CSS_Editor_Widget
    ///  @brief     Stylesheet for the editor widget.
    ///
    const char CSS_Editor_Widget[] = {
        "QPlainTextEdit {"
        "   border-top: %tpx solid #%c;"
        "   border-right: %rpx solid #%c;"
        "   border-bottom: %bpx solid #%c;"
        "   border-left: %lpx solid #%c;"
        "}"
    };

    ///
    ///  @file      QCodeEditorSheets.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///  @def       CSS_Popup_Widget
    ///  @brief     Stylesheet for the popup widget.
    ///
    const char CSS_Popup_Widget[] = {
        "QListView {"
        "   border-top: %tpx solid #%border;"
        "   border-left: %lpx solid #%border;"
        "   border-right: %rpx solid #%border;"
        "   border-bottom: %bpx solid #%border;"
        "   background-color: #%back;"
        "   color: #%text;"
        "   outline: %focus;"
        "}"
        "QListView::item:selected {"
        "   padding: -1px;"
        "   border: 1px solid #%selbrd;"
        "   background-color: #%selback;"
        "   color: #%text;"
        "}"
        "QListView::item:!selected:hover {"
        "   background: transparent;"
        "}"
    };


    ///
    ///  @file      QCodeEditorSheets.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///  @class     QCodeEditorSheets
    ///
    class QCodeEditorSheets {
    public:

        ///
        ///  @fn      border
        ///  @brief   Retrieves the style-sheet for the border.
        ///  @param   design Current code editor design
        ///  @returns the style-sheet for the border.
        ///
        static QString border(const QCodeEditorDesign &design);
    };

    ///
    ///  @file      QCodeEditorSheets.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///  @class     QCodeEditorPopupSheets
    ///
    class QCodeEditorPopupSheets {
    public:

        ///
        ///  @fn      hover
        ///  @brief   Retrieves the style-sheet for mouse-hover.
        ///  @param   design Current code editor design
        ///  @returns the style-sheet for mouse-hovers.
        ///
        static QString hover(const QCodeEditorDesign &design);

        ///
        ///  @fn      press
        ///  @brief   Retrieves the style-sheet for mouse-press.
        ///  @param   design Current code editor design
        ///  @returns the style-sheet for mouse-presses.
        ///
        static QString press(const QCodeEditorDesign &design);
    };
}


#endif  // __KGL_QCODEEDITORSHEETS_HPP__
