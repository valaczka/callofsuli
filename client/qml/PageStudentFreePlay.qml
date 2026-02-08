import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPageGradient {
	id: control

	property StudentMapHandler studentMapHandler: null

	title: qsTr("Szabad játék")

	appBar.backButtonVisible: true
	progressBarEnabled: true

	property bool _firstRun: studentMapHandler && !studentMapHandler.offlineEngine

	TeacherGroupFreeMapList {
		id: _mapList
	}


	QScrollable {
		id: _scroll
		anchors.fill: parent

		refreshEnabled: true
		onRefreshRequest: reloadList()

		QListView {
			id: view

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			height: contentHeight
			boundsBehavior: Flickable.StopAtBounds

			currentIndex: -1
			autoSelectChange: false

			refreshEnabled: false

			readonly property bool showPlaceholders: _mapList.count === 0 && _firstRun

			model: SortFilterProxyModel {
				sourceModel: _mapList

				sorters: [
					StringSorter {
						roleName: "name"
						sortOrder: Qt.AscendingOrder
						priority: 2
					},
					StringSorter {
						roleName: "missionUuid"
						sortOrder: Qt.AscendingOrder
						priority: 1
					}

				]
			}


			header: Item {
				width: ListView.width
				height: control.paddingTop
			}

			delegate: QItemDelegate {
				id: _delegate

				property TeacherGroupFreeMap mapObject: model.qtObject

				readonly property bool _downloaded: mapObject && mapObject.map && mapObject.map.downloaded
				property bool _playAfterDownload: false

				iconColor: Qaterial.Style.iconColor()

				text: mapObject ? (mapObject.missionUuid !== "" ? mapObject.missionName() : mapObject.name)
								: ""

				secondaryText: mapObject && mapObject.missionUuid !== "" ? mapObject.name : ""

				iconSource:  _downloaded ? Qaterial.Icons.map :
										   _playAfterDownload ? Qaterial.Icons.refresh :
																Qaterial.Icons.download

				onClicked: if (_downloaded)
							   studentMapHandler.playCampaignMap(null, mapObject.map, mapObject.missionUuid)
						   else {
							   _playAfterDownload = true
							   studentMapHandler.mapDownload(mapObject.map)
						   }

				Connections {
					target: _delegate.mapObject ? _delegate.mapObject.map : null

					function onDownloadedChanged() {
						if (_delegate._playAfterDownload && _delegate.mapObject.map.downloaded)
							studentMapHandler.playCampaignMap(null, _delegate.mapObject.map, _delegate.mapObject.missionUuid)
					}
				}
			}
		}


		Repeater {
			id: _cmpPlacholder

			model: 3

			delegate: Qaterial.FullLoaderItemDelegate {
				id: _delegatePlaceholder

				visible: view.showPlaceholders

				width: view.width
				height: Qaterial.Style.textTheme.body2.pixelSize*3 + topPadding+bottomPadding+topInset+bottomInset

				anchors.horizontalCenter: parent.horizontalCenter

				spacing: 10  * Qaterial.Style.pixelSizeRatio
				leftPadding: 0
				rightPadding: 0

				contentSourceComponent: QPlaceholderItem {
					horizontalAlignment: Qt.AlignLeft
					heightRatio: 0.5
				}

				leftSourceComponent: QPlaceholderItem {
					contentComponent: ellipseComponent
					fixedHeight: _delegatePlaceholder.height-6
					fixedWidth: fixedHeight
				}
			}
		}
	}



	Qaterial.LabelHeadline6 {
		visible: _mapList.count === 0 && !view.showPlaceholders
		anchors.centerIn: parent
		text: qsTr("Jelenleg nincsen szabadon játszható pálya")
		color: Qaterial.Style.accentColor
	}


	Connections {
		target: studentMapHandler

		function onReloaded() {
			reloadList()
		}
	}




	StackView.onActivated: {
		Client.contextHelper.setCurrentContext(ContextHelperData.ContextStudentFreePlay)
		reloadList()
	}

	StackView.onDeactivating: {
		Client.contextHelper.unsetContext(ContextHelperData.ContextStudentFreePlay)
	}


	function reloadList() {
		if (!studentMapHandler)
			return

		studentMapHandler.reloadFreePlayMapList(_mapList)

	}

}
