import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPageGradient {
	id: control

	property StudentMapHandler studentMapHandler: null

	title: qsTr("Szabad játék")

	appBar.backButtonVisible: true
	progressBarEnabled: true

	property bool _firstRun: true

	StudentMapList {
		id: _mapList
	}


	QScrollable {
		id: _scroll
		anchors.fill: parent

		refreshEnabled: true
		onRefreshRequest: reloadList()


		Item {
			id: _content
			width: parent.width
			height: Math.max(_scroll.height-_scroll.contentContainer.y, _grid.height)

			QDashboardGrid {
				id: _grid
				anchors.centerIn: parent

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

						icon.source: map && map.downloaded ? Qaterial.Icons.map :
															 _playAfterDownload ? Qaterial.Icons.refresh :
																				  Qaterial.Icons.download

						onClicked: if (map && map.downloaded)
									   studentMapHandler.playCampaignMap(null, map)
								   else {
									   _playAfterDownload = true
									   studentMapHandler.mapDownload(map)
								   }

						property bool _playAfterDownload: false

						Connections {
							target: map

							function onDownloadedChanged() {
								if (_btn._playAfterDownload && downloaded)
									studentMapHandler.playCampaignMap(null, map)
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


			Qaterial.LabelHeadline6 {
				visible: !_grid.visible
				anchors.centerIn: parent
				text: qsTr("Jelenleg nincsen szabadon játszható pálya")
				color: Qaterial.Style.accentColor
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
			reloadList()
		}
	}



	StackView.onActivated: reloadList()


	function reloadList() {
		if (!studentMapHandler)
			return

		Client.send(HttpConnection.ApiUser, "freeplay")
		.done(control, function(r){
			_mapList.clear()
			_firstRun = false

			var l = studentMapHandler.mapList

			for (var i=0; i<l.length; ++i) {
				var m = l.get(i)
				if (r.list.includes(m.uuid)) {
					_mapList.append(m)
				}
			}
		})
		.fail(control, JS.failMessage(qsTr("Szabad játékok letöltése sikertelen")))
	}
}
