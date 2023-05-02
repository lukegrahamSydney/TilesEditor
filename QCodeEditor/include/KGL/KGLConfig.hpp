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


#ifndef __KGL_KGLCONFIG_HPP__
#define __KGL_KGLCONFIG_HPP__


//
//  Included headers
//
#include <QtCore>   ///< Provides Q_CC macroes


namespace kgl {

    ///
    ///  @def       KGL_API
    ///  @brief     Exports or imports dynamic symbols.
    ///
    ///  This macro will default to nothing in case KGL_STATIC is
    ///  defined. Note that it will be then impossible to
    ///  dynamically link to this library and the usage of the
    ///  General Public License 3+ will be enforced.
    ///
    #ifndef KGL_STATIC
    #   ifdef Q_CC_MSVC
    #       ifdef KGL_BUILD
    #           define KGL_API __declspec(dllexport)
    #       else
    #           define KGL_API __declspec(dllimport)
    #       endif
    #   else
    #       if __GNUC__ >= 4
    #           define KGL_API __attribute__((__visibility__("default")))
    #       else
    #           define KGL_API  ///< No visibility attribute
    #       endif
    #   endif
    #else
    #   define KGL_API
    #endif
}


#endif  // __KGL_KGLCONFIG_HPP__
