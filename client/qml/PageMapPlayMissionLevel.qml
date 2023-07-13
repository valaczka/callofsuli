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

	progressBarEnabled: true

	property MapPlay map: null
	property MapPlayMission mission: null
	property MapPlayMissionLevel missionLevel: null

	property bool _firstLoad: true
	property bool _currentGameFailed: false

	stackPopFunction: function() {
		if (map && map.gameState == MapPlay.StateFinished) {
			map.gameState = MapPlay.StateSelect
			return false
		}

		return true
	}

	MapGameList {
		id: _mapGameList
	}

	QScrollable {
		anchors.fill: parent

		contentCentered: map && map.gameState != MapPlay.StateSelect

		Item {
			width: parent.width
			height: root.paddingTop
		}

		Label {
			width: parent.width
			horizontalAlignment: Text.AlignHCenter
			leftPadding: Math.max(20 * Qaterial.Style.pixelSizeRatio, Client.safeMarginLeft)
			rightPadding: Math.max(20 * Qaterial.Style.pixelSizeRatio, Client.safeMarginRight)
			font.family: "HVD Peace"
			font.pixelSize: Qaterial.Style.textTheme.headline3.pixelSize
			text: mission ? mission.name : ""
			wrapMode: Text.Wrap
			bottomPadding: 10 * Qaterial.Style.pixelSizeRatio

			visible: map && map.gameState != MapPlay.StateInvalid
		}

		Qaterial.LabelBody1 {
			anchors.horizontalCenter: parent.horizontalCenter
			horizontalAlignment: Text.AlignHCenter
			text: missionLevel ?
					  qsTr("Level %1").arg(missionLevel.level)
					  +(missionLevel.deathmatch ? qsTr("\nSudden Death") : "")
					: ""
			bottomPadding: 20 * Qaterial.Style.pixelSizeRatio
			visible: map && map.gameState != MapPlay.StateInvalid
		}

		Qaterial.LabelBody2 {
			anchors.horizontalCenter: parent.horizontalCenter
			horizontalAlignment: Text.AlignHCenter
			width: parent.width
			leftPadding: Math.max(50 * Qaterial.Style.pixelSizeRatio, Client.safeMarginLeft)
			rightPadding: Math.max(50 * Qaterial.Style.pixelSizeRatio, Client.safeMarginRight)
			text: mission && missionLevel ? (_modeGroup.checkedButton && _modeGroup.checkedButton.gameMode === GameMap.Test ?
												 mission.description + qsTr("\n\nA sikeres teljesítéshez %1% eredményt kell elérni").arg(Math.floor(missionLevel.passed*100)) :
												 mission.description)
										  : ""
			wrapMode: Text.Wrap
			bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
			visible: text != "" && map && map.gameState == MapPlay.StateSelect
			color: Qaterial.Style.iconColor()
		}

		Qaterial.GroupBox {
			title: qsTr("Játék mód kiválasztása")

			width: Math.min(300*Qaterial.Style.pixelSizeRatio, parent.width, Qaterial.Style.maxContainerSize)

			visible: map && map.gameState == MapPlay.StateSelect

			anchors.horizontalCenter: parent.horizontalCenter
			inlineTitle: true

			ButtonGroup {
				id: _modeGroup
				onClicked: {
					if (map && checkedButton)
						_xpLabel.xp = map.calculateXP(missionLevel, checkedButton.gameMode)
					reload()
				}
			}

			Column {
				Repeater {
					model: [
						GameMap.Exam,
						GameMap.Test,
						GameMap.Action,
						GameMap.Lite,
						GameMap.Quiz
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
			topPadding: 7 * Qaterial.Style.pixelSizeRatio
			bottomPadding: 7 * Qaterial.Style.pixelSizeRatio
			color: Qaterial.Colors.lightGreen400

			visible: map && map.gameState == MapPlay.StateSelect && !map.readOnly

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
			icon.width: 28 * Qaterial.Style.pixelSizeRatio
			icon.height: 28 * Qaterial.Style.pixelSizeRatio
			/*font.family: Qaterial.Style.textTheme.headline6.family
			font.pixelSize: Qaterial.Style.textTheme.headline6.pixelSize
			font.capitalization: Font.AllUppercase*/

			bgColor: Qaterial.Colors.green700
			textColor: Qaterial.Colors.white
			//display: AbstractButton.TextUnderIcon
			topPadding: 10 * Qaterial.Style.pixelSizeRatio
			bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
			leftPadding: 40 * Qaterial.Style.pixelSizeRatio
			rightPadding: 40 * Qaterial.Style.pixelSizeRatio
			enabled: missionLevel && missionLevel.lockDepth == 0 && map && _modeGroup.checkedButton
			text:  map && map.gameState != MapPlay.StateSelect && _currentGameFailed ? qsTr("Újra") : qsTr("Play")

			visible: map && !map.readOnly && (map.gameState == MapPlay.StateSelect || (map.gameState == MapPlay.StateFinished && _currentGameFailed))

			outlined: !enabled

			onClicked: map.play(missionLevel, _modeGroup.checkedButton.gameMode)
		}





		// ----- FINISHED ------


		StudentStreak {
			id: _streakRow
			anchors.horizontalCenter: parent.horizontalCenter
			visible: false
			value: 0
			forceToday: false
			maxIconCount: Math.floor((parent.width-implicitLabelWidth)/iconSize)

			Timer {
				id: _streakTimer
				running: false
				repeat: false
				interval: 1250
				triggeredOnStart: false

				property int _newValue: 0
				property bool _newToday: true

				onTriggered: {
					_streakRow.forceToday = _newToday
					_streakRow.value = _newValue
				}
			}
		}

		Qaterial.LabelHeadline4 {
			id: _finishedXP

			property int xp: 0

			text: qsTr("%1 XP megszerezve").arg(xp)
			anchors.horizontalCenter: parent.horizontalCenter
			topPadding: 20 * Qaterial.Style.pixelSizeRatio
			bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
			color: Qaterial.Style.accentColor

			visible: map && map.gameState == MapPlay.StateFinished && map.online

			Behavior on xp {
				NumberAnimation {
					duration: 950
					easing.type: Easing.OutQuint
				}
			}
		}

		ListView {
			id: _xpView
			width: Math.min(Qaterial.Style.maxContainerSize, 768*Qaterial.Style.pixelSizeRatio*0.6)
			anchors.horizontalCenter: parent.horizontalCenter

			visible: map && map.gameState == MapPlay.StateFinished

			height: contentHeight

			model: ListModel {
				id: _xpModel
			}

			boundsBehavior: Flickable.StopAtBounds

			delegate: Qaterial.FullLoaderItemDelegate {
				spacing: 10
				leftPadding: 0
				rightPadding: 0

				width: ListView.view.width

				height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

				contentSourceComponent: Label {
					font: Qaterial.Style.textTheme.body2Upper
					verticalAlignment: Label.AlignVCenter
					text: name
					color: Qaterial.Style.accentColor
				}

				rightSourceComponent: Label {
					font: Qaterial.Style.textTheme.body2Upper
					text: qsTr("%1 XP").arg(xp)
					color: Qaterial.Style.accentColor
				}
			}
		}



		// ----- Unlocks -------

		Row {
			anchors.left: _unlockView.left
			visible: _unlockView.visible
			spacing: 10 * Qaterial.Style.pixelSizeRatio
			topPadding: 30 * Qaterial.Style.pixelSizeRatio

			Qaterial.Icon {
				icon: Qaterial.Icons.lockOff
				anchors.verticalCenter: parent.verticalCenter
				visible: _unlockView.isUnlocked
			}

			Qaterial.LabelSubtitle1 {
				text: _unlockView.isUnlocked ? qsTr("Feloldott szintek") : qsTr("Következő szint")
				anchors.verticalCenter: parent.verticalCenter
			}
		}

		ListView {
			id: _unlockView
			width: Math.min(Qaterial.Style.maxContainerSize, 768*Qaterial.Style.pixelSizeRatio*0.6)
			anchors.horizontalCenter: parent.horizontalCenter

			property bool showPlaceholders: true
			property bool _forceShow: false
			property bool isUnlocked: true

			height: contentHeight

			visible: (showPlaceholders || _model.count) && map && (map.gameState == MapPlay.StateFinished || _forceShow) && !_currentGameFailed

			model: 3

			boundsBehavior: Flickable.StopAtBounds

			ListModel {
				id: _model
			}


			delegate: _cmpPlacholder

			Component {
				id: _cmpDelegate
				Qaterial.ItemDelegate {
					id: _delegate

					required property MapPlayMissionLevel missionLevel
					required property int xp

					icon.source: Qaterial.Icons.playCircle

					width: ListView.view.width
					iconColor: Qaterial.Colors.green400

					textColor: Qaterial.Colors.green400
					secondaryTextColor: Qaterial.Colors.green600

					text: missionLevel.mission.name + qsTr(" (%1 XP)").arg(xp)
					secondaryText: qsTr("Level %1").arg(missionLevel.level) + (missionLevel.deathmatch ? qsTr(" Sudden Death") : "")

					enabled: _modeGroup.checkedButton

					onClicked: {
						root.mission = missionLevel.mission
						root.missionLevel = missionLevel

						map.play(missionLevel, _modeGroup.checkedButton.gameMode)
					}
				}
			}

			Component {
				id: _cmpPlacholder

				Qaterial.FullLoaderItemDelegate {
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

				}

			}
		}



		// Lists



		QExpandableHeader {
			text: qsTr("Leggyorsabb megoldás")
			icon: Qaterial.Icons.timerOutline

			anchors.left: _listDuration.left
			anchors.right: _listDuration.right
			topPadding: 10 * Qaterial.Style.pixelSizeRatio

			visible: _listDuration.visible && (_mapGameList.length > 0 || _firstLoad) &&
					 map && (map.gameState == MapPlay.StateSelect)

			onClicked: _listDuration.filterEnabled = !_listDuration.filterEnabled

			expanded: !_listDuration.filterEnabled
		}

		MapPlayMissionLevelInfoList {
			id: _listDuration
			width: Math.min(Qaterial.Style.maxContainerSize, 768*Qaterial.Style.pixelSizeRatio*0.8)
			anchors.horizontalCenter: parent.horizontalCenter
			mapGameList: _mapGameList
			positionType: MapPlayMissionLevelInfoList.Duration
			showPlaceholders: _mapGameList.length === 0 && _firstLoad
			visible: map && map.online && _modeGroup.checkedButton &&
					 (map.gameState == MapPlay.StateSelect)
		}




		QExpandableHeader {
			text: qsTr("Legtöbb megoldás")
			icon: Qaterial.Icons.counter

			anchors.left: _listSolved.left
			anchors.right: _listSolved.right
			topPadding: 20 * Qaterial.Style.pixelSizeRatio

			visible: _listSolved.visible && _modeGroup.checkedButton && (_mapGameList.length > 0 || _firstLoad) &&
					 map && (map.gameState == MapPlay.StateSelect)

			onClicked: _listSolved.filterEnabled = !_listSolved.filterEnabled

			expanded: !_listSolved.filterEnabled
		}

		MapPlayMissionLevelInfoList {
			id: _listSolved
			width: Math.min(Qaterial.Style.maxContainerSize, 768*Qaterial.Style.pixelSizeRatio*0.8)
			anchors.horizontalCenter: parent.horizontalCenter
			mapGameList: _mapGameList
			positionType: MapPlayMissionLevelInfoList.Solved
			showPlaceholders: _mapGameList.length === 0 && _firstLoad
			visible: map && map.online && _modeGroup.checkedButton && (map.gameState == MapPlay.StateSelect)
		}


		// ----- LOADING ----

		Row {
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 5
			visible: map && map.gameState == MapPlay.StateLoading
			Qaterial.BusyIndicator {
				anchors.verticalCenter: parent.verticalCenter
			}

			Qaterial.LabelBody1 {
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Betöltés...")
			}
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
						mode: _modeGroup.checkedButton.gameMode
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
		if (map.gameState == MapPlay.StateFinished) {
			reloadFinishedData()
		} else if (map.gameState == MapPlay.StatePlay) {
			_unlockView._forceShow = true
		}
	}

	Connections {
		target: map

		function onGameStateChanged() {
			if (map.gameState == MapPlay.StatePlay) {
				_currentGameFailed = false
				_finishedXP.xp = 0
				_streakRow.forceToday = Client.server && Client.server.user ? Client.server.user.streakToday : false
				_streakRow.value = Client.server && Client.server.user ? Client.server.user.streak : 0
				_streakRow.visible = false
				_xpModel.clear()
				_model.clear()

				_unlockView.delegate = _cmpPlacholder
				_unlockView.model = 3
				_unlockView.showPlaceholders = true
			} else if (map.gameState == MapPlay.StateFinished && root.StackView.status === StackView.Active) {
				reloadFinishedData()
			} else {
				_unlockView._forceShow = false
				_streakRow.visible = false
			}
		}

		function onCurrentGameFailed() {
			_currentGameFailed = true
		}


		function onMissionLevelUnlocked(list) {
			_model.clear()
			let gameMode = _modeGroup.checkedButton.gameMode

			for (let i=0; i<list.length; ++i) {
				let ml=list[i]
				if (!ml)
					continue

				if (ml.mission.modeEnabled(gameMode) && (!ml.deathmatch || gameMode === GameMap.Action)) {

					let xp = map.calculateXP(ml, gameMode)

					_model.append({
									  xp: xp,
									  missionLevel: ml
								  })
				}
			}

			if (list.length > 0)
				_unlockView.isUnlocked = true
			else {
				_unlockView.isUnlocked = false

				var nextLevel = map.getNextLevel(root.missionLevel, gameMode)

				if (nextLevel) {
					_model.append({
									  xp: map.calculateXP(nextLevel, gameMode),
									  missionLevel: nextLevel
								  })
				}
			}

			_unlockView.showPlaceholders = false
			_unlockView.model = _model
			_unlockView.delegate = _cmpDelegate
		}
	}


	function reloadFinishedData() {
		_unlockView._forceShow = false

		if (!map || !map.online)
			return

		_finishedXP.xp = map.finishedData.sumXP

		if (map.finishedData.streak !== undefined) {
			_streakRow.visible = true
			_streakRow.value = (map.finishedData.streak-1)
			_streakRow.forceToday = false
			_streakTimer._newValue = map.finishedData.streak
			_streakTimer._newToday = true
			_streakTimer.start()
		}

		if (map.finishedData.xpStreak)
			_xpModel.append({
								name: map.finishedData.longestStreak ? qsTr("Leghosszabb streak") : qsTr("Mai streak"),
								xp: map.finishedData.xpStreak
							})

		if (map.finishedData.xpSolved)
			_xpModel.append({
								name: qsTr("Megoldásért kapott"),
								xp: map.finishedData.xpSolved
							})

		if (map.finishedData.xpGame)
			_xpModel.append({
								name: qsTr("Játék közben szerzett"),
								xp: map.finishedData.xpGame
							})


		if (map.finishedData.xpDuration)
			_xpModel.append({
								name: qsTr("Gyorsabb megoldásért kapott"),
								xp: map.finishedData.xpDuration
							})

	}
}
