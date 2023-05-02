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
#include <KGL/Widgets/QCodeEditorPopup.hpp>
#include <KGL/Widgets/QCodeEditorLineWidget.hpp>
#include <KGL/Widgets/QCodeEditorHighlighter.hpp>
#include <KGL/Design/QCodeEditorSheets.hpp>
#include <QCompleter>


namespace kgl {

    ///
    ///  @fn        Default constructor
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    QCodeEditor::QCodeEditor(QWidget *parent)
        : QPlainTextEdit(parent),
          m_Popup(NULL),
          m_Highlighter(NULL),
          m_SourceModel(NULL),
          m_RuleFilter(NULL),
          m_AutoComplete(NULL),
          m_CompletionTrigger(3) {

        // Changes miscellaneous properties
        setAutoFillBackground(true);
        setFrameStyle(QFrame::NoFrame);

        // Specifies the default font for the code editor
        QFont editorFont("Monospace");
        editorFont.setPointSize(10);
        editorFont.setKerning(true);
        editorFont.setStyleHint(QFont::TypeWriter);
        editorFont.setStyleStrategy(QFont::PreferAntialias);
        setFont(editorFont);


        // Constructs the highlighter and the intelliBox
        m_Highlighter = new QCodeEditorHighlighter(this);
        m_Popup = new QCodeEditorPopup(this);

        // Constructs the intelliBox's completer
        m_AutoComplete = new QCompleter;
        m_AutoComplete->setWidget(this);
        m_AutoComplete->setCompletionMode(QCompleter::PopupCompletion);
        m_AutoComplete->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
        m_AutoComplete->setPopup(m_Popup);

        // Constructs the source model
        m_SourceModel = new QStandardItemModel;

        // Constructs the sorting proxy model
        m_RuleFilter = new QSortFilterProxyModel;
        m_RuleFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_RuleFilter->setDynamicSortFilter(false);


        // Constructs the line widget
        m_LineWidget = new QCodeEditorLineWidget(this);
        updateLineColumn(0);


        // Connects signals with slots
        connect(m_AutoComplete, SIGNAL(activated(QString)), this, SLOT(completeWord(QString)));
        connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(scrollLineColumn(QRect,int)));
        connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineColumn(int)));
        connect(this, SIGNAL(textChanged()), this, SLOT(textChanged()));
    }

    ///
    ///  @fn        Destructor
    ///  @author    Nicolas Kogler
    ///
    QCodeEditor::~QCodeEditor() {
        delete m_Highlighter;
        delete m_LineWidget;
        delete m_RuleFilter;
        delete m_AutoComplete;
    }


    ///
    ///  @fn        rules
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QList<QSyntaxRule> &QCodeEditor::rules() const {
        return m_Rules;
    }

    ///
    ///  @fn        design
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    const QCodeEditorDesign &QCodeEditor::design() const {
        return m_Design;
    }

    ///
    ///  @fn        lineCount
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    quint32 QCodeEditor::lineCount() const {
        return static_cast<quint32>(document()->blockCount());
    }

    ///
    ///  @fn        textAtLine
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    QString QCodeEditor::textAtLine(quint32 index) const {
        return document()->findBlockByLineNumber(index).text();
    }

    ///
    ///  @fn        highlighter
    ///  @author    Nicolas Kogler
    ///  @date      October 15th, 2016
    ///
    QCodeEditorHighlighter *QCodeEditor::highlighter() const {
        return m_Highlighter;
    }


    ///
    ///  @fn        setRules
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::setRules(const QList<QSyntaxRule> &rules) {
        m_Rules = rules;
        m_Highlighter->updateFormats();
        m_Highlighter->rehighlight();
    }

    ///
    ///  @fn        setDesign
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::setDesign(const QCodeEditorDesign &design) {
        m_Design = design;
        setFont(design.editorFont());

        // Modifies the palette of the underlying QPlainTextEdit
        QPalette palette;
        palette.setColor(QPalette::Base, design.editorBackColor());
        palette.setColor(QPalette::Text, design.editorTextColor());
        setPalette(palette);

        // Modifies the border through a style-sheet
        setStyleSheet(QCodeEditorSheets::border(design));
        m_Highlighter->updateFormats();
    }

    ///
    ///  @fn        setDesign
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    void QCodeEditor::setCompletionTrigger(qint32 amount) {
        m_CompletionTrigger = amount;
    }

    ///
    ///  @fn        setKeywords
    ///  @author    Nicolas Kogler
    ///  @date      October 9th, 2016
    ///
    void QCodeEditor::setKeywords(const QStringList &keywords) {
        qint32 size = keywords.size();

        // Constructs the model from the keywords
        for (int i = 0; i < size; ++i) {
            QStandardItem *item = new QStandardItem;
            item->setText(keywords.at(i));
            m_SourceModel->appendRow(item);
        }

        // Changes the auto complete menu model
        m_RuleFilter->setSourceModel(m_SourceModel);
        m_RuleFilter->sort(0);
        m_AutoComplete->setModel(m_RuleFilter);
    }

    ///
    ///  @fn        setKeywordModel
    ///  @author    Nicolas Kogler
    ///  @date      October 9th, 2016
    ///
    void QCodeEditor::setKeywordModel(QStandardItemModel *model) {
        m_RuleFilter->setSourceModel(model);
        m_AutoComplete->setModel(m_RuleFilter);
    }

    ///
    ///  @fn        addKeyword
    ///  @author    Nicolas Kogler
    ///  @date      October 19th, 2016
    ///
    void QCodeEditor::addKeyword(const QString &keyword) {
        if (!keywordExists(keyword)) {
            QStandardItem *item = new QStandardItem;
            item->setText(keyword);
            m_SourceModel->appendRow(item);
            m_RuleFilter->sort(0);
        }
    }

    ///
    ///  @fn        removeKeyword
    ///  @author    Nicolas Kogler
    ///  @date      October 19th, 2016
    ///
    void QCodeEditor::removeKeyword(const QString &keyword) {
        QList<QStandardItem *> m = m_SourceModel->findItems(keyword);
        if (m.size() == 1) {
            m_SourceModel->removeRow(m.at(0)->row());
        }
    }

    ///
    ///  @fn        keywordExists
    ///  @author    Nicolas Kogler
    ///  @date      October 19th, 2016
    ///
    bool QCodeEditor::keywordExists(const QString &keyword) {
        return !m_SourceModel->findItems(keyword).isEmpty();
    }


    ///
    ///  @fn        rehighlight
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    void QCodeEditor::rehighlight() {
        m_Highlighter->rehighlight();
    }

    ///
    ///  @fn        lineColumnWidth
    ///  @author    Nicolas Kogler
    ///  @date      October 5th, 2016
    ///
    int QCodeEditor::lineColumnWidth() const {
        // Gets the amount of lines
        int minDigits = 1;
        int maxDigits = qMax(minDigits, document()->blockCount());

        // Calculates the number of digits in the biggest line number
        while (maxDigits >= 10) {
            maxDigits /= 10;
            minDigits++;
        }

        // Retrieves the line column padding
        QLineColumnPadding pad = m_Design.lineColumnPadding();

        // Instead of going through all numbers and picking the one
        // rendering the widest, we approximate the width of it.

        return fontMetrics().averageCharWidth()
                    * minDigits
                    + pad.left()
                    + pad.right();
    }
}
