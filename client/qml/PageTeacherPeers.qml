import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	title: Client.server ? Client.server.serverName : ""
	subtitle: Client.server && Client.server.user ? Client.server.user.fullName : ""

	appBar.backButtonVisible: true

	Connections {
		target: Client.httpConnection.webSocket

		function onMessageReceived(operation, data) {
			if (operation === "peers")
				_view.loadFromJson(data)
		}
	}


	QScrollable {
		anchors.fill: parent
		horizontalPadding: 0
		topPadding: Math.max(verticalPadding, Client.safeMarginTop, control.topPadding)
		bottomPadding: 0

		refreshEnabled: false
		onRefreshRequest: Client.httpConnection.webSocket.connect()

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


			function loadFromJson(list) {
				_model.clear()

				for (let i=0; i<list.length; ++i)
					_model.append(list[i])
			}
		}
	}

	StackView.onActivated: Client.httpConnection.webSocket.connect()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) Client.httpConnection.webSocket.connect()

	Component.onCompleted: Client.httpConnection.webSocket.observerAdd("peers")
	Component.onDestruction: Client.httpConnection.webSocket.observerRemove("peers")
}
