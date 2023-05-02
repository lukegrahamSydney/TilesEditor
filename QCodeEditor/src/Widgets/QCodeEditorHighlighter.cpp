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
#include <KGL/Widgets/QCodeEditorHighlighter.hpp>
#include <KGL/Widgets/QCodeEditor.hpp>
#include <QUuid>


namespace kgl {

    ///
    ///  @fn        Constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QCodeEditorHighlighter::QCodeEditorHighlighter(QCodeEditor *parent)
        : QSyntaxHighlighter(parent->document()),
          m_Rules(&parent->rules()),
          m_Design(&parent->design()),
          m_Parent(parent) {
    }

    ///
    ///  @fn        Destructor
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QCodeEditorHighlighter::~QCodeEditorHighlighter() {
    }


    ///
    ///  @fn        updateFormats
    ///  @author    Nicolas Kogler
    ///  @date      October 8th, 2016
    ///
    void QCodeEditorHighlighter::updateFormats() {

        m_Formats.clear();

        // Pre-parses the rules' style into a list of QTextCharFormat
        for (const QSyntaxRule &rule : m_Parent->rules()) {
            QTextCharFormat format;

            if (rule.useFont()) {
                format.setFont(rule.font());
            } else {
                format.setFont(m_Parent->font());
            } if (rule.foreColor().alpha() == 0) {
                format.setForeground(QBrush(m_Parent->design().editorTextColor()));
            } else {
                format.setForeground(rule.foreColor());
            }

            format.setBackground(QBrush(rule.backColor()));
            m_Formats.push_back(format);
        }
    }

    ///
    ///  @fn        setHighlighting
    ///  @author    Nicolas Kogler
    ///  @date      October 12th, 2016
    ///
    void QCodeEditorHighlighter::setHighlighting(int start, int length, const QTextCharFormat &format) {
        setFormat(start, length, format);
    }


    ///
    ///  @fn        highlightBlock
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    void QCodeEditorHighlighter::highlightBlock(const QString &text) {
        if (currentBlockUserData() != NULL) {
            auto d = static_cast<QCodeEditorBlockData *>(currentBlockUserData());
            QRegularExpression regex(d->re);
            if (!regex.match(text).hasMatch()) {
                onRemove(d);
            }
        }

        int ruleIndex = 0;
        setCurrentBlockState(-1);

        // Applies highlighting rules
        const QList<QSyntaxRule> &rules = *m_Rules;
        for (const QSyntaxRule &rule : rules) {
            const QTextCharFormat &format = m_Formats.at(ruleIndex);
            QRegularExpression regex(rule.regex());
            QRegularExpressionMatch match;

            if (rule.isGlobal()) {

                // Searches for more than one match
                QRegularExpressionMatchIterator iter = regex.globalMatch(text);

                if (iter.hasNext() && !rule.closeRegex().isEmpty() && currentBlockState() == -1) {
                    setCurrentBlockState(ruleIndex);
                }

                // Iterates through all other matches and highlights them
                while (iter.hasNext()) {
                    match = iter.next();
                    setFormat(match.capturedStart(), match.capturedLength(), format);
                    if (!rule.id().isEmpty()) {
                        if (currentBlockUserData() == NULL) {
                            setCurrentBlockUserData(new QCodeEditorBlockData(rule.regex()));
                            emit onMatch(rule, match.captured(), currentBlock());
                        } else {
                            auto d = static_cast<QCodeEditorBlockData *>(currentBlockUserData());
                            if (d->re == rule.regex()) {
                                // Already containing a match that has the exact regex:
                                // Remove it first, then add it again
                                emit onRemove(d);
                                setCurrentBlockUserData(new QCodeEditorBlockData(rule.regex()));
                                emit onMatch(rule, match.captured(), currentBlock());
                            }
                        }
                    }
                }
            } else {

                // Searches for only one match
                match = regex.match(text);

                if (match.hasMatch()) {
                    if (!rule.closeRegex().isEmpty() && currentBlockState() == -1) {
                        // Before setting the multiline trigger, checks if
                        // the closing sequence is on the same line.
                        QRegularExpression mulEnd(rule.closeRegex());
                        if (!mulEnd.match(text).hasMatch())
                            setCurrentBlockState(ruleIndex);
                    } if (!rule.id().isEmpty()) {
                        if (currentBlockUserData() == NULL) {
                            setCurrentBlockUserData(new QCodeEditorBlockData(rule.regex()));
                            emit onMatch(rule, match.captured(), currentBlock());
                        } else {
                            auto d = static_cast<QCodeEditorBlockData *>(currentBlockUserData());
                            if (d->re == rule.regex()) {
                                // Already containing a match that has the exact regex:
                                // Remove it first, then add it again
                                emit onRemove(d);
                                setCurrentBlockUserData(new QCodeEditorBlockData(rule.regex()));
                                emit onMatch(rule, match.captured(), currentBlock());
                            }
                        }
                    }

                    setFormat(match.capturedStart(), match.capturedLength(), format);
                }
            }

            ++ruleIndex;
        }

        // If there has been a multi-line-rule match in the previous
        // block, we need to check against its closing regex.
        if (previousBlockState() != -1) {
            const QSyntaxRule &rule = m_Rules->at(previousBlockState());
            const QTextCharFormat format = m_Formats.at(previousBlockState());
            QRegularExpression regex(rule.closeRegex());
            QRegularExpressionMatch match = regex.match(text);

            // If now has a match, ends the multi-line regex
            if (match.hasMatch()) {
                setFormat(match.capturedStart(), match.capturedLength(), format);
                setCurrentBlockState(-1);

                if (!rule.id().isEmpty()) {
                    if (currentBlockUserData() == NULL) {
                        setCurrentBlockUserData(new QCodeEditorBlockData(rule.regex()));
                        emit onMatch(rule, match.captured(), currentBlock());
                    } else {
                        auto d = static_cast<QCodeEditorBlockData *>(currentBlockUserData());
                        if (d->re == rule.regex()) {
                            // Already containing a match that has the exact regex:
                            // Remove it first, then add it again
                            emit onRemove(d);
                            setCurrentBlockUserData(new QCodeEditorBlockData(rule.regex()));
                            emit onMatch(rule, match.captured(), currentBlock());
                        }
                    }
                }
            } else {
                // Has no match, highlights the entire line and
                // forwards the previous state to the next line.
                setFormat(0, text.length(), format);
                setCurrentBlockState(previousBlockState());
            }
        }


        // Provides custom highlighting logic
        emit onHighlight(this);
    }


    ///
    ///  @brief Defines the global uuid list
    ///
    QList<QUuid> QCodeEditorBlockData::p;
}
