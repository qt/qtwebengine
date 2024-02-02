import QtQuick
import QtQuick.Pdf

PdfMultiPageView {
    id: view
    width: 480
    height: 480

    document: PdfDocument {
        id: document
        onStatusChanged: {
            if(status === PdfDocument.Ready)
                view.goToPage(2)
        }
    }

    Component.onCompleted: document.source = "bookmarksAndLinks.pdf"
}
