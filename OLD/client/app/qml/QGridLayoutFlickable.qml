import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"
import "JScript.js" as JS


Item {
    id: item

    implicitHeight: 400
    implicitWidth: 800

    property alias flickable: flick
    property alias columns: layout.columns
    default property alias _data: layout.data

    property alias watchModification: layout.watchModification
    property alias modified: layout.modified
    property bool acceptable: true

    property bool isFullscreen: false
    property real panelPaddingLeft: isFullscreen ? Math.max(CosStyle.panelPaddingLeft, mainWindow.safeMarginLeft) : CosStyle.panelPaddingLeft
    property real panelPaddingRight: isFullscreen ? Math.max(CosStyle.panelPaddingRight, mainWindow.safeMarginRight) : CosStyle.panelPaddingRight

    property bool verticalCentered: false

    signal accepted()

    width: parent.width
    height: Math.min(parent.height, flick.contentHeight)
    anchors.verticalCenter: verticalCentered ? parent.verticalCenter : undefined

    Flickable {
        id: flick

        anchors.fill: parent

        clip: true

        contentWidth: layout.width
        contentHeight: layout.height

        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        GridLayout {
            id: layout

            property bool watchModification: false
            property bool modified: false

            width: flick.width-item.panelPaddingLeft-item.panelPaddingRight
            x: item.panelPaddingLeft

            columns: item.width < item.implicitWidth ? 1 : 2
            columnSpacing: 5
            rowSpacing: columns > 1 ? 5 : 0

            function accept() {
                item.accepted()
            }
        }
    }
}

