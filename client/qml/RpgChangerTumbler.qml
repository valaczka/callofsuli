import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Tumbler {
    id: _tumbler

    property color mainColor: Qaterial.Style.iconColor()

    delegate: cmpDefault

    implicitWidth: 200
    implicitHeight: 200

    wrap: false

    readonly property Component cmpDefault: Rectangle {
        readonly property bool isCurrent: Tumbler.displacement === 0

        color: isCurrent ? mainColor : "transparent"

        Qaterial.IconLabel {
            anchors.fill: parent

            icon.source: model.icon
            text: description

            font: Qaterial.Style.textTheme.body1
            icon.width: 32 * Qaterial.Style.pixelSizeRatio
            icon.height: 32 * Qaterial.Style.pixelSizeRatio

            color: parent.isCurrent ? Qaterial.Colors.black : _tumbler.mainColor
        }

        opacity: 1.0 - Math.abs(Tumbler.displacement) / (_tumbler.visibleItemCount / 2)
    }

    Rectangle {
        anchors.horizontalCenter: _tumbler.horizontalCenter
        y: _tumbler.height * Math.floor(_tumbler.visibleItemCount/2)*(1.0/_tumbler.visibleItemCount)
        width: _tumbler.width * 0.9
        height: 1
        color: mainColor
        visible: _tumbler.moving
    }

    Rectangle {
        anchors.horizontalCenter: _tumbler.horizontalCenter
        y: _tumbler.height * Math.ceil(_tumbler.visibleItemCount/2)*(1.0/_tumbler.visibleItemCount)
        width: _tumbler.width * 0.9
        height: 1
        color: mainColor
        visible: _tumbler.moving
    }
}
