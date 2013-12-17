// FIXME: prompt missing in Qt Quick Dialogs atm. Make our own for now.
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0
import QtQuick 2.0

ApplicationWindow {
    signal input(string text);
    signal accepted;
    signal rejected;
    property alias text: message.text;
    property alias prompt: field.text;

    width: 350
    height: 100
    flags: Qt.Dialog

    function open() {
        show();
    }

    ColumnLayout {
        anchors.fill: parent;
        anchors.margins: 4;
        Text {
            id: message;
            Layout.fillWidth: true;
        }
        TextField {
            id:field;
            Layout.fillWidth: true;
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8;
            Button {
                text: "OK"
                onClicked: {
                    input(field.text)
                    accepted();
                    close();
                    destroy();
                }
            }
            Button {
                text: "Cancel"
                onClicked: {
                    rejected();
                    close();
                    destroy();
                }
            }
        }
    }

}
