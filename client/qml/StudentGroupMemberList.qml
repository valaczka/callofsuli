import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "./JScript.js" as JS

Item
{
	id: control

	property StudentGroup group: null
	property alias api: _scoreList.api
	property alias path: _scoreList.path
	property alias apiData: _scoreList.apiData
	property alias sortOrder: _scoreList.sortOrder

	property EventStream eventStream: null

	property real topPadding: 0

	property bool _firstRun: true

	readonly property bool _live: Qt.platform.os == "wasm" ? false : true

	ScoreListImpl {
		id: _scoreList

		api: WebSocket.ApiUser
		path: "group/%1/score".arg(group ? group.groupid : -1)
		sortOrder: ScoreListImpl.SortXPdesc
		eventStream: control.eventStream

		onModelReloaded: {
			_firstRun = false
		}
	}

	QScrollable {
		anchors.fill: parent
		horizontalPadding: 0
		topPadding: Math.max(verticalPadding, Client.safeMarginTop, control.topPadding)
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

			model: showPlaceholders ? 10 : _scoreList.model

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

			move: Transition {
				NumberAnimation { properties: "x,y"; duration: 450 }
			}

			displaced: Transition {
				NumberAnimation { properties: "x,y"; duration: 450 }
			}

		}

	}

	StackView.onActivated: reload()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) reload()


	Timer {
		interval: 5000
		triggeredOnStart: true
		repeat: true
		running: !_live
		onTriggered: _scoreList.reload()
	}


	function reload() {
		if (!group)
			return

		if (eventStream) {
			eventStream.destroy()
			eventStream = null
		}

		if (_live)
			eventStream = Client.webSocket.getEventStream(WebSocket.ApiUser, "group/%1/score/live".arg(group.groupid))
	}

	Component.onDestruction: {
		if (eventStream) {
			eventStream.destroy()
			eventStream = null
		}
	}
}
