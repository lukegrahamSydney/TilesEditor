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
#include <KGL/Design/QCodeEditorSheets.hpp>


namespace kgl {


    ///
    ///  @fn        border
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QString QCodeEditorSheets::border(const QCodeEditorDesign &design) {
        QString sheet(CSS_Editor_Widget);
        const QMargins &border = design.editorBorder();

        // Replaces each variable with the respective design property
        sheet.replace("%t", QString::number(border.top()));
        sheet.replace("%r", QString::number(border.right()));
        sheet.replace("%b", QString::number(border.bottom()));
        sheet.replace("%l", QString::number(border.left()));
        sheet.replace("%c", QString::number(design.editorBorderColor().rgba(), 16));

        // Returns the modified sheet; can be applied immediately
        return sheet;
    }

    ///
    ///  @fn        hover
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QString QCodeEditorPopupSheets::hover(const QCodeEditorDesign &design) {
        QString sheet(CSS_Popup_Widget);

        // Replaces each variable with the respective design property
        sheet.replace("%border", QString::number(design.intelliBoxBorderColor().rgba(), 16));
        sheet.replace("%back", QString::number(design.intelliBoxBackColor().rgba(), 16));
        sheet.replace("%text", QString::number(design.intelliBoxTextColor().rgba(), 16));
        sheet.replace("%focus", design.hasFocusRect() ? "dotted" : "none");
        sheet.replace("%selbrd", QString::number(design.intelliBoxSelectionBorderColor().rgba(), 16));
        sheet.replace("%selback", QString::number(design.intelliBoxSelectionBackColor().rgba(), 16));
        sheet.replace("%t", QString::number(design.intelliBoxBorder().top()));
        sheet.replace("%r", QString::number(design.intelliBoxBorder().right()));
        sheet.replace("%b", QString::number(design.intelliBoxBorder().bottom()));
        sheet.replace("%l", QString::number(design.intelliBoxBorder().left()));

        // Returns the modified sheet; can be applied immediately
        return sheet;
    }

    ///
    ///  @fn        press
    ///  @author    Nicolas Kogler
    ///  @date      October 6th, 2016
    ///
    QString QCodeEditorPopupSheets::press(const QCodeEditorDesign &design) {
        QString sheet(CSS_Popup_Widget);

        // Replaces each variable with the respective design property
        sheet.replace("%border", QString::number(design.intelliBoxBorderColor().rgba(), 16));
        sheet.replace("%back", QString::number(design.intelliBoxBackColor().rgba(), 16));
        sheet.replace("%text", QString::number(design.intelliBoxTextColor().rgba(), 16));
        sheet.replace("%focus", design.hasFocusRect() ? "dotted" : "none");
        sheet.replace("%selbrd", QString::number(design.intelliBoxPressBorderColor().rgba(), 16));
        sheet.replace("%selback", QString::number(design.intelliBoxPressBackColor().rgba(), 16));
        sheet.replace("%t", QString::number(design.intelliBoxBorder().top()));
        sheet.replace("%r", QString::number(design.intelliBoxBorder().right()));
        sheet.replace("%b", QString::number(design.intelliBoxBorder().bottom()));
        sheet.replace("%l", QString::number(design.intelliBoxBorder().left()));

        // Returns the modified sheet; can be applied immediately
        return sheet;
    }
}
