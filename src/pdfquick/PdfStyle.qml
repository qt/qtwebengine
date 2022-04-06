/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

/*!
    \qmltype PdfStyle
    \inqmlmodule QtQuick.Pdf
    \brief A styling interface for the PDF viewer components.

    PdfStyle provides properties to modify the appearance of PdfMultiPageView,
    PdfScrollablePageView, and PdfPageView.

    Default styles are provided to match the
    \l {Styling Qt Quick Controls}{styles in Qt Quick Controls}.
    \l {Using File Selectors with Qt Quick Controls}{File selectors}
    are used to load the PDF style corresponding to the Controls style in use.
    Custom styles are possible, using different \l {QFileSelector}{file selectors}.
*/
QtObject {
    /*! \internal
        \qmlproperty SystemPalette PdfStyle::palette
    */
    property SystemPalette palette: SystemPalette { }

    /*! \internal
        \qmlmethod color PdfStyle::withAlpha()
    */
    function withAlpha(color, alpha) {
        return Qt.hsla(color.hslHue, color.hslSaturation, color.hslLightness, alpha)
    }

    /*!
        \qmlproperty color PdfStyle::selectionColor

        The color of translucent rectangles that are overlaid on
        \l {PdfMultiPageView::selectedText}{selected text}.

        \sa PdfSelection
    */
    property color selectionColor: withAlpha(palette.highlight, 0.5)

    /*!
        \qmlproperty color PdfStyle::pageSearchResultsColor

        The color of translucent rectangles that are overlaid on text that
        matches the \l {PdfMultiPageView::searchString}{search string}.

        \sa PdfSearchModel
    */
    property color pageSearchResultsColor: "#80B0C4DE"

    /*!
        \qmlproperty color PdfStyle::currentSearchResultStrokeColor

        The color of the box outline around the
        \l {PdfSearchModel::currentResult}{current search result}.

        \sa PdfMultiPageView::searchBack(), PdfMultiPageView::searchForward(), PdfSearchModel::currentResult
    */
    property color currentSearchResultStrokeColor: "cyan"

    /*!
        \qmlproperty real PdfStyle::currentSearchResultStrokeWidth

        The line width of the box outline around the
        \l {PdfSearchModel::currentResult}{current search result}.

        \sa PdfMultiPageView::searchBack(), PdfMultiPageView::searchForward(), PdfSearchModel::currentResult
    */
    property real currentSearchResultStrokeWidth: 2
}
