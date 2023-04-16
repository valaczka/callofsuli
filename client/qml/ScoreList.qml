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

	property UserList userList: Client.cache("scoreList")
	property ClassList classList: Client.cache("classList")

	property double paddingTop: 0

	background: null

	ListModel {
		id: _preparedClassList

		function reload() {
			clear()

			append({classid: -1, name: qsTr("Mindenki")})

			for (var i=0; i<classList.length; i++) {
				var o = classList.get(i)
				append({classid: o.classid, name: o.name})
			}
		}
	}

	SortFilterProxyModel {
		id: sortedClassList
		sourceModel: _preparedClassList

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

				required property int classid

				QListView {
					id: view

					currentIndex: -1
					height: parent.height
					width: Math.min(parent.width, 700)
					anchors.horizontalCenter: parent.horizontalCenter

					model: SortFilterProxyModel {
						sourceModel: userList

						filters: AllOf {
							ValueFilter {
								roleName: "classid"
								enabled: _item.classid != -1
								value: _item.classid
							}
							ValueFilter {
								roleName: "classid"
								enabled: _item.classid != -1
								value: _item.classid
							}
						}

						sorters: [
							RoleSorter {
								roleName: "xp"
								sortOrder: Qt.DescendingOrder
								priority: 1
							},
							StringSorter {
								roleName: "fullNickName"
								sortOrder: Qt.AscendingOrder
							}
						]
					}

					refreshProgressVisible: Client.webSocket.pending
					refreshEnabled: true
					onRefreshRequest: Client.reloadCache("scoreList")

					delegate: QLoaderItemDelegate {
						id: _delegate
						property User user: model.qtObject

						text: user ? user.fullNickName : ""
						secondaryText: user ? user.rank.name + (user.rank.sublevel > 0 ? qsTr(" (level %1)").arg(user.rank.sublevel) : "") : ""

						leftSourceComponent: UserImage { user: _delegate.user }

						rightSourceComponent: Column {
							Qaterial.LabelHeadline6 {
								anchors.right: parent.right
								text: user ? qsTr("%1 XP").arg(Number(user.xp).toLocaleString()) : ""
								color: Qaterial.Style.accentColor
							}
							Qaterial.LabelCaption {
								anchors.right: parent.right
								text: user ? qsTr("streak: %1").arg(Number(user.streak).toLocaleString()) : ""
								color: Qaterial.Style.primaryTextColor()
							}
						}
					}

					header: Item {
						width: ListView.width
						height: control.paddingTop
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

	function reload() {
		Client.reloadCache("scoreList")
		Client.reloadCache("classList", function(){_preparedClassList.reload()})
	}

	SwipeView.onIsCurrentItemChanged: {
		if (SwipeView.isCurrentItem) {
			reload()
		}
	}
}
