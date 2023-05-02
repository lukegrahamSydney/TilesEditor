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


#ifndef __KGL_QCODEEDITORPOPUP_HPP__
#define __KGL_QCODEEDITORPOPUP_HPP__


//
//  Included headers
//
#include <KGL/KGLConfig.hpp>
#include <KGL/Widgets/QCodeEditor.hpp>
#include <QListView>


namespace kgl {

    ///
    ///  @file      QCodeEditorPopup.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///  @class     QCodeEditorPopup
    ///  @brief
    ///
    class KGL_API QCodeEditorPopup : public QListView {
    public:

        ///
        ///  @fn    Constructor
        ///  @brief Initializes a new instance of QCodeEditorPopup.
        ///
        QCodeEditorPopup(QCodeEditor *parent);

        ///
        ///  @fn    Destructor
        ///  @brief Frees all resources allocated by QCodeEditorPopup.
        ///
        ~QCodeEditorPopup();


    protected:

        ///
        /// @fn      viewportEvent
        /// @brief   Intercepts events for the QCodeEditor viewport.
        /// @param   event Event of unknown type
        /// @returns true to indicate we have handled the event ourselves.
        ///
        bool viewportEvent(QEvent *event) Q_DECL_OVERRIDE;


    private:

        //
        // Private class members
        //
        QString m_StyleSheetNormal;     ///< Holds the stylesheet on mouse-hover
        QString m_StyleSheetPress;      ///< Holds the stylesheet on mouse-press
        QCodeEditor *m_Parent;          ///< Holds the parent for design-property access
    };
}


#endif  // __KGL_QCODEEDITORPOPUP_HPP__
