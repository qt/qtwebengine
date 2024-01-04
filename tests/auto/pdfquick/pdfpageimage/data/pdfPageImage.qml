import QtQuick
import QtQuick.Pdf

Item {
    width: 320
    height: 320

    PdfDocument {
        id: doc
        source: "bookmarksAndLinks.pdf"
    }

    PdfPageImage {
        anchors.centerIn: parent
    }
}
