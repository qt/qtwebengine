import QtQuick
import QtQuick.Pdf

PdfMultiPageView {
    id: view
    property point hoverPos: hover.point.position
    width: 640; height: 480
    document: PdfDocument { }

    // mouse hover feedback for test development
    Rectangle {
        width: 200
        height: hoverPosLabel.implicitHeight + 12
        color: "beige"
        Text { id: hoverPosLabel; x: 6; y: 6; text: view.hoverPos.x + ", " + view.hoverPos.y }
    }
    HoverHandler { id: hover }
}
