import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QPageGradient {
	id: root

	property User user: null
	property Campaign campaign: null
	property StudentMapHandler studentMapHandler: null

	property bool _firstRun: true

	StudentMapList {
		id: _mapList
	}

	QScrollable {
		anchors.fill: parent
		spacing: 15
		contentCentered: true

		visible: user && campaign

		refreshEnabled: true

		onRefreshRequest: reload()

		Item {
			width: parent.width
			height: root.paddingTop
		}

		StudentCampaign {
			anchors.horizontalCenter: parent.horizontalCenter
			campaign: root.campaign
			user: root.user
			mapHandler: studentMapHandler


			QDashboardGrid {
				id: _grid
				anchors.horizontalCenter: parent.horizontalCenter

				readonly property bool showPlaceholders: _mapList.count === 0 && _firstRun

				visible: _mapList.count || showPlaceholders
				contentItems: showPlaceholders ? 3 : _mapList.count

				Repeater {
					model: _grid.showPlaceholders ? 3 : _sortedMapList

					delegate: _grid.showPlaceholders ? _cmpPlaceholder : _cmpButton
				}

				Component {
					id: _cmpButton

					QDashboardButton {
						id: _btn
						property StudentMap map: model && model.qtObject ? model.qtObject : null
						text: map ? map.name : ""
						bgColor: "saddlebrown"
						icon.source: map && map.downloaded ? Qaterial.Icons.group :
															 _playAfterDownload ? Qaterial.Icons.refresh :
																				  Qaterial.Icons.download
						//highlighted: true
						onClicked: if (map && map.downloaded)
									   studentMapHandler.playCampaignMap(campaign, map)
								   else {
									   _playAfterDownload = true
									   studentMapHandler.mapDownload(map)
								   }

						property bool _playAfterDownload: false

						Connections {
							target: map

							function onDownloadedChanged() {
								if (_btn._playAfterDownload && downloaded && campaign)
									studentMapHandler.playCampaignMap(campaign, map)
							}
						}
					}
				}

				Component {
					id: _cmpPlaceholder

					QPlaceholderItem {
						widthRatio: 1.0
						heightRatio: 1.0
						width: _grid.buttonSize
						height: _grid.buttonSize
						rectangleRadius: 5
					}
				}

			}

		}

	}

	SortFilterProxyModel {
		id: _sortedMapList
		sourceModel: _mapList

		sorters: [
			StringSorter {
				roleName: "name"
				sortOrder: Qt.AscendingOrder
			}
		]
	}

	Connections {
		target: studentMapHandler

		function onReloaded() {
			_mapList.clear()
			_firstRun = false

			if (!campaign)
				return

			var l = studentMapHandler.mapList
			var maps = campaign.usedMapUuids()

			for (var i=0; i<l.length; ++i) {
				var m = l.get(i)
				if (maps.includes(m.uuid)) {
					_mapList.append(m)
				}
			}
		}
	}

	StackView.onActivated: reload()

	function reload() {
		if (studentMapHandler && campaign)
			studentMapHandler.getUserCampaign(campaign)
	}

}


