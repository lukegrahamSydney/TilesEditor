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
#include <QAbstractItemView>
#include <QScrollBar>
#include <QCompleter>
#include <QPainter>


namespace kgl {

    ///
    ///  @fn        paintEvent
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::paintEvent(QPaintEvent *event) {
        // Paints the text edit and the line column
        QPlainTextEdit::paintEvent(event);

        // Using style-sheets as for now!
        /*// Paints a border around the widget, if requested
        if (!m_Design.editorBorder().isNull()) {
            QPainter painter(viewport());

            // Changes the pen-color of the painter
            painter.setBrush(QBrush(m_Design.editorBorderColor()));

            // Draws a line in each direction requested
            QMargins border = m_Design.editorBorder();
            if (border.top()) {
                painter.fillRect(0, 0, width(), border.top(), Qt::SolidPattern);
            } if (border.left()) {
                painter.fillRect(0, 0, border.left(), height(), Qt::SolidPattern);
            } if (border.bottom()) {
                painter.fillRect(0, height()-border.bottom(), width(), border.bottom(), Qt::SolidPattern);
            } if (border.right()) {
                painter.fillRect(width()-border.right(), 0, border.right(), height(), Qt::SolidPattern);
            }
        }*/
    }

    ///
    ///  @fn        keyReleaseEvent
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::keyReleaseEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Tab) {
            return;
        }

        QPlainTextEdit::keyReleaseEvent(event);
    }

    ///
    ///  @fn        keyPressEvent
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::keyPressEvent(QKeyEvent *event) {
        static QChar par(0x2029); // Qt uses paragraph separators
        static QString tab = QString(" ").repeated(4);

        // The code editor should not receive keys
        // while the auto complete menu is open.
        if (m_AutoComplete->popup()->isVisible()) {
            switch (event->key()) {
            case Qt::Key_Tab:
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Backtab:
                event->ignore();
                return;
            default:
                break;
            }
        }


        // Adds tabs in front of the selected block(s)
        if (event->key() == Qt::Key_Tab && !textCursor().selectedText().isEmpty()) {
            // Retrieves the amount of lines within the selected text
            QTextCursor cursor = textCursor();
            QString selected = cursor.selectedText();
            qint32 amountOfLines = selected.count(par) + 1;

            // Does not do anything if only one line is selected
            if (amountOfLines == 1) {
                return;
            }

            // Selects the start of the current line and retrieves the position
            int linePos, lineCopy;
            cursor.setPosition(cursor.selectionStart());
            cursor.movePosition(QTextCursor::StartOfLine);
            linePos = lineCopy = cursor.position();
            cursor.beginEditBlock();

            // Inserts tabs for each selected line
            for (int i = 0; i < amountOfLines; ++i) {
                cursor.setPosition(linePos);
                cursor.insertText(tab);
                cursor.movePosition(QTextCursor::Down);
                cursor.movePosition(QTextCursor::StartOfLine);
                linePos = cursor.position();
            }

            // Selects all the text
            cursor.movePosition(QTextCursor::Down);
            cursor.movePosition(QTextCursor::EndOfLine);
            cursor.setPosition(lineCopy, QTextCursor::KeepAnchor);
            cursor.endEditBlock();
            setTextCursor(cursor);
            return;
        }

        // Removes tabs in front of selected block(s)
        if (event->key() == Qt::Key_Backtab && !textCursor().selectedText().isEmpty()) {
            // Retrieves the amount of lines within the selected text
            QTextCursor cursor = textCursor();
            QString selected = cursor.selectedText();
            qint32 amountOfLines = selected.count(par) + 1;

            // Does not do anything if only one line is selected
            if (amountOfLines == 1) {
                return;
            }

            // Retrieves the start of the selection
            int start = 0, line, diff, copy;
            cursor.setPosition(cursor.selectionStart());
            cursor.movePosition(QTextCursor::StartOfLine);
            copy = cursor.position();

            if (selected.at(0).isSpace()) {
                cursor.movePosition(QTextCursor::NextWord);
                start = cursor.position();
            }

            cursor.clearSelection();
            cursor.beginEditBlock();

            // Removes a tab from each line
            for (int i = 0; i < amountOfLines; ++i) {
                cursor.setPosition(start);
                cursor.movePosition(QTextCursor::StartOfLine);
                line = cursor.position();
                cursor.setPosition(start);

                if (start == line) {
                    continue;   // nothing to remove
                }

                diff = qMin(4, start - line);
                for (int i = 0; i < diff; ++i) {
                    cursor.deletePreviousChar();
                }

                // Finds position of the first word in the next line
                cursor.movePosition(QTextCursor::Down);
                cursor.movePosition(QTextCursor::StartOfLine);
                cursor.movePosition(QTextCursor::NextWord);
                cursor.movePosition(QTextCursor::StartOfWord);
                start = cursor.position();
            }

            // Selects all the text
            cursor.movePosition(QTextCursor::Down);
            cursor.movePosition(QTextCursor::EndOfLine);
            cursor.setPosition(copy, QTextCursor::KeepAnchor);
            cursor.endEditBlock();
            setTextCursor(cursor);
            return;
        }

        // Replaces a tab with four whitespaces
        if (event->key() == Qt::Key_Tab) {
            QTextCursor cursor = textCursor();
            cursor.insertText(QString(" ").repeated(4));
            setTextCursor(cursor);
            return;
        }


        // Forwards the pressed key to the QPlainTextEdit
        QPlainTextEdit::keyPressEvent(event);

        // Retrieves the underlying word for further checks
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::WordUnderCursor);

        // Close intellisense box if the current character is a
        // whitespace and the backspace key was pressed.
        if (event->key() == Qt::Key_Backspace) {
            if (cursor.selectedText().trimmed().isEmpty()) {
                m_AutoComplete->popup()->hide();
                return;
            }
        }

        // Does absolutely nothing if a single modifier key was pressed
        if ((event->modifiers() & (Qt::ControlModifier|Qt::ShiftModifier)) &&
             event->text().isEmpty()) {
            return;
        }

        // Does absolutely nothing if:
        // the word is interrupted by an invalid character
        // the word is too short to trigger a completion
        static QString invalidChars("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
        if (event->text().isEmpty() ||
            cursor.selectedText().length() < m_CompletionTrigger ||
            invalidChars.contains(event->text().right(1))) {

            // Hides the completion menu
            m_AutoComplete->popup()->hide();
            return;
        }

        // Filters the keyword list
        QString prefix = QString("^") + cursor.selectedText();
        
        m_RuleFilter->setFilterRegularExpression(QRegularExpression(prefix, QRegularExpression::CaseInsensitiveOption));
        m_AutoComplete->popup()->setCurrentIndex(m_AutoComplete->completionModel()->index(0, 0));

        // Determines whether any match has been found
        if (m_AutoComplete->popup()->model()->rowCount() == 0) {
            m_AutoComplete->popup()->hide();
            return;
        }

        // Shows the auto-complete box, if not already
        if (!m_AutoComplete->popup()->isVisible()) {
            // Sets the position of the popup menu and shows it
            QRect rect = cursorRect();
            rect.moveTo(rect.x() + lineColumnWidth() - fontMetrics().size(0, prefix).width(), rect.y() + 4);
            rect.setWidth(m_AutoComplete->popup()->sizeHintForColumn(0) +
                          m_AutoComplete->popup()->verticalScrollBar()->sizeHint().width());

            m_AutoComplete->complete(rect);
        }
    }

    ///
    ///  @fn        resizeEvent
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::resizeEvent(QResizeEvent *event) {
        Q_UNUSED(event);

        // Makes space for the line widget
        if (m_Design.isLineColumnVisible()) {
            QRect content = contentsRect();
            setViewportMargins(lineColumnWidth(), 0, 0, 0);
            m_LineWidget->setGeometry(
                        content.left(),
                        content.top(),
                        lineColumnWidth(),
                        content.height());
        } else {
            setViewportMargins(0, 0, 0, 0);
            m_LineWidget->setGeometry(0, 0, 0, 0);
        }
        QPlainTextEdit::resizeEvent(event);
    }
}
