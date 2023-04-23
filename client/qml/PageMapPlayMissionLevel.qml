import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QPageGradient {
	id: root

	property MapPlay map: null
	property MapPlayMission mission: null
	property MapPlayMissionLevel missionLevel: null

	property bool _firstLoad: true

	MapGameList {
		id: _mapGameList
	}

	QScrollable {
		anchors.fill: parent

		Item {
			width: parent.width
			height: root.paddingTop
		}

		Label {
			width: parent.width
			horizontalAlignment: Text.AlignHCenter
			leftPadding: Math.max(20, Client.safeMarginLeft)
			rightPadding: Math.max(20, Client.safeMarginRight)
			font.family: "HVD Peace"
			font.pixelSize: Qaterial.Style.textTheme.headline3.pixelSize
			text: mission ? mission.name : ""
			wrapMode: Text.Wrap
			bottomPadding: 10
		}

		Qaterial.LabelBody1 {
			anchors.horizontalCenter: parent.horizontalCenter
			horizontalAlignment: Text.AlignHCenter
			text: missionLevel ?
					  qsTr("Level %1").arg(missionLevel.level)
					  +(missionLevel.deathmatch ? qsTr("\nSudden Death") : "")
					: ""
			bottomPadding: 50
		}

		Qaterial.LabelBody1 {
			anchors.horizontalCenter: parent.horizontalCenter
			horizontalAlignment: Text.AlignHCenter
			width: parent.width
			leftPadding: Math.max(50, Client.safeMarginLeft)
			rightPadding: Math.max(50, Client.safeMarginRight)
			text: mission ? mission.description : ""
			wrapMode: Text.Wrap
			bottomPadding: 30
			visible: text != ""
			color: Qaterial.Style.iconColor()
		}

		Qaterial.GroupBox {
			title: qsTr("Játék mód kiválasztása")

			width: Math.min(300*Qaterial.Style.pixelSizeRatio, parent.width, Qaterial.Style.maxContainerSize)

			anchors.horizontalCenter: parent.horizontalCenter
			inlineTitle: true

			ButtonGroup {
				id: _modeGroup
				onClicked: {
					if (map)
						_xpLabel.xp = map.calculateXP(missionLevel, checkedButton.gameMode)
					reload()
				}
			}

			Column {
				Repeater {
					model: [
						GameMap.Test,
						GameMap.Action,
						GameMap.Lite,
						GameMap.Quiz,
						GameMap.Exam
					]

					delegate: Qaterial.RadioButton {
						id: _btn
						readonly property int gameMode: modelData
						//width: parent.width

						visible: mission && mission.modeEnabled(gameMode) && (!missionLevel.deathmatch || gameMode == GameMap.Action)

						text: {
							switch (modelData) {
							case GameMap.Action:
								qsTr("Akciójáték")
								break
							case GameMap.Lite:
								qsTr("Csak feladatmegoldás")
								break
							case GameMap.Test:
								qsTr("Teszt")
								break
							case GameMap.Quiz:
								qsTr("Kvíz")
								break
							case GameMap.Exam:
								qsTr("Dolgozat")
								break
							default:
								""
							}

						}

						Component.onCompleted: {
							_modeGroup.addButton(_btn)
							if (visible && !_modeGroup.checkedButton)
								checked = true
						}
					}
				}
			}

		}

		Qaterial.LabelHeadline3 {
			id: _xpLabel

			property int xp: 0

			text: qsTr("%1 XP").arg(xp)
			anchors.horizontalCenter: parent.horizontalCenter
			topPadding: 30
			bottomPadding: 30
			color: Qaterial.Colors.lightGreen400

			Behavior on xp {
				NumberAnimation {
					duration: 750
					easing.type: Easing.OutQuint
				}
			}
		}

		QButton {
			id: _btnPlay

			anchors.horizontalCenter: parent.horizontalCenter

			icon.source: Qaterial.Icons.play
			icon.width: 36 * Qaterial.Style.pixelSizeRatio
			icon.height: 36 * Qaterial.Style.pixelSizeRatio
			/*font.family: Qaterial.Style.textTheme.headline6.family
			font.pixelSize: Qaterial.Style.textTheme.headline6.pixelSize
			font.capitalization: Font.AllUppercase*/

			bgColor: Qaterial.Colors.green700
			textColor: Qaterial.Colors.white
			//display: AbstractButton.TextUnderIcon
			topPadding: 10
			bottomPadding: 10
			leftPadding: 40
			rightPadding: 40
			enabled: missionLevel && missionLevel.lockDepth == 0 && map && _modeGroup.checkedButton
			text: qsTr("Play")

			outlined: !enabled

			onClicked: map.play(missionLevel, _modeGroup.checkedButton.gameMode)
		}

		Row {
			anchors.left: _listDuration.left
			anchors.right: _listDuration.right

			topPadding: 50
			bottomPadding: 10

			visible: map && map.online && _modeGroup.checkedButton && _modeGroup.checkedButton.gameMode == GameMap.Action && (_mapGameList.length > 0 || _firstLoad)

			Qaterial.LabelSubtitle1 {
				text: qsTr("Leggyorsabb megoldás")
				anchors.verticalCenter: _btnOpen.verticalCenter
				width: parent.width-_btnOpen.width
				elide: Text.ElideRight
			}

			Qaterial.AppBarButton {
				id: _btnOpen
				anchors.bottom: parent.bottom
				icon.source: _listDuration.filterEnabled ? Qaterial.Icons.folderOpen : Qaterial.Icons.folderRemove
				onClicked: _listDuration.filterEnabled = !_listDuration.filterEnabled
			}
		}

		MapPlayMissionLevelInfoList {
			id: _listDuration
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter
			mapGameList: _mapGameList
			positionType: MapPlayMissionLevelInfoList.Duration
			showPlaceholders: _mapGameList.length === 0 && _firstLoad
			visible: map && map.online && _modeGroup.checkedButton && _modeGroup.checkedButton.gameMode == GameMap.Action
		}

		Row {
			anchors.left: _listSolved.left
			anchors.right: _listSolved.right

			topPadding: 20
			bottomPadding: 10

			visible: map && map.online && _modeGroup.checkedButton && _modeGroup.checkedButton.gameMode == GameMap.Action && (_mapGameList.length > 0 || _firstLoad)

			Qaterial.LabelSubtitle1 {
				text: qsTr("Legtöbb megoldás")
				anchors.verticalCenter: _btnOpen2.verticalCenter
				width: parent.width-_btnOpen2.width
				elide: Text.ElideRight
			}

			Qaterial.AppBarButton {
				id: _btnOpen2
				anchors.bottom: parent.bottom
				icon.source: _listSolved.filterEnabled ? Qaterial.Icons.folderOpen : Qaterial.Icons.folderRemove
				onClicked: _listSolved.filterEnabled = !_listSolved.filterEnabled
			}
		}

		MapPlayMissionLevelInfoList {
			id: _listSolved
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter
			mapGameList: _mapGameList
			positionType: MapPlayMissionLevelInfoList.Solved
			showPlaceholders: _mapGameList.length === 0 && _firstLoad
			visible: map && map.online && _modeGroup.checkedButton && _modeGroup.checkedButton.gameMode == GameMap.Action
		}
	}


	function reload() {
		if (!map || !mission || !missionLevel || !_modeGroup.checkedButton)
			return

		if (!map.online) {
			_firstLoad = false
			return
		}

		Client.send(WebSocket.ApiUser, "game/info", {
						map: map.uuid,
						mission: mission.uuid,
						level: missionLevel.level,
						deathmatch: missionLevel.deathmatch,
						mode: GameMap.Action
					}).done(function(r) {
						var maxD = 1
						var minD = 0
						var maxS = 1

						for (var i=0; i<r.list.length; ++i) {
							var l = r.list[i]
							maxD = Math.max(l.dMin, maxD)
							minD = (i == 0 ? l.dMin : Math.min(l.dMin, minD))
							maxS = Math.max(l.num, maxS)
						}

						_listDuration._maxValue = maxD
						_listDuration._minValue = minD
						_listSolved._maxValue = maxS

						Client.callReloadHandler("mapGame", _mapGameList, r.list)
						_firstLoad = false
					})
		.fail(JS.failMessage("Letöltés sikertelen"))
	}

	StackView.onActivated: {
		reload()
		_modeGroup.clicked(null)
	}
}
