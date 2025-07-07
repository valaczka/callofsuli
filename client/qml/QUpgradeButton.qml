import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QDashboardButton {
    id: _root

    text: qsTr("Frissítés")
    visible: Client.updater.updateAvailable
    icon.source: Qaterial.Icons.refresh
    highlighted: false
    outlined: true
    flat: true

    textColor: Qaterial.Colors.blue400

    onClicked: {
        Client.updater.checkAvailableUpdates(true)
        enabled = false
    }

    Connections {
        target: Client.updater

        function onGitHubUpdateCheckFailed() {
            visible = false
        }

        function onGitHubUpdateAvailable(data) {
            visible = false
        }

        function onAppImageUpdateFailed(errorString) {
            visible = false
        }

        function onAppImageUpdateAvailable() {
            visible = false
        }

        function onUpdateNotAvailable() {
            visible = false
        }

    }
}
