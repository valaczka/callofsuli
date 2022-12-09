import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.15
import "Style"

RowLayout {
    id: control
    //property string fieldName: ""
    //property string sqlField: ""
    //property string sqlData: field.text+combo.textAt(combo.currentIndex)
    property bool modified: false

    property bool watchModification: parent.watchModification

    property alias acceptableInput: field2.acceptableInput

    property alias first: field1
    property alias second: field2
    property bool readOnly: false
    property alias separator: separator.text
    property alias canDelete: removeButton.visible

    property bool waitForImage: false

    Layout.fillWidth: true
    Layout.bottomMargin: parent.columns === 1 ? 10 : 0

    spacing: 0

    signal modifyAction()
    signal acceptAction()
    signal deleteAction()
    signal imageAddAction()

    Row {
        id: field1

        property int imageId: -1

        Layout.fillWidth: false
        Layout.minimumWidth: CosStyle.pixelSize*4
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.bottomMargin: 0
        property bool watchModification: false
        property string fieldName: ""
        property string sqlField: ""
        property var sqlData: control.text
        property bool modified: false

        spacing: 5

        Item {
            width: CosStyle.pixelSize*4.5
            height: width

            anchors.verticalCenter: parent.verticalCenter

            visible: field1.imageId != -1

            Image {
                id: img
                fillMode: Image.PreserveAspectFit
                width: parent.width-20
                height: parent.height-20
                anchors.centerIn: parent
                source: field1.imageId == -1 ? "" : "image://mapimage/%1".arg(field1.imageId)
                visible: false
                cache: false
            }

            Glow {
                anchors.fill: img
                source: img
                radius: 4
                samples: 9
                color: CosStyle.colorPrimaryLighter
                spread: 0.5
            }
        }


        QToolButton {
            color: CosStyle.colorError
            icon.source: "qrc:/internal/icon/image-remove.svg"

            anchors.verticalCenter: parent.verticalCenter

            visible: field1.imageId != -1 && !readOnly

            onClicked: {
                field1.imageId = -1
            }
        }

        QToolButton {
            color: CosStyle.colorOKLighter
            icon.source: "qrc:/internal/icon/image-plus.svg"

            anchors.verticalCenter: parent.verticalCenter

            visible: field1.imageId == -1 && !readOnly

            onClicked: {
                waitForImage = true
                imageAddAction()
            }
        }

        /*onTextEdited:  {
            if (control.watchModification) {
                control.modified = true
                control.parent.modified = true
            }
        }

        onTextModified: modifyAction()

        onAccepted: field2.forceActiveFocus()*/

        function setData(t) {
            imageId = t
            modified = false
        }
    }

    QLabel {
        id: separator
        text: "—"
        color: field2.textColor

        font.weight: Font.DemiBold

        visible: text

        leftPadding: 5
        rightPadding: 5

        Layout.fillWidth: false
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

    }


    QGridTextField {
        id: field2

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        Layout.bottomMargin: 0
        watchModification: false

        lineVisible: true
        readOnly: control.readOnly

        onTextEdited:  {
            if (control.watchModification) {
                control.modified = true
                control.parent.modified = true
            }
        }

        onTextModified: modifyAction()

        onAccepted: acceptAction()
    }

    QToolButton {
        id: removeButton
        color: CosStyle.colorError
        icon.source: "qrc:/internal/icon/delete.svg"

        visible: false

        Layout.fillWidth: false
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

        onClicked: deleteAction()
    }

    function accept() {

    }
}
