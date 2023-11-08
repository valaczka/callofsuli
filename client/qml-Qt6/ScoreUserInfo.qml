import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
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
			id: _counter
			asynchronous: true
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			sourceComponent: UserInfoCounter {
				visible: _counter.status == Loader.Ready
				userLogList: _userLog
			}

			onLoaded: reload()
		}

		Loader {
			id: _log
			asynchronous: true
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			sourceComponent: UserInfoLog {
				visible: _log.status == Loader.Ready
				userLogList: _userLog
			}

			onLoaded: reload()
		}

	}



	UserLogListImpl {
		id: _userLog
		username: userData ? userData.username: ""
	}


	function reload() {
		if (!userData || !userData.username || !_log.item)
			return

		Client.send(HttpConnection.ApiGeneral, "user/%1".arg(userData.username))
		.done(root, function(r){
			userData = Client.userToMap(r)
			_userLog.reload()
		})
		.fail(root, JS.failMessage("Letöltés sikertelen"))
	}

}


