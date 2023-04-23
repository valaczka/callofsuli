import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
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
			height: root.paddingTop - parent.spacing
		}

		StudentCampaign {
			anchors.horizontalCenter: parent.horizontalCenter
			campaign: root.campaign
			user: root.user
			mapHandler: studentMapHandler

			ColumnLayout {
				id: _grid
				anchors.horizontalCenter: parent.horizontalCenter

				width: Math.min(implicitWidth, parent.width)

				readonly property bool showPlaceholders: _mapList.count === 0 && _firstRun

				visible: _mapList.count || showPlaceholders


				Repeater {
					model: _grid.showPlaceholders ? 3 : _sortedMapList

					delegate: _grid.showPlaceholders ? _cmpPlaceholder : _cmpButton
				}

				Component {
					id: _cmpButton

					QButton {
						id: _btn
						property StudentMap map: model && model.qtObject ? model.qtObject : null
						text: map ? map.name : ""
						bgColor: "saddlebrown"

						Layout.topMargin: index === 0 ? 20 : undefined
						Layout.fillWidth: true
						horizontalAlignment: Qt.AlignLeft

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
						Layout.fillWidth: true
						Layout.topMargin: index === 0 ? 20 : undefined
						implicitWidth: 200
						widthRatio: 1.0
						heightRatio: 0.85
						height: Qaterial.Style.rawButton.minHeight
						rectangleRadius: Qaterial.Style.rawButton.cornerRadius
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


