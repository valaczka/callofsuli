import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

Qaterial.Page
{
	id: control

	implicitWidth: 200
	implicitHeight: 200

	property ClassList classList: Client.cache("classList")

	property double paddingTop: 0

	property bool _firstRun: true

	background: null

	ScoreListImpl {
		id: _scoreList

		onModelReloaded: _firstRun = false
	}

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


	SortFilterProxyModel {
		id: _sortedUserList
		sourceModel: _scoreList.model

		filters: ValueFilter {
			id: _classidFilter

			property int classid: -1

			onClassidChanged: view.positionViewAtBeginning()

			roleName: "classid"
			enabled: classid != -1
			value: classid
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


	QScrollable {
		anchors.fill: parent
		horizontalPadding: 0
		topPadding: 0
		bottomPadding: 0

		refreshEnabled: true
		onRefreshRequest: reload()


		QListView {
			id: view

			readonly property bool showPlaceholders: _scoreList.model.count == 0 && _firstRun

			currentIndex: -1
			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: showPlaceholders ? 10 : _sortedUserList

			delegate: showPlaceholders ? _cmpPlaceholder : _cmpDelegate

			onShowPlaceholdersChanged: positionViewAtBeginning()
			Component.onCompleted: positionViewAtBeginning()


			Component {
				id: _cmpDelegate

				QLoaderItemDelegate {
					id: _delegate

					text: fullNickName
					secondaryText: rank.name + (rank.sublevel > 0 ? qsTr(" (level %1)").arg(rank.sublevel) : "")
					highlighted: Client.server && Client.server.user && username === Client.server.user.username

					leftSourceComponent: UserImage { userData: model }

					rightSourceComponent: Column {
						Qaterial.LabelSubtitle1 {
							anchors.right: parent.right
							text: qsTr("%1 XP").arg(Number(xp).toLocaleString())
							color: Qaterial.Style.accentColor
						}
						Row {
							anchors.right: parent.right
							spacing: 2
							visible: streak
							Qaterial.LabelCaption {
								anchors.verticalCenter: parent.verticalCenter
								text: streak
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

					onClicked: {
						Client.stackPushPage("ScoreUserInfo.qml", {
												 userData: model
											 })
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

	}

	footer: Qaterial.ScrollableTabBar
	{
		id: tabBar
		width: parent.width

		onCurrentIndexChanged: {
			if (currentIndex != -1) {
				var o = model.get(currentIndex)
				_classidFilter.classid = o.classid
			}
		}

		onPrimary: true
		model: sortedClassList

		textRole: "name"

		leftPadding: 0
		rightPadding: 0

	}

	Component.onCompleted: Client.reloadCache("classList", function(){_preparedClassList.reload()})

	function reload() {
		_scoreList.reload()
	}
}
