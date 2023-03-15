import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2

Qaterial.Page
{
	id: control

	implicitWidth: 200
	implicitHeight: 200

	property UserList userList: Client.cache("userScoreList")
	property ClassList classList: Client.cache("classList")

	background: Rectangle { color: "transparent" }

	property bool _loadedFirst: true

	/*AsyncMessageHandler {
		id: msgHandler

		function userList(obj : QJsonObject) {
			if (obj.status === "ok")
				Client.setCache("userScoreList", obj.list)
			else
				Client.messageWarning(qsTr("Nem sikerült frissíteni az adatokat"))
		}

		function classList(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.loadClassListFromArray(obj.list)
				if (_loadedFirst) {
					swipeView.setCurrentIndex(0)
					_loadedFirst = false
				}
			} else
				Client.messageWarning(qsTr("Nem sikerült frissíteni az adatokat"))
		}
	}*/


	SortFilterProxyModel {
		id: sortedClassList
		sourceModel: classList

		filters: [
			RangeFilter {
				roleName: "classid"
				minimumValue: -1
			}
		]

		sorters: [
			FilterSorter {
				ValueFilter {
					roleName: "classid"
					value: -1
				}
				priority: 1
			},
			StringSorter {
				roleName: "name"
				sortOrder: Qt.AscendingOrder
			}
		]
	}


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		clip: true

		Repeater {
			model: sortedClassList

			Item {
				id: _item
				property ClassObject _class: model.qtObject

				QListView {
					id: view

					currentIndex: -1
					height: parent.height
					width: Math.min(parent.width, 700)
					anchors.horizontalCenter: parent.horizontalCenter

					model: SortFilterProxyModel {
						sourceModel: userList

						filters: [
							ValueFilter {
								roleName: "classid"
								enabled: _item._class.classid != -1
								value: _item._class.classid
							}
						]

						sorters: [
							RoleSorter {
								roleName: "xp"
								sortOrder: Qt.DescendingOrder
								priority: 1
							},
							StringSorter {
								roleName: "fullName"
								sortOrder: Qt.AscendingOrder
							}
						]
					}

					/*refreshProgressVisible: msgHandler.pending
					refreshEnabled: true
					onRefreshRequest: msgHandler.sendRequestFunc(WebSocketMessage.ClassGeneral, "userList")*/

					delegate: QLoaderItemDelegate {
						id: _delegate
						property User user: model.qtObject

						text: user ? user.fullName : ""
						secondaryText: user ? user.rank.name + (user.rank.sublevel > 0 ? qsTr(" (level %1)").arg(user.rank.sublevel) : "") : ""

						leftSourceComponent: UserImage { user: _delegate.user }

						rightSourceComponent: Qaterial.LabelHeadline5 {
							text: user ? qsTr("%1 XP").arg(Number(user.xp).toLocaleString()) : ""
							color: Qaterial.Style.accentColor
						}


						//onClicked: if (!view.selectEnabled)
						//			   Client.connectToServer(server)
					}

				}
			}
		}

	}

	footer: Qaterial.ScrollableTabBar
	{
		id: tabBar
		width: parent.width
		currentIndex: swipeView.currentIndex

		onPrimary: true
		model: sortedClassList

		textRole: "name"

		leftPadding: 0
		rightPadding: 0

	}

	/*SwipeView.onIsCurrentItemChanged: {
		if (SwipeView.isCurrentItem) {
			msgHandler.sendRequestFunc(WebSocketMessage.ClassGeneral, "userList")
			msgHandler.sendRequestFunc(WebSocketMessage.ClassGeneral, "classList")
		}
	}*/
}
