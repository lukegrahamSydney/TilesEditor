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


#ifndef __KGL_QCODEEDITORDESIGN_HPP__
#define __KGL_QCODEEDITORDESIGN_HPP__


//
//  Included headers
//
#include <KGL/KGLConfig.hpp>
#include <KGL/Design/QLineColumnPadding.hpp>
#include <QColor>
#include <QFont>


namespace kgl {

    ///
    ///  @file      QCodeEditorDesign.hpp
    ///  @author    Nicolas Kogler
    ///  @date      October 4th, 2016
    ///  @class     QCodeEditorDesign
    ///  @brief     Specifies the visual appearance of the QCodeEditor.
    ///
    class KGL_API QCodeEditorDesign {
    public:

        ///
        ///  @fn    Default constructor
        ///  @brief Initializes a new instance of QCodeEditorDesign.
        ///
        QCodeEditorDesign();

        ///
        ///  @fn    Constructor
        ///  @brief Loads a code editor design from a file.
        ///  @param path Path to the XML design file
        ///
        QCodeEditorDesign(const QString &path);

        ///
        ///  @fn    Copy constructor
        ///  @brief Copies one QCodeEditorDesign to another.
        ///  @param design Other design
        ///
        QCodeEditorDesign(const QCodeEditorDesign &design);

        ///
        ///  @fn     Destructor
        ///  @brief  Frees all resources allocated by QCodeEditorDesign.
        ///
        ~QCodeEditorDesign();


        ///
        ///  @fn      editorBackColor : const
        ///  @brief   Retrieves the background color of the code editor.
        ///  @returns the background color of the editor.
        ///
        const QColor &editorBackColor() const;

        ///
        ///  @fn      editorTextColor : const
        ///  @brief   Retrieves the text color of the code editor.
        ///  @returns the text color of the editor.
        ///
        const QColor &editorTextColor() const;

        ///
        ///  @fn      editorBorderColor : const
        ///  @brief   Retrieves the border color of the code editor.
        ///  @returns the border color of the editor.
        ///
        const QColor &editorBorderColor() const;

        ///
        ///  @fn      lineColumnBackColor : const
        ///  @brief   Retrieves the bg color of the line column.
        ///  @returns the bg color of the line column.
        ///
        const QColor &lineColumnBackColor() const;

        ///
        ///  @fn      lineColumnSeparatorColor : const
        ///  @brief   Retrieves the color of the separator between column and editor.
        ///  @returns the separator color between line widget and editor.
        ///
        const QColor &lineColumnSeparatorColor() const;

        ///
        ///  @fn      lineColumnTextColor : const
        ///  @brief   Retrieves the color of the line number texts.
        ///  @returns the color in which the line numbers are rendered.
        ///
        const QColor &lineColumnTextColor() const;

        ///
        ///  @fn      activeLineColor : const
        ///  @brief   Retrieves the color of the line number text that is currently active.
        ///  @returns the active line number text.
        ///
        const QColor &activeLineColor() const;

        ///
        ///  @fn      intelliBoxBackColor : const
        ///  @brief   Retrieves the bg color of the intellisense box.
        ///  @returns the bg color of the intellisense box.
        ///
        const QColor &intelliBoxBackColor() const;

        ///
        ///  @fn      intelliBoxTextColor : const
        ///  @brief   Retrieves the text color of the intellisense box.
        ///  @returns the text color of the intellisense box.
        ///
        const QColor &intelliBoxTextColor() const;

        ///
        ///  @fn      intelliBoxBorderColor : const
        ///  @brief   Retrieves the border color of the intellisense box.
        ///  @returns the border color of the intellisense box.
        ///
        const QColor &intelliBoxBorderColor() const;

        ///
        ///  @fn      intelliBoxSelectionBackColor : const
        ///  @brief   Retrieves the selection back color of the intellisense items.
        ///  @returns the selection back color of the intellisense items.
        ///
        const QColor &intelliBoxSelectionBackColor() const;

        ///
        ///  @fn      intelliBoxSelectionBorderColor : const
        ///  @brief   Retrieves the selection border color of the intellisense items.
        ///  @returns the selection border color of the intellisense items.
        ///
        const QColor &intelliBoxSelectionBorderColor() const;

        ///
        ///  @fn      intelliBoxPressBackColor : const
        ///  @brief   Retrieves the back color of the intellisense items on mouse-press.
        ///  @returns the back color of the intellisense items on mouse-press.
        ///
        const QColor &intelliBoxPressBackColor() const;

        ///
        ///  @fn      intelliBoxPressBorderColor : const
        ///  @brief   Retrieves the border color of the intellisense items on mouse-press.
        ///  @returns the Press border color of the intellisense items on mouse-press.
        ///
        const QColor &intelliBoxPressBorderColor() const;

        ///
        ///  @fn      editorFont : const
        ///  @brief   Retrieves the font of the default editor text.
        ///  @returns a pointer to the editor font.
        ///
        const QFont &editorFont() const;

        ///
        ///  @fn      intelliBoxFont : const
        ///  @brief   Retrieves the font of the intellisense box text.
        ///  @returns a pointer to the intellisense box font.
        ///
        const QFont &intelliBoxFont() const;

        ///
        ///  @fn      editorBorder : const
        ///  @brief   Retrieves the border-width in all directions.
        ///  @returns a reference to a QMargins instance.
        ///
        const QMargins &editorBorder() const;

        ///
        ///  @fn      intelliBoxBorder : const
        ///  @brief   Retrieves the border-width in all directions.
        ///  @returns a reference to a QMargins instance.
        ///
        const QMargins &intelliBoxBorder() const;

        ///
        ///  @fn      lineColumnPadding : const
        ///  @brief   Retrieves the padding in the right- and left direction.
        ///  @returns a ref to a QLineColumnPadding instance.
        ///
        const QLineColumnPadding &lineColumnPadding() const;

        ///
        ///  @fn      popupSize : const
        ///  @brief   Retrieves the size of the auto-completion menu.
        ///  @returns a ref to a QSize instance.
        ///
        const QSize &popupSize() const;

        ///
        ///  @fn      isLineColumnVisible : const
        ///  @brief   Determines whether the line column is visible.
        ///  @returns true if the line column is visible.
        ///
        bool isLineColumnVisible() const;

        ///
        ///  @fn      hasFocusRect : const
        ///  @brief   Determines whether the intelliBox items should have a focus rectangle.
        ///  @returns true if a dotted rectangle should be drawn around a focused item.
        ///
        bool hasFocusRect() const;

        ///
        ///  @fn      startsWithOne : const
        ///  @brief   Determines whether the line numbers start with 1.
        ///  @returns true if 1 is the first line.
        ///
        bool startsWithOne() const;


        ///
        ///  @fn    setEditorBackColor
        ///  @brief Specifies the bg color of the code editor.
        ///  @param color Color to use as background
        ///
        void setEditorBackColor(const QColor &color);

        ///
        ///  @fn    setEditorTextColor
        ///  @brief Specifies the bg color of the code editor.
        ///  @param color Color to draw text in
        ///
        void setEditorTextColor(const QColor &color);

        ///
        ///  @fn    setEditorBorderColor
        ///  @brief Specifies the bg color of the code editor.
        ///  @param color Color to draw border in
        ///
        void setEditorBorderColor(const QColor &color);

        ///
        ///  @fn    setLineColumnBackColor
        ///  @brief Specifies the bg color of the line column.
        ///  @param color Color to use as background
        ///
        void setLineColumnBackColor(const QColor &color);

        ///
        ///  @fn    setLineColumnSeparatorColor
        ///  @brief Specifies the separator color of the line column.
        ///  @param color Color to draw separator in
        ///
        void setLineColumnSeparatorColor(const QColor &color);

        ///
        ///  @fn    setLineColumnTextColor
        ///  @brief Specifies the text color of the line column.
        ///  @param color Color to draw text in
        ///
        void setLineColumnTextColor(const QColor &color);

        ///
        ///  @fn    setActiveLineColor
        ///  @brief Specifies the active line color
        ///  @param color Color to draw active line text in
        ///
        void setActiveLineColor(const QColor &color);

        ///
        ///  @fn    setIntelliBoxBackColor
        ///  @brief Specifies the bg color of the intelliBox.
        ///  @param color Color to use as background
        ///
        void setIntelliBoxBackColor(const QColor &color);

        ///
        ///  @fn    setIntelliBoxTextColor
        ///  @brief Specifies the text color of the intelliBox.
        ///  @param color Color to draw text in
        ///
        void setIntelliBoxTextColor(const QColor &color);

        ///
        ///  @fn    setIntelliBoxBorderColor
        ///  @brief Specifies the border color of the intelliBox.
        ///  @param color Color to draw border in
        ///
        void setIntelliBoxBorderColor(const QColor &color);

        ///
        ///  @fn    setIntelliBoxSelectionBackColor
        ///  @brief Specifies the bg color of the intelliBox-selection.
        ///  @param color Color to use as selection background
        ///
        void setIntelliBoxSelectionBackColor(const QColor &color);

        ///
        ///  @fn    setIntelliBoxSelectionBorderColor
        ///  @brief Specifies the border color of the intelliBox-selection.
        ///  @param color Color to draw selection border in
        ///
        void setIntelliBoxSelectionBorderColor(const QColor &color);

        ///
        ///  @fn    setIntelliBoxPressBackColor
        ///  @brief Specifies the bg color of the items on mouse-press.
        ///  @param color Color to use on mouse-press
        ///
        void setIntelliBoxPressBackColor(const QColor &color);

        ///
        ///  @fn    setIntelliBoxPressBorderColor
        ///  @brief Specifies the border color of the items on mouse-press.
        ///  @param color Color to draw border in on mouse-press
        ///
        void setIntelliBoxPressBorderColor(const QColor &color);

        ///
        ///  @fn    setEditorFont
        ///  @brief Specifies the default font for editor-text.
        ///  @param font Pointer to font to use for default text
        ///
        void setEditorFont(const QFont &font);

        ///
        ///  @fn    setIntelliBoxFont
        ///  @brief Specifies the default font for intelliBox-text.
        ///  @param font Pointer to font to use for default text
        ///
        void setIntelliBoxFont(const QFont &font);

        ///
        ///  @fn    setEditorBorder
        ///  @brief Specifies the thickness of the editor border.
        ///  @param border QMargins specifying the thickness in all directions
        ///
        void setEditorBorder(const QMargins &border);

        ///
        ///  @fn    setIntelliBoxBorder
        ///  @brief Specifies the thickness of the intelliBox border.
        ///  @param border QMargins specifying the thickness in all directions
        ///
        void setIntelliBoxBorder(const QMargins &border);

        ///
        ///  @fn    setLineColumnPadding
        ///  @brief Specifies the line column padding.
        ///  @param padding Ref to instance of QLineColumnPadding
        ///
        void setLineColumnPadding(const QLineColumnPadding &padding);

        ///
        ///  @fn    setPopupSize
        ///  @brief Specifies the size of the auto-complete menu.
        ///  @param size Ref to instance of QSize
        ///
        void setPopupSize(const QSize &size);

        ///
        ///  @fn    setLineColumnVisible
        ///  @brief Determines the visibility of the line column.
        ///  @param visible True if should be drawn, otherwise false
        ///
        void setLineColumnVisible(bool visible);

        ///
        ///  @fn    showFocusRect
        ///  @brief Specifies whether to show a dotted rectangle around a focused intelliBox item.
        ///  @param show True if a dotted rectangle should be drawn.
        ///
        void showFocusRect(bool show);

        ///
        ///  @fn    setFirstLineOne
        ///  @brief Specifies whether to set the first line as one and not zero.
        ///  @param one True if line numbers should start with 1.
        ///
        void setFirstLineAsOne(bool one);


    private:

        //
        // Private class members
        //
        QColor m_EditorBackColor;
        QColor m_EditorTextColor;
        QColor m_EditorBorderColor;
        QColor m_LineColumnBackColor;
        QColor m_LineColumnTextColor;
        QColor m_LineColumnSeparatorColor;
        QColor m_ActiveLineColor;
        QColor m_IntelliBoxBackColor;
        QColor m_IntelliBoxTextColor;
        QColor m_IntelliBoxBorderColor;
        QColor m_IntelliBoxSelectionBackColor;
        QColor m_IntelliBoxSelectionBorderColor;
        QColor m_IntelliBoxPressBackColor;
        QColor m_IntelliBoxPressBorderColor;
        QFont m_EditorFont;
        QFont m_IntelliBoxFont;
        QMargins m_EditorBorder;
        QMargins m_IntelliBoxBorder;
        QLineColumnPadding m_LineColumnPadding;
        QSize m_PopupSize;
        bool m_HasLineColumn;
        bool m_ShowFocusRect;
        bool m_FirstLineOne;
    };
}


#endif  // __KGL_QCODEEDITORDESIGN_HPP__
