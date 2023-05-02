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
#include <KGL/Widgets/QCodeEditorPopup.hpp>
#include <KGL/Design/QCodeEditorSheets.hpp>


namespace kgl {

    ///
    ///  @fn        Constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QCodeEditorPopup::QCodeEditorPopup(QCodeEditor *parent)
        : QListView(parent),
          m_Parent(parent) {

        // Generates the stylesheets from the visual properties
        m_StyleSheetNormal = QCodeEditorPopupSheets::hover(parent->design());
        m_StyleSheetPress  = QCodeEditorPopupSheets::press(parent->design());

        // Changes listview properties
        setMouseTracking(true);
        setFrameShape(QFrame::NoFrame);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectItems);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setFixedWidth(parent->design().popupSize().width());
        setFixedHeight(parent->design().popupSize().height());
        setStyleSheet(m_StyleSheetNormal);

        // Applies several other properties
        setFont(parent->design().intelliBoxFont());
    }

    ///
    ///  @fn        Destructor
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QCodeEditorPopup::~QCodeEditorPopup() {
    }


    ///
    ///  @fn        viewportEvent
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    bool QCodeEditorPopup::viewportEvent(QEvent *event) {
        if (event->type() == QEvent::MouseMove) {
            // Retrieves the item at the current mouse position
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            QModelIndex index = indexAt(me->pos());

            // Selects the item at the mouse cursor position
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

        } else if (event->type() == QEvent::MouseButtonPress) {
            // This is a little hack: There is no 'pressed' pseudo class
            // to style the listview item on mouse-press, therefore we
            // dynamically change the style-sheet.
            if (!selectedIndexes().isEmpty()) {
                setStyleSheet(m_StyleSheetPress);
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            setStyleSheet(m_StyleSheetNormal);
        }

        // Processes the default event logic
        return QListView::viewportEvent(event);
    }
}
