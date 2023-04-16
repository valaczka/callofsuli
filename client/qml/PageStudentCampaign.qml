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

	StudentMapList {
		id: _mapList
	}

	QScrollable {
		anchors.fill: parent
		spacing: 15

		visible: user && campaign

		refreshEnabled: true

		onRefreshRequest: reload()

		Qaterial.LabelHeadline5 {
			width: parent.width
			topPadding: 50+root.paddingTop
			leftPadding: 50
			rightPadding: 50
			horizontalAlignment: Qt.AlignHCenter
			text: qsTr("Hadjárat")
		}

		Qaterial.LabelHeadline6 {
			anchors.horizontalCenter: parent.horizontalCenter
			text: campaign ? campaign.readableName : ""
		}


		Repeater {
			model: campaign ? campaign.taskList : null

			Qaterial.LabelBody2 {
				property Task task: model.qtObject
				anchors.horizontalCenter: parent.horizontalCenter
				text: JSON.stringify(task.criterion)
			}

		}

		Qaterial.HorizontalLineSeparator {
			anchors.horizontalCenter: parent.horizontalCenter
		}

		QDashboardGrid {
			anchors.horizontalCenter: parent.horizontalCenter

			visible: _mapList.count
			contentItems: _mapList.count

			Repeater {
				model: SortFilterProxyModel {
					sourceModel: _mapList

					sorters: [
						StringSorter {
							roleName: "name"
							sortOrder: Qt.AscendingOrder
						}
					]
				}

				QDashboardButton {
					property StudentMap map: model.qtObject
					text: map ? map.name : ""
					icon.source: map && map.downloaded ? Qaterial.Icons.group : Qaterial.Icons.download
					//highlighted: true
					onClicked: if (map && map.downloaded)
								   studentMapHandler.playCampaignMap(campaign, map)
							   else
								   studentMapHandler.mapDownload(map)
				}
			}
		}

	}


	Connections {
		target: studentMapHandler

		function onReloaded() {
			_mapList.clear()

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


