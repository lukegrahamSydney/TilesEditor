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
#include <KGL/Widgets/QCodeEditor.hpp>
#include <KGL/Widgets/QCodeEditorLineWidget.hpp>
#include <QCompleter>


namespace kgl {

    ///
    ///  @fn        updateLineColumn
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::updateLineColumn(int lineCount) {
        Q_UNUSED(lineCount);
        setViewportMargins(lineColumnWidth(), 0, 0, 0);
    }

    ///
    ///  @fn        scrollLineColumn
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::scrollLineColumn(QRect view, int scroll) {
        // Scrolls the line widget to the current scrollbar value
        if (m_Design.isLineColumnVisible()) {
            if (scroll != 0) {
                m_LineWidget->scroll(0, scroll);
            } else {
                m_LineWidget->update(0, view.y(), m_LineWidget->width(), view.height());
            }

            // Changes margins, line number digits may have changed
            setViewportMargins(lineColumnWidth(), 0, 0, 0);
        } else {
            setViewportMargins(0, 0, 0, 0);
        }
    }

    ///
    ///  @fn        completeWord
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::completeWord(const QString &word) {
        QTextCursor caretPos = textCursor();

        // Inserts the missing characters at the current pos
        caretPos.movePosition(QTextCursor::Left);
        caretPos.movePosition(QTextCursor::StartOfWord);
        caretPos.select(QTextCursor::WordUnderCursor);
        caretPos.removeSelectedText();
        caretPos.insertText(word);
        setTextCursor(caretPos);
    }

    ///
    ///  @fn        textChanged
    ///  @author    Nicolas Kogler
    ///  @date      October 20th, 2016
    ///
    void QCodeEditor::textChanged() {
        emit lineChanged(textCursor().block());
    }
}
