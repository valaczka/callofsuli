import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QItemGradient {
    id: root

    property RpgGame game: null

    title: game ? game.name + qsTr(" – level %1").arg(game.level): ""

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

            contentItem: ColumnLayout {
                id: _grid1

                spacing: 10
                //width: parent.width - 2 * Qaterial.Style.card.horizontalPadding
                //height: parent.height - 2 * Qaterial.Style.card.verticalPadding


                RpgSelectTitle {
                    Layout.fillHeight: false
                    Layout.fillWidth: true

                    rightPadding: Qaterial.Style.card.horizontalPadding
                    leftPadding: Qaterial.Style.card.horizontalPadding

                    icon.source: Qaterial.Icons.accountGroup
                    text: qsTr("Rooms")
                }

                ListView {
                    id: _viewRooms

                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Layout.leftMargin: Qaterial.Style.card.horizontalPadding
                    Layout.rightMargin: Qaterial.Style.card.horizontalPadding

                    //implicitHeight: Math.max(contentHeight, 50)
                    //implicitWidth: 50

                    snapMode: ListView.SnapToItem

                    clip: true
                    model: game ? game.modelLobby : null

                    delegate: Qaterial.ItemDelegate {
                        width: ListView.view.width

                        text: readableId

                        //secondaryText: model

                        icon.source: Qaterial.Icons.accountMultiple

                        onClicked: {
                            game.connectLobby(model)
                        }
                    }
                }

                QButton {
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.leftMargin: Qaterial.Style.card.horizontalPadding
                    Layout.rightMargin: Qaterial.Style.card.horizontalPadding
                    Layout.bottomMargin: Qaterial.Style.card.verticalPadding
                    Layout.topMargin: Qaterial.Style.card.verticalPadding

                    icon.source: Qaterial.Icons.accountMultiplePlus
                    text: qsTr("Új szoba létrehozása")

                    onClicked: game.connectLobby({})
                }

            }


        }
    }


    Timer {
        interval: 1250
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: if (game) game.reloadLobby()
    }


    StackView.onActivated: {
    }

    StackView.onDeactivating: {
    }
}
