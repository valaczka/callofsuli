import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Item {
    id: _root

    property string iconSource: ""
    property string text: ""
    property color iconColor: Qaterial.Style.accentColor
    property color textColor: Qaterial.Style.primaryTextColor()
    property int iconSize: 32 * Qaterial.Style.pixelSizeRatio

    implicitWidth: 200
    implicitHeight: Math.max(_icon.implicitHeight, _text.implicitHeight, 70 * Qaterial.Style.pixelSizeRatio)

    Row {
        anchors.centerIn: parent
        spacing: Qaterial.Style.card.horizontalPadding

        Qaterial.RoundColorIcon
        {
            id: _icon

            anchors.verticalCenter: parent.verticalCenter
            highlighted: false
            fill: false
            visible: source != ""

            source: _root.iconSource
            iconSize: _root.iconSize
            color: _root.iconColor

        }

        Qaterial.LabelHeadline6
        {
            id: _text

            text: _root.text
            width: Math.min(implicitWidth,
                            _root.width - 2 * Qaterial.Style.card.horizontalPadding
                            - (_icon.visible ? _icon.width + parent.spacing : 0)
                            )
            color: _root.textColor
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            maximumLineCount: 3

            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
