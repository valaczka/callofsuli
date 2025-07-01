import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QItemGradient {
    id: root

    property ActionRpgMultiplayerGame game: null

    property bool _isFirst: true

    title: game ? game.name + qsTr(" – level %1").arg(game.level): ""

    Qaterial.BusyIndicator {
        id: _busyIndicator
        anchors.centerIn: parent
        visible: false
    }

    Item {
        id: _content

        property real horizontalPadding: Qaterial.Style.horizontalPadding
        property real verticalPadding: Qaterial.Style.horizontalPadding

        anchors.leftMargin: Math.max(horizontalPadding, Client.safeMarginLeft)
        anchors.rightMargin: Math.max(horizontalPadding, Client.safeMarginRight)
        anchors.topMargin: Math.max(verticalPadding, Client.safeMarginTop, root.paddingTop)
        anchors.bottomMargin: Math.max(verticalPadding, Client.safeMarginBottom)

        anchors.fill: parent

        Qaterial.Card {
            outlined: true

            width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
            height: Math.min(parent.height, 500)

            anchors.centerIn: parent

            contentItem: QListView {
                id: _view

                anchors.fill: parent
                anchors.leftMargin: Qaterial.Style.card.horizontalPadding
                anchors.rightMargin: Qaterial.Style.card.horizontalPadding
                anchors.topMargin: Qaterial.Style.card.verticalPadding
                anchors.bottomMargin: Qaterial.Style.card.verticalPadding

                model: game ? game.enginesModel : null

                delegate: Qaterial.ItemDelegate {
                    width: ListView.view.width

                    text: id + " " + owner.username + " - " + players.length

                    icon.source: Qaterial.Icons.account

                    onClicked: {
                        game.connectToEngine(id)
                    }
                }

                footer: Qaterial.ItemDelegate {
                    width: ListView.view.width
                    height: visible ? implicitHeight : 0
                    visible: game && game.canAddEngine
                    textColor: Qaterial.Colors.green500
                    iconColor: textColor
                    icon.source: Qaterial.Icons.plus
                    text: qsTr("Új játék létrehozása")

                    onClicked: game.connectToEngine(0)
                }
            }
        }
    }
}
