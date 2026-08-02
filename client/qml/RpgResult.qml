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
            outlined: true

            width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
            height: Math.min(parent.height, 500)

            anchors.centerIn: parent

            contentItem: GridLayout {
                id: _grid1

                columns: 2//width > height ? 3 : 2
                columnSpacing: 10
                rowSpacing: 10
                width: parent.width - 2 * Qaterial.Style.card.horizontalPadding
                height: parent.height - 2 * Qaterial.Style.card.verticalPadding


                Qaterial.LabelBody1 {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.columnSpan: 2

                    text: game ? JSON.stringify(game.gameResultData) : ""

                    wrapMode: Text.Wrap
                }


                QButton {
                    Layout.fillHeight: false
                    Layout.fillWidth: false
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.columnSpan: 2

                    text: "EXIT"

                    onClicked: Client.stackPop()
                }
            }


        }
    }


    StackView.onActivated: {

    }

    StackView.onDeactivating: {
    }
}
