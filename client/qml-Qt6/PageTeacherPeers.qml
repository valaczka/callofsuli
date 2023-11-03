import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	title: Client.server ? Client.server.serverName : ""
	subtitle: Client.server && Client.server.user ? Client.server.user.fullName : ""

	appBar.backButtonVisible: true

	QLiveStream {
		id: _liveStream

		reloadCallback: function() { _list.reload() }
		api: (Client.server.user.roles & Credential.Admin) ? WebSocket.ApiAdmin : WebSocket.ApiTeacher
		path: "user/peers/live"
	}

	Connections {
		target: _liveStream.eventStream
		function onEventJsonReceived(event, json) {
			if (event === "peerUsers")
				_view.loadFromJson(json.list)
		}
	}


	QScrollable {
		anchors.fill: parent
		horizontalPadding: 0
		topPadding: Math.max(verticalPadding, Client.safeMarginTop, control.topPadding)
		bottomPadding: 0

		refreshEnabled: false
		onRefreshRequest: _liveStream.reload()

		QListView {
			id: _view

			currentIndex: -1
			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds


			model: ListModel {
				id: _model
			}

			delegate: QItemDelegate {
				highlighted: ListView.isCurrentItem
				iconSource: Qaterial.Icons.accountOutline

				text: username
				secondaryText: host+" – "+agent
			}

			function reload() {
				Client.send(_liveStream.api, "user/peers")
				.done(control, function(r){
					_view.loadFromJson(r.list)
				})
				.fail(control, function(err) {
					Client.messageWarning(err, qsTr("Sikertelen"))
				})
			}

			function loadFromJson(list) {
				_model.clear()

				for (let i=0; i<list.length; ++i)
					_model.append(list[i])
			}
		}
	}

	StackView.onActivated: _liveStream.reload()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) _liveStream.reload()
}
