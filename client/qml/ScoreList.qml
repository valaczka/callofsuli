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

	background: null

	QFetchLoaderGroup {
		id: _loaderGroup
		showPlaceholders: true
	}

	ScoreListImpl {
		id: _scoreList

		filterClassId: -1
		sortOrder: ScoreListImpl.SortXPdesc

		onFilterClassIdChanged: _scrollable.flickable.contentY = 0
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


	QScrollable {
		id: _scrollable

		anchors.fill: parent
		horizontalPadding: 0
		topPadding: 0
		bottomPadding: 0

		refreshEnabled: true
		onRefreshRequest: _scoreList.reload()

		ListView {
			id: _viewPlaceholder

			visible: _loaderGroup.showPlaceholders

			height: contentHeight

			width: view.width
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


		QFetchListView {
			id: view

			visible: !_loaderGroup.showPlaceholders

			currentIndex: -1
			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize*0.85)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			fetchModel: _scoreList
			flickable: _scrollable.flickable
			fillHeight: _scrollable.height


			delegate: QFetchLoader {
				id: _ldr

				group: _loaderGroup
				sourceComponent: Qaterial.LoaderItemPlaceholderDelegate {
					id: _delegate

					visible: _ldr.status == Loader.Ready

					width: _ldr.ListView.view.width

					text: (index+1)+". "+fullNickName
					secondaryText: rank.name + (rank.sublevel > 0 ? qsTr(" (level %1)").arg(rank.sublevel) : "")
					highlighted: Client.server && Client.server.user && username === Client.server.user.username

					leftPlaceholderSourceComponent: QPlaceholderItem {
						width: _delegate.height
						height: _delegate.height
						widthRatio: 1.0
						heightRatio: 1.0
						contentComponent: ellipseComponent
					}

					leftSourceComponent: UserImage { userData: model }

					rightAsynchronous: false

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
				_scoreList.filterClassId = o.classid
			}
		}

		onPrimary: true
		model: sortedClassList

		textRole: "name"

		leftPadding: 0
		rightPadding: 0

	}

	Component.onCompleted: {
		Client.reloadCache("classList", function(){_preparedClassList.reload()})
	}

	function reload() {
		_scoreList.reload()
	}
}
