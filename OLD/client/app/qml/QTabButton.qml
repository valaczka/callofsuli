import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import "Style"
import "JScript.js" as JS

TabButton {
    id: control

    font.weight: Font.DemiBold
    font.pixelSize: CosStyle.pixelSize*0.9
    font.capitalization: Font.AllUppercase

    display: control.width>150 && text.length < 20 ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly

    property color iconColor: CosStyle.colorPrimaryLighter

    Material.foreground: iconColor
}