import QtQuick 2.1
import QtQuick.Controls 1.0 as Controls

Controls.Menu {
    signal done()

    // Use private API for now
    on__MenuClosed: done();
}
