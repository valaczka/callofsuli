import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Rectangle {
    id: root

    property RpgGame game: null
    property bool _active: false

    color: Qaterial.Colors.black

    Item {
        id: _content

        property real horizontalPadding: Qaterial.Style.horizontalPadding
        property real verticalPadding: Qaterial.Style.horizontalPadding

        anchors.leftMargin: Math.max(horizontalPadding, Client.safeMarginLeft)
        anchors.rightMargin: Math.max(horizontalPadding, Client.safeMarginRight)
        anchors.topMargin: Math.max(verticalPadding, Client.safeMarginTop)
        anchors.bottomMargin: Math.max(verticalPadding, Client.safeMarginBottom)

        anchors.fill: parent

        Qaterial.Card {
            id: _card
            outlined: true


            readonly property int _count: 1 +
                                          (game && game.gameResultData.rpg.unlocked !== undefined ? 1 : 0) +
                                          (game && game.gameResultData.rpg.dropList.length > 0 ? 1 : 0)

            width: Math.min(parent.width, Qaterial.Style.maxContainerSize * _count/3)
            height: Math.min(parent.height, 450)

            anchors.centerIn: parent


            contentItem: Item {
                Column {
                    id: _grid1

                    readonly property real _cardHeight: Math.min(_card.width/_card._count-30, _card.height-100)

                    spacing: 30
                    width: parent.width
                    anchors.centerIn: parent

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter

                        spacing: 15

                        RpgSelectCard {
                            image: game ? game.getCharacterImage(game.gameResultData.rpg.character) : ""

                            borderVisible: true

                            width: _grid1._cardHeight
                            height: _grid1._cardHeight

                            anchors.verticalCenter: parent.verticalCenter

                            Qaterial.IconLabel {
                                text: game.gameResultData.rpg.newLevel
                                icon.source: Qaterial.Icons.power

                                anchors.left: parent.left
                                anchors.bottom: parent.bottom
                                anchors.leftMargin: 10 * Qaterial.Style.pixelSizeRatio
                                anchors.bottomMargin: 10 * Qaterial.Style.pixelSizeRatio

                                icon.width: 12 * Qaterial.Style.pixelSizeRatio
                                icon.height: 12 * Qaterial.Style.pixelSizeRatio
                                spacing: 0
                            }

                            Qaterial.IconLabel {
                                property int pt: _active && game ? game.gameResultData.rpg.newPoint : 0

                                Behavior on pt {
                                    NumberAnimation { duration: 450; easing.type: Easing.InOutQuad }
                                }

                                color: Qaterial.Colors.amber400
                                text: pt
                                icon.source: "qrc:/rpg/coin/coins.png"
                                icon.color: "transparent"

                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                anchors.rightMargin: 10 * Qaterial.Style.pixelSizeRatio
                                anchors.bottomMargin: 10 * Qaterial.Style.pixelSizeRatio

                                icon.width: 12 * Qaterial.Style.pixelSizeRatio
                                icon.height: 12 * Qaterial.Style.pixelSizeRatio
                                spacing: 0
                            }
                        }


                        RpgSelectCard {
                            id: _unlocked
                            visible: game && game.gameResultData.rpg.unlocked !== undefined

                            image: game && game.gameResultData.rpg.unlocked !== undefined ?
                                       game.getCharacterImage(game.gameResultData.rpg.unlocked) : ""

                            locked: true
                            borderVisible: true
                            selected: true

                            width: _grid1._cardHeight
                            height: _grid1._cardHeight

                            anchors.verticalCenter: parent.verticalCenter

                            Timer {
                                interval: 750
                                running: _active
                                onTriggered: {
                                    _unlocked.text = qsTr("UNLOCKED")
                                    _unlocked.locked = false
                                }
                            }
                        }

                        RpgSelectCard {
                            visible: game && game.gameResultData.rpg.dropList.length > 0
                            enabled: game && game.rpgUserData.drops.length > 0

                            borderVisible: true

                            onClicked: {
                                enabled = false
                                Client.stackPushPage("PageRpgDrop.qml", {
                                                         game: root.game
                                                     })
                            }


                            Qaterial.Icon {
                                icon: Qaterial.Icons.abacus
                                color: Qaterial.Colors.yellow500
                                size: 48

                                anchors.centerIn: parent
                            }

                            width: _grid1._cardHeight
                            height: _grid1._cardHeight

                            anchors.verticalCenter: parent.verticalCenter
                        }


                        /* Qaterial.LabelBody1 {
                        width: _grid1._cardHeight

                        anchors.verticalCenter: parent.verticalCenter

                        text: game ? JSON.stringify(game.gameResultData) : ""

                        wrapMode: Text.Wrap
                    }*/

                    }


                    QButton {
                        anchors.horizontalCenter: parent.horizontalCenter

                        text: qsTr("OK")

                        onClicked: Client.stackPop()
                    }
                }
            }


        }
    }


    StackView.onActivated: {
        _active = true

        console.warn(JSON.stringify(game.gameResultData))
    }

    StackView.onDeactivating: {
    }
}
