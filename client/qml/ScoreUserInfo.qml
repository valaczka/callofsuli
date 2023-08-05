import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QPageGradient {
	id: root

	progressBarEnabled: true

	property var userData: null

	property bool _firstRun: true

	QScrollable {
		anchors.fill: parent
		spacing: 15
		refreshEnabled: true

		contentCentered: true

		onRefreshRequest: reload()

		UserInfoHeader {
			id: _header
			width: parent.width
			userData: root.userData

			topPadding: root.paddingTop
		}


		Loader {
			id: _log
			asynchronous: true
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			sourceComponent: UserInfoLog {
				username: userData ? userData.username : ""
				visible: _log.status == Loader.Ready
			}

			onLoaded: reload()
		}

	}


	function reload() {
		if (!userData || !userData.username || !_log.item)
			return

		Client.send(WebSocket.ApiGeneral, "user/%1".arg(userData.username))
		.done(function(r){
			userData = Client.userToMap(r)
		})
		.fail(JS.failMessage("Letöltés sikertelen"))

		_log.item.reload()
	}

}

