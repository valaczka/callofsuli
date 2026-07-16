import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


ColumnLayout {
    id: root

    required property RpgChangerImpl changer
    required property string type

    property var model: []
    property int modelIdx: -1

    property bool replaceMode: false

    readonly property color mainColor: type == "weapon" ?
                                           Qaterial.Colors.red400 :
                                           type == "defender" ?
                                               Qaterial.Colors.green400 :
                                               type == "utility" ?
                                                   Qaterial.Colors.pink400 :
                                                   Qaterial.Colors.white
    implicitWidth: 250


    Tumbler {
        id: _tumbler

        visible: replaceMode

        model: root.model

        Layout.fillHeight: true
        Layout.fillWidth: true

        currentIndex: modelIdx

        delegate: Qaterial.IconLabel {
            icon.source: modelData.icon
            text: modelData.description

            font: Qaterial.Style.textTheme.body1
            icon.width: 32 * Qaterial.Style.pixelSizeRatio
            icon.height: 32 * Qaterial.Style.pixelSizeRatio

            color: root.mainColor

            opacity: 1.0 - Math.abs(Tumbler.displacement) / (_tumbler.visibleItemCount / 2)
        }

        Rectangle {
            anchors.horizontalCenter: _tumbler.horizontalCenter
            y: _tumbler.height * 0.4
            width: _tumbler.width * 0.9
            height: 1
            color: root.mainColor
        }

        Rectangle {
            anchors.horizontalCenter: _tumbler.horizontalCenter
            y: _tumbler.height * 0.6
            width: _tumbler.width * 0.9
            height: 1
            color: root.mainColor
        }
    }


    Qaterial.IconLabel {
        id: _content

        visible: !replaceMode

        display: IconLabel.Display.TextUnderIcon

        font: Qaterial.Style.textTheme.body1
        icon.width: 32 * Qaterial.Style.pixelSizeRatio
        icon.height: 32 * Qaterial.Style.pixelSizeRatio
        icon.source: modelIdx >= 0 ? model[modelIdx].icon : ""

        text: modelIdx >= 0 ? model[modelIdx].description : ""


        Layout.fillHeight: true
        Layout.fillWidth: true

        color: mainColor
    }



    QButton {
        id: _btnChange

        visible: !replaceMode

        highlightedBgColor: root.mainColor
        highlightedTextColor: Qaterial.Colors.black
        highlighted: enabled


        readonly property bool hasItem: changer.player ?
                                            (type == "defender" ? changer.player.hasDefender :
                                                                  type == "utility" ? changer.player.hasUtility :
                                                                                      false) :
                                            false

        enabled: changer.player && modelIdx >= 0 &&
                 changer.player.mp >= model[modelIdx].cost &&
                 !hasItem

        icon.source: enabled ? Qaterial.Icons.shimmer : Qaterial.Icons.lock
        text: modelIdx >= 0 ? qsTr("%1 MP").arg(model[modelIdx].cost) : "---"

        leftPadding: 18 * Qaterial.Style.pixelSizeRatio
        rightPadding: 18 * Qaterial.Style.pixelSizeRatio
        topPadding: 20 * Qaterial.Style.pixelSizeRatio
        bottomPadding: 20 * Qaterial.Style.pixelSizeRatio

        onClicked: type == "defender" ?
                       changer.useDefender() :
                       type == "utility" ?
                           changer.useUtility() :
                           type == "weapon" ?
                               changer.useWeapon() :
                               console.warn("Invalid type", type)

        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    }


    QButton {
        id: _btnReplace

        enabled: changer.replaceEnabled  && type != "weapon"

        visible: !replaceMode

        opacity: type == "weapon" ? 0.0 : 1.0

        icon.source: Qaterial.Icons.refresh
        text: qsTr("Csere")

        outlined: false
        flat: true

        textColor: Qaterial.Style.iconColor()

        onClicked: replaceMode = true

        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    }


    QButton {
        id: _btnReplaceOk

        visible: replaceMode

        icon.source: Qaterial.Icons.checkBold
        text: qsTr("Csere")

        bgColor: Qaterial.Colors.green600
        textColor: Qaterial.Colors.white

        onClicked: {
            modelIdx = _tumbler.currentIndex

            replaceMode = false
        }

        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    }



    function reset() {
        replaceMode = false

        if (type == "weapon") {
            modelIdx = model.length-1
            return
        }

        let t = -1

        if (type == "defender")
            t = changer.currentDefender()
        else if (type == "utility")
            t = changer.currentUtility()
        else {
            modelIdx = -1
            console.error("Invalid type", type)
            return
        }

        for (let idx=0; idx<model.length; ++idx) {
            if (model[idx].key === t) {
                modelIdx = idx
                return
            }
        }

        modelIdx = -1
    }


    function reload() {
        if (type == "defender")
            model = changer.availableDefenders
        else if (type == "utility")
            model = changer.availableUtilites
        else if (type == "weapon")
            model = [ changer.availableWeapon() ]
        else {
            console.error("Invalid type", type)
            return
        }

        reset()
    }


    Connections {
        target: changer

        function onPlayerReloaded() {
            reload()
        }
    }

    //changer.onPlayerReloaded: reload()

    Component.onCompleted: reload()
}
