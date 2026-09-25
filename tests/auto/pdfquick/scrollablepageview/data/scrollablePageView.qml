import QtQuick
import QtQuick.Pdf

PdfScrollablePageView {
    width: 480; height: 480
    property alias source: document.source
    document: PdfDocument { id: document }
}
