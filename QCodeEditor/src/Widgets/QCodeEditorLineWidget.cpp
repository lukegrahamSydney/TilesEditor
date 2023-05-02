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
#include <KGL/Widgets/QCodeEditorLineWidget.hpp>
#include <QTextBlock>
#include <QPainter>


namespace kgl {

    ///
    ///  @fn        Constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QCodeEditorLineWidget::QCodeEditorLineWidget(QCodeEditor *parent)
        : QWidget(parent),
          m_Parent(parent),
          m_EditorView(parent->viewport()) {
    }

    ///
    ///  @fn        Destructor
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QCodeEditorLineWidget::~QCodeEditorLineWidget() {
    }


    ///
    ///  @fn        sizeHint
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QSize QCodeEditorLineWidget::sizeHint() const {
        if (m_Parent->design().isLineColumnVisible()) {
            return QSize(m_Parent->lineColumnWidth(), 0);
        } else {
            return QSize(0, 0); // "Hide" line column
        }
    }

    ///
    ///  @fn        paintEvent
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    void QCodeEditorLineWidget::paintEvent(QPaintEvent *event) {

        if (!m_Parent->design().isLineColumnVisible()) {
            return;
        }

        // Retrieves the content rectangle and creates the QPainter
        const QCodeEditorDesign &design = m_Parent->design();
        const QRect &content = event->rect();
        QPainter painter(this);

        // Paints the background and the separator
        painter.fillRect(content, design.lineColumnBackColor());
        painter.setPen(design.lineColumnSeparatorColor());
        painter.drawLine(content.width()-1, 0, content.width()-1, height()-1);
        painter.setPen(design.lineColumnTextColor());

        // Retrieves the visible line numbers
        QTextBlock firstLine = m_Parent->firstVisibleBlock();
        qint32 bnActive = m_Parent->textCursor().blockNumber() + ((design.startsWithOne()) ? 1 : 0);
        qint32 blockNumber = firstLine.blockNumber() + ((design.startsWithOne()) ? 1 : 0);
        qint32 blockTop = static_cast<int>(m_Parent->blockBoundingGeometry(firstLine)
                                            .translated(m_Parent->contentOffset())
                                            .top());
        qint32 blockBottom = static_cast<int>(m_Parent->blockBoundingRect(firstLine)
                                            .height()) + blockTop;


        // Paints all visible line numbers
        while (firstLine.isValid() && blockTop <= content.bottom()) {
            if (firstLine.isVisible() && blockBottom >= content.top()) {
                if (blockNumber == bnActive) {
                    painter.setPen(design.activeLineColor());
                    painter.drawText(0, blockTop, width(), fontMetrics().height(),
                        Qt::AlignCenter, QString::number(blockNumber));
                    painter.setPen(design.lineColumnTextColor());
                } else {
                    painter.drawText(0, blockTop, width(), fontMetrics().height(),
                        Qt::AlignCenter, QString::number(blockNumber));
                }
            }

            // Jumps to the next line
            blockNumber++;
            firstLine = firstLine.next();
            blockTop = blockBottom;
            blockBottom += static_cast<int>(m_Parent->blockBoundingRect(firstLine).
                                            height());
        }
    }
}
