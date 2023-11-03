import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "./JScript.js" as JS

Item
{
	id: control

	property StudentGroup group: null
	property alias api: _scoreList.api
	property alias path: _scoreList.path
	property alias apiData: _scoreList.apiData
	property alias sortOrder: _scoreList.sortOrder

	property real topPadding: 0



	QFetchLoaderGroup {
		id: _loaderGroup
	}

	QLiveStream {
		id: _liveStream

		reloadCallback: function() { _scoreList.reload() }
		api: WebSocket.ApiUser
		path: "group/%1/score/live".arg(group.groupid)
	}


	ScoreListImpl {
		id: _scoreList

		api: WebSocket.ApiUser
		path: "group/%1/score".arg(group ? group.groupid : -1)
		sortOrder: ScoreListImpl.SortXPdesc
		eventStream: _liveStream.eventStream
		limit: -1
	}

	QScrollable {
		anchors.fill: parent
		horizontalPadding: 0
		topPadding: Math.max(verticalPadding, Client.safeMarginTop, control.topPadding)
		bottomPadding: 0

		refreshEnabled: true
		onRefreshRequest: _liveStream.reload()

		QListView {
			id: _viewPlaceholder

			visible: _loaderGroup.showPlaceholders

			currentIndex: -1
			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: 10

			delegate: Qaterial.FullLoaderItemDelegate {
				id: _placeholder

				width: ListView.view.width

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


			header: Item {
				width: ListView.width
				height: control.paddingTop
			}

		}


		QListView {
			id: view

			visible: !_loaderGroup.showPlaceholders

			currentIndex: -1
			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: _scoreList.model

			delegate: QFetchLoader {
				id: _ldr

				group: _loaderGroup

				Qaterial.LoaderItemDelegate {
					id: _delegate

					text: fullNickName
					secondaryText: rank.name + (rank.sublevel > 0 ? qsTr(" (level %1)").arg(rank.sublevel) : "")
					highlighted: Client.server && Client.server.user && username === Client.server.user.username

					width: _ldr.ListView.view.width

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

                            Qaterial.LabelCaption {
                                visible: trophy
                                anchors.verticalCenter: parent.verticalCenter
                                text: trophy
                                color: Qaterial.Colors.green700
                            }

                            Qaterial.Icon {
                                visible: trophy
                                anchors.verticalCenter: parent.verticalCenter
                                icon: Qaterial.Icons.trophy
                                color: Qaterial.Colors.green700
                                width: Qaterial.Style.smallIcon*0.8
                                height: Qaterial.Style.smallIcon*0.8
                            }

                            Qaterial.LabelCaption {
                                visible: streak
                                anchors.verticalCenter: parent.verticalCenter
                                text: streak
                                color: Qaterial.Style.primaryTextColor()
                                leftPadding: trophy ? 5 * Qaterial.Style.pixelSizeRatio : 0
                            }
                            Qaterial.Icon {
                                visible: streak
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

	StackView.onActivated: _liveStream.reload()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) _liveStream.reload()

}
