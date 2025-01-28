import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "JScript.js" as JS

QPageGradient {
	id: root

	progressBarEnabled: true

	property User user: null
	property Campaign campaign: null
	property StudentMapHandler studentMapHandler: null
	property TeacherMapHandler teacherMapHandler: null
	property TeacherGroupCampaignResultModel campaignResultModel: null

	property bool withResult: false

	property bool _firstRun: true

	stackPopFunction: function() {
		if (!_scrollable.flickable.atYBeginning) {
			_scrollable.flickable.contentY = 0
			return false
		}

		return true
	}

	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.eye
		ToolTip.text: qsTr("Eredmények megjelenítése")
		onClicked: {
			withResult = true
			reload()
		}
		visible: !withResult
	}

	// Külön kell, mert nem az összeset jelenítjük meg!

	StudentMapList {
		id: _mapList
	}

	Campaign {
		id: _tmpCampaign
	}


	QScrollable {
		id: _scrollable
		anchors.fill: parent
		spacing: 15
		contentCentered: !withResult

		visible: user && campaign

		refreshEnabled: true

		onRefreshRequest: reload()

		topPadding: Math.max(verticalPadding, Client.safeMarginTop, root.paddingTop + (withResult ? 25 : 0))


		StudentCampaign {
			anchors.horizontalCenter: parent.horizontalCenter
			campaign: root.campaign
			user: root.user
			mapHandler: studentMapHandler ? studentMapHandler : teacherMapHandler

			ColumnLayout {
				id: _grid
				anchors.horizontalCenter: parent.horizontalCenter

				width: Math.min(implicitWidth, parent.width)

				readonly property bool showPlaceholders: _mapList.count === 0 && _firstRun

				visible: studentMapHandler && (_mapList.count || showPlaceholders)


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

						icon.source: map && map.downloaded ? (campaign && campaign.finished ? Qaterial.Icons.eye : Qaterial.Icons.map) :
															 _playAfterDownload ? Qaterial.Icons.refresh :
																				  Qaterial.Icons.download
						//highlighted: true
						onClicked: if (map && map.downloaded)
									   studentMapHandler.playCampaignMap(campaign, map, "")
								   else {
									   _playAfterDownload = true
									   studentMapHandler.mapDownload(map)
								   }

						property bool _playAfterDownload: false

						Connections {
							target: map

							function onDownloadedChanged() {
								if (_btn._playAfterDownload && downloaded && campaign)
									studentMapHandler.playCampaignMap(campaign, map, "")
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


		Qaterial.LabelHeadline5 {
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			//topPadding: 50+root.paddingTop
			//leftPadding: 50
			//rightPadding: 50
			//horizontalAlignment: Qt.AlignHCenter
			text: qsTr("Teljesített küldetések")
			visible: withResult
		}

		QOffsetListView {
			id: _view

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			refreshEnabled: false
			refreshProgressVisible: false

			visible: withResult && campaign

			delegate: Qaterial.LoaderItemDelegate {
				id: _delegate
				width: _view.width

				text: model.readableMission+" | "+model.readableMap+" [%1%2]".arg(model.level).arg(model.deathmatch ? qsTr(" SD") : "")
				secondaryText: {
					let t = JS.readableTimestamp(model.timestamp)
					switch (model.mode) {
					case GameMap.Action:
						t += qsTr(" [régi akció]")
						break
					case GameMap.Rpg:
						t += qsTr(" [akció]")
						break
					case GameMap.Lite:
						t += qsTr(" [feladatmegoldás]")
						break
					case GameMap.Quiz:
						t += qsTr(" [kvíz]")
						break
					case GameMap.Test:
						t += qsTr(" [teszt]")
						break
					case GameMap.Exam:
						t += qsTr(" [dolgozat]")
						break
					case GameMap.Conquest:
						t += qsTr(" [multiplayer]")
						break
					default:
						break
					}

					t += " "+Client.Utils.formatMSecs(model.duration)

					return t
				}



				enabled: model.success

				Component {
					id: _cmpIcon
					Qaterial.RoundColorIcon
					{
						source: model.success ? Qaterial.Icons.checkBold : Qaterial.Icons.close
						color: model.success ? Qaterial.Colors.green400 : Qaterial.Style.disabledTextColor()
						iconSize: Qaterial.Style.delegate.iconWidth
						enabled: model.success

						fill: true
						width: roundIcon ? roundSize : iconSize
						height: roundIcon ? roundSize : iconSize
					}
				}

				Component {
					id: _cmpMedal
					MedalImage {
						width: 40 * Qaterial.Style.pixelSizeRatio
						height: 40 * Qaterial.Style.pixelSizeRatio
						deathmatch: model.deathmatch
						image: model.medal
						level: model.level
					}
				}

				leftSourceComponent: model.success && model.medal !== "" ? _cmpMedal : _cmpIcon

				rightSourceComponent: Qaterial.LabelSubtitle1 {
					anchors.verticalCenter: parent.verticalCenter
					text: qsTr("%1 XP").arg(Number(model.xp).toLocaleString())
					color: Qaterial.Style.accentColor
				}

			}


			offsetModel: StudentCampaignOffsetModelImpl {
				mapList: studentMapHandler ? studentMapHandler.mapList :
											 teacherMapHandler ? teacherMapHandler.mapList :
																 null
				campaign: root.campaign
				username: teacherMapHandler && root.user ? root.user.username : ""
			}

		}

		flickable.onAtYEndChanged: {
			if (withResult && flickable.atYEnd && flickable.moving)
				_view.offsetModel.fetch()
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

		if (!campaign && campaignResultModel && user) {
			if (campaignResultModel.loadCampaignDataFromUser(_tmpCampaign, user)) {
				campaign = _tmpCampaign
				_firstRun = false
				_tmpCampaign.taskListReloaded()
			}
		}

		if (withResult && campaign)
			_view.offsetModel.reload()
	}

}


