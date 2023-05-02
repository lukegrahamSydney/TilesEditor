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


#ifndef __KGL_QCODEEDITORLINEWIDGET_HPP__
#define __KGL_QCODEEDITORLINEWIDGET_HPP__


//
//  Included headers
//
#include <KGL/KGLConfig.hpp>
#include <KGL/Widgets/QCodeEditor.hpp>
#include <QWidget>


namespace kgl {

    ///
    ///  @file      QCodeEditorLineWidget.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///  @class     QCodeEditorLineWidget
    ///  @brief     Paints the line column and the line numbers.
    ///
    class KGL_API QCodeEditorLineWidget : public QWidget {
    public:

        ///
        ///  @fn    Constructor
        ///  @brief Initializes a new instance of QCodeEditorLineWidget.
        ///
        QCodeEditorLineWidget(QCodeEditor *parent);

        ///
        ///  @fn    Destructor
        ///  @brief Frees all resources allocated by QCodeEditorLineWidget.
        ///
        ~QCodeEditorLineWidget();


        ///
        ///  @fn      sizeHint
        ///  @brief   Reserves space for the line widget.
        ///  @returns the desired size.
        ///
        QSize sizeHint() const Q_DECL_OVERRIDE;


    protected:

        ///
        ///  @fn    paintEvent
        ///  @brief Paints the line column and the line numbers.
        ///  @param event Pointer to a QPaintEvent instance
        ///
        void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;


    private:

        //
        // Private class members
        //
        QCodeEditor *m_Parent;
        QWidget *m_EditorView;
    };
}


#endif  // __KGL_QCODEEDITORLINEWIDGET_HPP__
