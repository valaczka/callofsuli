import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Qaterial.Pane
{
    id: _control

    anchors.bottom: parent.bottom
    anchors.bottomMargin: 2*Qaterial.Style.horizontalPadding
    anchors.horizontalCenter: parent.horizontalCenter
    width: Math.min(parent.width - 2*2*Qaterial.Style.horizontalPadding, Qaterial.Style.maxContainerSize)
    height: Qaterial.Style.snackbar.implicitHeight
    elevation: Qaterial.Style.snackbar.elevation
    radius: Qaterial.Style.snackbar.radius
    color: Qaterial.Colors.yellow200
    visible: false

    property bool alwaysHide: false

    onAlwaysHideChanged: if (!alwaysHide) close()

    Qaterial.LabelHint1 {
        id: _text
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: _buttonClose.left
        leftPadding: Qaterial.Style.horizontalPadding
        rightPadding: Qaterial.Style.horizontalPadding
        elide: Text.ElideRight
        color: Qaterial.Colors.black
        textFormat: Text.RichText
    }

    MouseArea {
        anchors.fill: parent
        onClicked: _control.close()
    }

    Qaterial.FlatButton {
        id: _buttonClose
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        icon.source: Qaterial.Icons.close
        icon.color: Qaterial.Colors.black

        onClicked: _control.close()
    }

    Timer {
        id: _timer
        interval: 7500
        running: false
        repeat: false
        triggeredOnStart: false
        onTriggered: _control.visible = false
    }

    Connections {
        target: Client.server

        function onNotificationActivated(type, id, text) {
            if (alwaysHide)
                return

            _text.text = text
            _notificationId = id
            _notificationType = type
            _control.visible = true
            _timer.restart()
        }
    }

    property int _notificationType: Server.NotificationInvalid
    property int _notificationId: -1


    function check() {
        if (Client.server)
            Client.server.checkNotification()
    }

    function close() {
        if (Client.server)
            Client.server.closeNotification(_notificationType, _notificationId)

        _control.visible = false
    }
}
