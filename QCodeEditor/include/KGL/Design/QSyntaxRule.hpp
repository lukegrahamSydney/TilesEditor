//
//  QCodeEditor - Widget to highlight and auto-complete code.
//  Copyright (C) 2016 Nicolas Kogler (kogler.cml@hotmail.com)
//
//  This file is part of QCodeEditor.
//
//  QCodeEditor is free software: you can redistribute it and/or modify
//  it under the terms of the GNU (Lesser) General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU (Lesser) General Public License for more details.
//
//  You should have received a copy of the GNU (Lesser) General Public License
//  along with QCodeEditor.  If not, see <http://www.gnu.org/licenses/>.
//
//


#ifndef __KGL_QSYNTAXRULE_HPP__
#define __KGL_QSYNTAXRULE_HPP__


//
//  Included headers
//
#include <KGL/KGLConfig.hpp>
#include <KGL/Design/QCodeEditorDesign.hpp>
#include <QColor>


namespace kgl {

    ///
    ///  @file      QSyntaxRule.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///  @class     QSyntaxRule
    ///  @brief     Specifies the appearance of keywords or regular expressions.
    ///
    ///  Under the hood, each regular expression is forwarded to a function
    ///  that searches for matches between the rule and the code editor's text.
    ///  If there are matches, they are coloured and transformed as specified.
    ///  Keywords will be directly converted to a regular expression.
    ///
    class KGL_API QSyntaxRule {
    public:

        ///
        ///  @fn    Default constructor
        ///  @brief Initializes a new instance of QSyntaxRule.
        ///
        QSyntaxRule();

        ///
        ///  @fn    Copy constructor
        ///  @brief Copies one QSyntaxRule to another.
        ///  @param rule Other rule
        ///
        QSyntaxRule(const QSyntaxRule &rule);

        ///
        ///  @fn     Destructor
        ///  @brief  Frees all resources allocated by QSyntaxRule.
        ///
        ~QSyntaxRule();


        ///
        ///  @fn      regex : const
        ///  @brief   Retrieves the underlying regular expression.
        ///
        ///  If keywords were specified instead of a regex,
        ///  this will be returned: "(keyword1|keyword2|...)"
        ///
        ///  @returns a ref to the regex string being styled.
        ///
        const QString &regex() const;

        ///
        ///  @fn      font : const
        ///  @brief   Retrieves the font of the matching text.
        ///  @returns the font in which matches are rendered.
        ///
        const QFont &font() const;

        ///
        ///  @fn      backColor : const
        ///  @brief   Retrieves the background color of the matching text.
        ///  @returns the background color.
        ///
        const QColor &backColor() const;

        ///
        ///  @fn      foreColor : const
        ///  @brief   Retrieves the foreground color of the matching text.
        ///  @returns the foreground (text) color.
        ///
        const QColor &foreColor() const;

        ///
        ///  @fn      id : const
        ///  @brief   Retrieves the unique id for this rule.
        ///  @returns the rule string identifier.
        ///
        const QString &id() const;

        ///
        ///  @fn      startRegex : const
        ///  @brief   Retrieves the starting regex for this multiline rule.
        ///  @returns the staring regex.
        ///
        const QString &startRegex() const;

        ///
        ///  @fn      closeRegex : const
        ///  @brief   Retrieves the closing regex for this multiline rule.
        ///  @returns the staring regex.
        ///
        const QString &closeRegex() const;

        ///
        ///  @fn      isGlobal : const
        ///  @brief   Determines whether the regex search should be global.
        ///  @returns true to search globally.
        ///
        bool isGlobal() const;

        ///
        ///  @fn      useFont : const
        ///  @brief   Determines whether a custom font is used.
        ///  @returns true if a custom font is used.
        ///
        bool useFont() const;


        ///
        ///  @fn    setRegex
        ///  @brief Specifies the regular expression for this rule.
        ///  @param regex Escaped regex sequence
        ///
        void setRegex(const QString &regex);

        ///
        ///  @fn    setKeywords
        ///  @brief Specifies the keywords.
        ///
        ///  The keywords will be automatically translated
        ///  into a regular expression string. A usage example
        ///  for 'lineBegin=true' could be preprocessor directives.
        ///
        ///  @param keywords List of keyword strings
        ///  @param lineBegin Should keywords only be at the beginning (also with whitespace) of a line?
        ///
        void setKeywords(const QStringList &keywords, bool lineBegin = false);

        ///
        ///  @fn    setFont
        ///  @brief Specifies the font in which matches are rendered.
        ///
        ///  This class will NOT take ownership of the font
        ///  and will NOT delete it. Must be managed by the user.
        ///  If the font is NULL, the matches will be rendered
        ///  with the font of the parental QCodeEditor.
        ///
        ///  @param font Pointer to font instance
        ///
        void setFont(const QFont &font);

        ///
        ///  @fn    setBackColor
        ///  @brief Specifies the background-color of the matches.
        ///  @param backColor Reference to QColor instance
        ///
        void setBackColor(const QColor &backColor);

        ///
        ///  @fn    setForeColor
        ///  @brief Specifies the foreground-color of the matches.
        ///  @param foreColor Reference to QColor instance
        ///
        void setForeColor(const QColor &foreColor);

        ///
        ///  @fn    setId
        ///  @brief Specifies the identifier for this rule.
        ///  @param id String identifier for this rule
        ///
        void setId(const QString &id);

        ///
        ///  @fn    setStartRegex
        ///  @brief Specifies the starting regular expression for this rule.
        ///  @param regex Escaped regex sequence
        ///
        void setStartRegex(const QString &regex);

        ///
        ///  @fn    setCloseRegex
        ///  @brief Specifies the closing regular expression for this rule.
        ///  @param regex Escaped regex sequence
        ///
        void setCloseRegex(const QString &regex);

        ///
        ///  @fn    setGlobal
        ///  @brief Specifies whether the regex search should be global.
        ///  @param global True to perform a global regex search
        ///
        void setGlobal(bool global);


    private:

        //
        // Private class members
        //
        QString m_Regex;    ///< Underlying regular expr
        QFont m_Font;       ///< Font to render matches in
        QColor m_BackColor; ///< Background color of the matches
        QColor m_ForeColor; ///< Foreground color of the matches
        QString m_Id;       ///< The rule name. NULL by default
        QString m_StartReg; ///< Starting regex for multiline matches
        QString m_EndReg;   ///< Closing regex for multiline matches
        bool m_IsGlobal;    ///< Is regex search global?
        bool m_UseFont;     ///< Use the font or not?

    };


    ///
    ///  @file      QSyntaxRule.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 7th, 2016
    ///  @class     QSyntaxRules
    ///  @brief     Wraps a static loader function.
    ///
    class KGL_API QSyntaxRules {
    public:

        ///
        ///  @fn    loadFromFile
        ///  @brief Loads a list of rules from a XML file.
        ///  @param path Absolute path or resource path
        ///  @param design Current editor design (need the default font)
        ///
        static QList<QSyntaxRule> loadFromFile(const char *path, const QCodeEditorDesign &design);
    };
}


#endif  // __KGL_QSYNTAXRULE_HPP__
