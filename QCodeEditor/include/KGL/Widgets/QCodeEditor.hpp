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


#ifndef __KGL_QCODEEDITOR_HPP__
#define __KGL_QCODEEDITOR_HPP__


//
//  Included headers
//
#include <KGL/KGLConfig.hpp>
#include <KGL/Design/QSyntaxRule.hpp>
#include <KGL/Design/QCodeEditorDesign.hpp>
#include <QAbstractProxyModel>
#include <QStandardItemModel>
#include <QPlainTextEdit>
#include <QCompleter>
#include <QTextBlock>


namespace kgl {

    //
    // Forward declarations
    //
    class QCodeEditorLineWidget;
    class QCodeEditorHighlighter;
    class QCodeEditorPopup;


    ///
    ///  @file      QCodeEditor.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///  @class     QCodeEditor
    ///  @brief     Highlights and auto-completes code.
    ///
    class KGL_API QCodeEditor : public QPlainTextEdit {
    Q_OBJECT
    public:

        ///
        ///  @fn    Default constructor
        ///  @brief Initializes a new instance of QCodeEditor.
        ///
        QCodeEditor(QWidget *parent = NULL);

        ///
        ///  @fn    Destructor
        ///  @brief Frees all resources allocated by QCodeEditor.
        ///
        ~QCodeEditor();


        ///
        ///  @fn      rules : const
        ///  @brief   Retrieves all the syntax highlighting rules.
        ///  @returns a ref to a list of QSyntaxRule instances.
        ///
        const QList<QSyntaxRule> &rules() const;

        ///
        ///  @fn      design : const
        ///  @brief   Retrieves all the visual properties.
        ///  @returns a ref to a QCodeEditorDesign instance.
        ///
        const QCodeEditorDesign &design() const;

        ///
        ///  @fn      lineCount : const
        ///  @brief   Retrieves the amount of lines.
        ///  @returns the the amount of lines.
        ///
        quint32 lineCount() const;

        ///
        ///  @fn      textAtLine : const
        ///  @brief   Retrieves the text on the specified line number.
        ///  @param   index Line index starting from zero
        ///  @returns the string value at line 'index'.
        ///
        QString textAtLine(quint32 index) const;

        ///
        ///  @fn      highlighter : const
        ///  @brief   Retrieves the syntax highlighter for this editor.
        ///  @returns the sytax highlighter.
        ///
        QCodeEditorHighlighter *highlighter() const;


        ///
        ///  @fn    setRules
        ///  @brief Specifies the syntax highlighting rules.
        ///  @param rules List of QSyntaxRule instances
        ///
        void setRules(const QList<QSyntaxRule> &rules);

        ///
        ///  @fn    setDesign
        ///  @brief Specifies the visual properties.
        ///  @param design Instance of QCodeEditorDesign
        ///
        void setDesign(const QCodeEditorDesign &design);

        ///
        ///  @fn    setCompletionTrigger
        ///  @brief Sets the completion trigger.
        ///
        ///  Specifies the amount of characters to be typed in the
        ///  currently written word until the auto-completion menu
        ///  is opened (in case it is part of a registered keyword, function ...).
        ///
        ///  @param amount Amount of characters to be typed
        ///  @default The default value is 3.
        ///
        void setCompletionTrigger(qint32 amount);

        ///
        ///  @fn    setKeywords
        ///  @brief Specifies the keywords for auto-completion.
        ///
        ///  It is not possible to add icons or anything else
        ///  if only the keywords are specified. The keywords
        ///  are sorted alphabetically. If you want to add
        ///  icons or other things to the keywords, you have to
        ///  specify a custom model through setKeywordModel.
        ///
        ///  @param keywords List of keywords
        ///
        void setKeywords(const QStringList &keywords);

        ///
        ///  @fn    setKeywordModel
        ///  @brief Specifies the auto-complete model directly.
        ///  @param model QStandardItemModel to use
        ///
        void setKeywordModel(QStandardItemModel *model);

        ///
        ///  @fn    addKeyword
        ///  @brief Adds a keyword to the existing model.
        ///  @param keyword New auto-complete keyword.
        ///  @note  If you specified a custom model, append
        ///         the keyword on your own and it will be
        ///         immediately visible to the intelliBox.
        ///
        void addKeyword(const QString &keyword);

        ///
        ///  @fn    removeKeyword
        ///  @brief Removes a keyword from the existing model.
        ///  @param keyword Keyword to remove from the list
        ///
        void removeKeyword(const QString &keyword);

        ///
        ///  @fn      keywordsExist
        ///  @brief   Determines whether the given keyword exists.
        ///  @param   keyword Keyword to check
        ///  @returns true if the keyword exísts.
        ///
        bool keywordExists(const QString &keyword);

        ///
        ///  @fn    rehighlight
        ///  @brief Applies syntax highlighting manually.
        ///
        ///  Normally one would not need this, because 'setRules'
        ///  already performs rehighlighting of the QCodeEditor.
        ///
        void rehighlight();

        ///
        ///  @fn      lineColumnWidth
        ///  @brief   Retrieves the width of the line column.
        ///  @returns the width of the line column.
        ///
        int lineColumnWidth() const;


    protected:

        ///
        ///  @fn    paintEvent
        ///  @brief Paints additional widgets or plugins.
        ///  @param event Pointer to a QPaintEvent instance
        ///
        void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

        ///
        ///  @fn    keyPressEvent
        ///  @brief Intercepts the key-press event.
        ///
        ///  Under the hood, it triggers the intellisense box
        ///  in case a partial keyword or function has been
        ///  detected or if one of the trigger keys for the
        ///  intellisense box have been pressed (e.g dot or
        ///  arrow for C++ methods).
        ///
        ///  @param event Pointer to a QKeyEvent instance
        ///
        void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

        ///
        ///  @fn    keyReleaseEvent
        ///  @brief Intercepts the key-release event.
        ///
        ///  Almost all IDEs convert tabulator characters to
        ///  whitespaces (mostly 4). This ensures that the
        ///  document looks the same, regardless of the context.
        ///
        ///  @param event Pointer to a QKeyEvent instance
        ///
        void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

        ///
        ///  @fn    resizeEvent
        ///  @brief Intercepts the resize event.
        ///
        ///  Whenever the user resizes the code editor in the
        ///  vertical direction, the line-number widget should
        ///  also be resized.
        ///
        ///  @param event Pointer to a QResizeEvent instance
        ///
        void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;


    signals:

        ///
        ///  @fn    lineChanged
        ///  @brief Is emitted when a line's text changed.
        ///  @param block Block (line) that was changed
        ///
        void lineChanged(QTextBlock block);


    private slots:

        ///
        ///  @fn    updateLineColumn : slot
        ///  @brief Updates the line column width.
        ///
        ///  We need to recalculate the width of the line
        ///  column because the amount of digits per line
        ///  number changes if the user exceeds line 9, 99, ...
        ///
        ///  @param lineCount Current amount of lines
        ///
        void updateLineColumn(int lineCount);

        ///
        ///  @fn    scrollLineColumn : slot
        ///  @brief Updates the scroll value of the line widget.
        ///
        ///  The first visible line number must be updated
        ///  in case the user scrolled the code editor vertically.
        ///
        ///  @param view Visible rectangle within the editor
        ///  @param scroll Vertical scroll value
        ///
        void scrollLineColumn(QRect view, int scroll);

        ///
        ///  @fn    completeWord : slot
        ///  @brief Auto-completes the given keyword, function, ...
        ///
        ///  This slot is executed as soon as the user picked an
        ///  element from the QCodeEditorPopup list.
        ///
        ///  @param word The word to complete
        ///
        void completeWord(const QString &word);

        ///
        ///  @fn    textChanged : slot
        ///  @brief Emits a line-changed event when appropriate
        ///
        void textChanged();

    private:

        //
        // Private class members
        //
        QList<QSyntaxRule> m_Rules;             ///< Holds all syntax highlighting rules
        QCodeEditorDesign m_Design;             ///< Holds all visual properties
        QCodeEditorLineWidget *m_LineWidget;    ///< Line column widget
        QCodeEditorPopup *m_Popup;              ///< Intellisense popup widget
        QCodeEditorHighlighter *m_Highlighter;  ///< The syntax highlighter
        QStandardItemModel *m_SourceModel;      ///< Source model for the rule filter
        QSortFilterProxyModel *m_RuleFilter;    ///< Filters all the keywords, funcs, ...
        QCompleter *m_AutoComplete;             ///< Auto completion widget
        qint32 m_CompletionTrigger;             ///< Amount of characters till AC trigger


        // Allow the line widget to access vars/funcs while rendering
        friend class QCodeEditorLineWidget;
    };
}


#endif  // __KGL_QCODEEDITOR_HPP__
