import QtQuick 2.15
import QtQuick.Controls 2.15
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

	property bool _firstRun: true

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



	QListView {
		id: view

		property int classid: -1

		readonly property bool showPlaceholders: userList && userList.count === 0 && _firstRun

		currentIndex: -1
		height: parent.height
		width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
		anchors.horizontalCenter: parent.horizontalCenter

		model: showPlaceholders ? 10 : _sortedUserList


		refreshEnabled: true
		onRefreshRequest: Client.reloadCache("scoreList")

		delegate: showPlaceholders ? _cmpPlaceholder : _cmpDelegate


		onClassidChanged: positionViewAtBeginning()
		onShowPlaceholdersChanged: positionViewAtBeginning()
		Component.onCompleted: positionViewAtBeginning()

		SortFilterProxyModel {
			id: _sortedUserList
			sourceModel: userList

			filters: ValueFilter {
				roleName: "classid"
				enabled: view.classid != -1
				value: view.classid
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

		Component {
			id: _cmpDelegate

			QLoaderItemDelegate {
				id: _delegate
				property User user: model && model.qtObject ? model.qtObject : null

				text: user ? user.fullNickName : ""
				secondaryText: user ? user.rank.name + (user.rank.sublevel > 0 ? qsTr(" (level %1)").arg(user.rank.sublevel) : "") : ""

				leftSourceComponent: UserImage { user: _delegate.user }

				rightSourceComponent: Column {
					Qaterial.LabelSubtitle1 {
						anchors.right: parent.right
						text: user ? qsTr("%1 XP").arg(Number(user.xp).toLocaleString()) : ""
						color: Qaterial.Style.accentColor
					}
					Row {
						anchors.right: parent.right
						spacing: 2
						visible: user && user.streak
						Qaterial.LabelCaption {
							anchors.verticalCenter: parent.verticalCenter
							text: user ? user.streak : ""
							color: Qaterial.Style.primaryTextColor()
						}
						Qaterial.Icon {
							anchors.verticalCenter: parent.verticalCenter
							icon: Qaterial.Icons.fire
							color: Qaterial.Colors.orange500
							width: Qaterial.Style.smallIcon*0.8
							height: Qaterial.Style.smallIcon*0.8
						}
					}
				}
			}
		}

		Component {
			id: _cmpPlaceholder

			QLoaderItemFullDelegate {
				id: _placeholder
				contentSourceComponent: QPlaceholderItem {
					heightRatio: 0.5
					horizontalAlignment: Qt.AlignLeft
				}

				leftSourceComponent: QPlaceholderItem {
					width: _placeholder.height
					height: _placeholder.height
					widthRatio: 0.8
					heightRatio: 0.8
					contentComponent: ellipseComponent
				}

				rightSourceComponent: QPlaceholderItem {
					fixedWidth: 75
					heightRatio: 0.5
				}
			}

		}

		header: Item {
			width: ListView.width
			height: control.paddingTop
		}

	}


	footer: Qaterial.ScrollableTabBar
	{
		id: tabBar
		width: parent.width

		onCurrentIndexChanged: {
			var o = model.get(currentIndex)
			view.classid = o && o.classid ? o.classid : -1
		}

		onPrimary: true
		model: sortedClassList

		textRole: "name"

		leftPadding: 0
		rightPadding: 0

	}

	function reload() {
		Client.reloadCache("scoreList")
		Client.reloadCache("classList", function(){_preparedClassList.reload()})
		_firstRun = false
	}
}
