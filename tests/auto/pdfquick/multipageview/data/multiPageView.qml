import QtQuick
import QtQuick.Pdf

PdfMultiPageView {
    width: 480; height: 480
    property alias source: document.source
    document: PdfDocument { id: document }
}
