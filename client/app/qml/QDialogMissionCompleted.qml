import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS





QDialogPanel {
	id: dialogPanel

	icon: CosStyle.iconTrophy
	titleColor: CosStyle.colorOK

	maximumWidth: 800
	maximumHeight: 700

	required property var gameData
	property bool demoMode: false

	readonly property real contentPixelSize: CosStyle.pixelSize*0.85
	readonly property int contentFontWeight: Font.Medium
	readonly property int contentDataFontWeight: Font.DemiBold

	readonly property bool _isSmall: height < 600

	property real _xp: 0

	Column {
		id: headerColumn
		anchors.top: parent.top
		anchors.horizontalCenter: parent.horizontalCenter

		topPadding: _isSmall ? 10 : 20

		spacing: 5

		Row {
			id: rw0
			anchors.horizontalCenter: parent.horizontalCenter

			spacing: 15

			QTrophyImage {
				height: _isSmall ? CosStyle.pixelSize*2.5 : CosStyle.pixelSize*3.5
				width: height

				anchors.verticalCenter: parent.verticalCenter

				isDeathmatch: gameData.deathmatch
				level: gameData.level
			}

			QLabel {
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Mission completed")
				font.family: "HVD Peace"
				font.pixelSize: _isSmall ? CosStyle.pixelSize*1.3 : CosStyle.pixelSize*1.75
				color: CosStyle.colorOKLighter
			}

			QMedalImage {
				anchors.verticalCenter: parent.verticalCenter
				visible: gameData.medalImage !== undefined && gameData.medalImage.length
				height: _isSmall ? CosStyle.pixelSize*2.5 : CosStyle.pixelSize*3.5
				width: height
				level: gameData.level
				isDeathmatch: gameData.deathmatch
				image: gameData.medalImage !== undefined ? gameData.medalImage : ""
			}

		}

		QLabel {
			anchors.horizontalCenter: parent.horizontalCenter
			id: xpLabel
			text: Math.floor(xp)+" XP"
			font.pixelSize: _isSmall ? CosStyle.pixelSize : CosStyle.pixelSize*1.5
			font.weight: Font.DemiBold
			color: CosStyle.colorOKLighter
			bottomPadding: 10

			property real xp: 0
		}
	}


	Flickable {
		id: flick
		width: parent.width-20
		height: Math.min(parent.height-headerColumn.height, contentHeight)
		anchors.centerIn: parent
		anchors.verticalCenterOffset: headerColumn.height/2

		contentWidth: col.width
		contentHeight: col.height

		clip: true

		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator { }

		Column {
			id: col
			width: parent.width


			QCollapsible {
				id: collapsibleGame
				title: "%1 (level %2%3)".arg(gameData.name).arg(gameData.level).arg(gameData.deathmatch ? " deathmatch" : "")
				interactive: false
				collapsed: true
				backgroundColor: "transparent"

				Column {
					width: parent.width
					bottomPadding: 15


					Row {
						spacing: 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Teljesítve:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}

						QLabel {
							width: collapsibleGame.width/2
							horizontalAlignment: Text.AlignRight
							text: qsTr("%1x").arg(gameData.solvedCount)
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}
					}


					Row {
						spacing: 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Megoldási idő:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
							color: labelDuration.color
						}

						QLabel {
							id: labelDuration
							text: JS.secToMMSS(gameData.duration)
							horizontalAlignment: Text.AlignRight
							font.pixelSize: CosStyle.pixelSize*0.8
							font.weight: Font.DemiBold
							anchors.verticalCenter: parent.verticalCenter
							color: gameData.duration < gameData.minDuration ?
									   CosStyle.colorOKLighter :
									   CosStyle.colorPrimaryLighter
							rightPadding: 10
						}


						ProgressBar {
							id: barDuration

							readonly property bool _isFaster: gameData.duration !== undefined &&
															  gameData.minDuration !== undefined &&
															  gameData.duration < gameData.minDuration

							LayoutMirroring.enabled: _isFaster

							anchors.verticalCenter: parent.verticalCenter
							width: collapsibleGame.width/2-labelDuration.width-labelDurationMin.width
							from: 0
							to: if (_isFaster)
									gameData.minDuration !== undefined ? gameData.minDuration : 0
								else
									gameData.duration !== undefined ? gameData.duration : 0

							value: if (_isFaster)
									   gameData.duration !== undefined ? gameData.minDuration-gameData.duration : 0
								   else
									   gameData.minDuration !== undefined ? gameData.minDuration : 0

							Material.accent: labelDuration.color

							Behavior on value {
								NumberAnimation { duration: 750; easing.type: Easing.OutQuad }
							}
						}

						QLabel {
							id: labelDurationMin
							text: gameData.minDuration !== undefined ? JS.secToMMSS(gameData.minDuration) : JS.secToMMSS(gameData.duration)
							anchors.verticalCenter: parent.verticalCenter
							font.pixelSize: CosStyle.pixelSize*0.8
							font.weight: Font.DemiBold
							leftPadding: 10
							color: labelDuration.color
						}

					}


					Row {
						spacing: 0
						visible: gameData.xp && gameData.xp.solved > 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Teljesítésért kapott:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}

						QLabel {
							width: collapsibleGame.width/2
							horizontalAlignment: Text.AlignRight
							text: qsTr("%1 XP").arg(gameData.xp.solved)
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}
					}

					Row {
						spacing: 0
						visible: gameData.xp && gameData.xp.game > 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Játék közben szerzett:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}

						QLabel {
							width: collapsibleGame.width/2
							horizontalAlignment: Text.AlignRight
							text: qsTr("%1 XP").arg(gameData.xp.game)
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}
					}

					Row {
						spacing: 0
						visible: gameData.xp && gameData.xp.duration > 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Gyorsabb megoldásért kapott:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}

						QLabel {
							width: collapsibleGame.width/2
							horizontalAlignment: Text.AlignRight
							text: qsTr("%1 XP").arg(gameData.xp.duration)
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}
					}


				}

			}


			QCollapsible {
				id: collapsibleStreak
				title: qsTr("Streak")
				interactive: false
				collapsed: true
				backgroundColor: "transparent"
				visible: gameData.streak !== undefined

				Column {
					width: parent.width
					bottomPadding: 15

					Row {
						spacing: 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Streak:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
							color: labelStreak.color
						}

						QLabel {
							id: labelStreak
							text: gameData.streak !== undefined ? gameData.streak.current : 0
							anchors.verticalCenter: parent.verticalCenter
							horizontalAlignment: Text.AlignRight
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight

							rightPadding: 10


							color: gameData.streak !== undefined && gameData.streak.current > gameData.streak.last
								   ? (gameData.streak.current > gameData.streak.max ? CosStyle.colorAccent : CosStyle.colorOKLighter )
								   : CosStyle.colorPrimaryLighter

						}


						ProgressBar {
							id: barStreak

							anchors.verticalCenter: parent.verticalCenter
							width: collapsibleGame.width/2-labelStreak.width-labelStreakMax.width
							from: 0
							to: gameData.streak !== undefined ? Math.max(2, gameData.streak.current+1, gameData.streak.max) : 0
							value: gameData.streak !== undefined && gameData.streak.current !== undefined ? gameData.streak.current : 0

							Material.accent: labelStreak.color

							Behavior on value {
								NumberAnimation { duration: 750; easing.type: Easing.OutQuad }
							}
						}

						QLabel {
							id: labelStreakMax
							visible: gameData.minDuration > 0
							text: gameData.streak !== undefined ? gameData.streak.max : ""
							anchors.verticalCenter: parent.verticalCenter
							horizontalAlignment: Text.AlignRight
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight
							leftPadding: 10
							color: labelStreak.color
						}

					}

					Row {
						spacing: 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Streak kezdete:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
							color: labelStreak.color
						}

						QLabel {
							property date _d: new Date()
							property int _streak: gameData.streak !== undefined ? gameData.streak.current-1 : 0

							width: collapsibleGame.width/2
							horizontalAlignment: Text.AlignLeft
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight
							anchors.verticalCenter: parent.verticalCenter
							elide: Text.ElideRight
							color: labelStreak.color

							on_StreakChanged: {
								var t = new Date(_d)
								t.setDate(_d.getDate()-_streak)
								text = t.toLocaleDateString(Qt.locale(), "yyyy. MMMM d. ddd")
							}
						}
					}

					Row {
						spacing: 0
						visible: gameData.xp && gameData.xp.streak > 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Streakért kapott:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}

						QLabel {
							width: collapsibleGame.width/2
							horizontalAlignment: Text.AlignRight
							text: qsTr("%1 XP").arg(gameData.xp.streak)
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}
					}

					Row {
						spacing: 0
						visible: gameData.xp && gameData.xp.maxStreak > 0

						QLabel {
							width: collapsibleGame.width/2
							text: qsTr("Leghosszabb streakért kapott:")
							font.pixelSize: contentPixelSize
							font.weight: contentFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}

						QLabel {
							width: collapsibleGame.width/2
							horizontalAlignment: Text.AlignRight
							text: qsTr("%1 XP").arg(gameData.xp.maxStreak)
							font.pixelSize: contentPixelSize
							font.weight: contentDataFontWeight
							anchors.verticalCenter: parent.verticalCenter
						}
					}
				}
			}



			QCollapsible {
				title: qsTr("Feloldások")
				interactive: false
				collapsed: false
				backgroundColor: "transparent"
				visible: gameData.unlocks !== undefined && gameData.unlocks.length > 0

				Column {
					width: parent.width
					//bottomPadding: 15

					Repeater {
						model: gameData.unlocks

						Row {
							spacing: 0

							QFontImage {
								id: imgMission
								anchors.verticalCenter: parent.verticalCenter
								icon: CosStyle.iconLockOpened
								color: CosStyle.colorOKLighter
								height: Math.max(labelMission.height, btnPlay.height, CosStyle.baseHeight)
								width: height
								size: height*0.8
							}


							QLabel {
								id: labelMission
								width: collapsibleGame.width-imgMission.width-btnPlay.width
								text: modelData.name+" - Level %1".arg(modelData.level)+(modelData.deathmatch ? " deathmatch": "")
								font.pixelSize: CosStyle.pixelSize
								font.weight: Font.Medium
								anchors.verticalCenter: parent.verticalCenter
								elide: Text.ElideRight
								leftPadding: 5
								rightPadding: 10
								font.capitalization: Font.AllUppercase
								color: CosStyle.colorOKLight
							}

							QButton {
								id: btnPlay
								text: qsTr("Play")
								icon.source: CosStyle.iconPlay
								themeColors: CosStyle.buttonThemeGreen

								onClicked: {
									acceptedData = {
										"missionid": modelData.missionid,
										"level": modelData.level,
										"deathmatch": modelData.deathmatch
									}
									dlgClose()
								}
							}

						}

					}
				}
			}




			QCollapsible {
				title: qsTr("Következő")
				interactive: false
				collapsed: false
				backgroundColor: "transparent"
				visible: gameData.next !== undefined

				Column {
					width: parent.width
					//bottomPadding: 15

					Row {
						spacing: 0

						Image {
							id: imgMission2
							anchors.verticalCenter: parent.verticalCenter
							source: gameData.next !== undefined ? cosClient.medalIconPath(gameData.next.image) : ""
							height: Math.max(labelMission2.height, btnPlay2.height, CosStyle.baseHeight)
							width: height
							fillMode: Image.PreserveAspectFit
						}


						QLabel {
							id: labelMission2
							width: collapsibleGame.width-imgMission2.width-btnPlay2.width
							text: gameData.next !== undefined ?
									  gameData.next.name+" - Level %1".arg(gameData.next.level)+(gameData.next.deathmatch ? " deathmatch": "") :
									  ""
							font.pixelSize: CosStyle.pixelSize
							font.weight: Font.Medium
							anchors.verticalCenter: parent.verticalCenter
							elide: Text.ElideRight
							leftPadding: 5
							rightPadding: 10
							font.capitalization: Font.AllUppercase
							color: CosStyle.colorAccentLighter
						}

						QButton {
							id: btnPlay2
							text: qsTr("Play")
							icon.source: CosStyle.iconPlay
							themeColors: CosStyle.buttonThemeGreen

							onClicked: {
								acceptedData = {
									"missionid": gameData.next.missionid,
									"level": gameData.next.level,
									"deathmatch": gameData.next.deathmatch
								}
								dlgClose()
							}
						}

					}

				}
			}

		}
	}



	states: [
		State {
			name: "VISIBLED"
			PropertyChanges {
				target: xpLabel
				xp: _xp
			}
			PropertyChanges {
				target: collapsibleGame
				collapsed: false
			}
			PropertyChanges {
				target: collapsibleStreak
				collapsed: false
			}
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "VISIBLED"

			SequentialAnimation {
				NumberAnimation {
					target: xpLabel
					property: "xp"
					duration: 1450
					easing.type: Easing.OutQuad
				}

				PropertyAction {
					targets: [collapsibleGame, collapsibleStreak]
					property: "collapsed"
				}
			}
		}
	]


	buttons: QButton {
		id: buttonOk
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Bezárás")
		icon.source: CosStyle.iconClose

		onClicked: dialogPanel.dlgClose()
	}


	function populated() {
		buttonOk.forceActiveFocus()

		//collapsibleGame.collapsed = false
		//collapsibleStreak.collapsed = false

		var xp = 0
		if (gameData.xp) {
			if (gameData.xp.solved > 0)
				xp += gameData.xp.solved

			if (gameData.xp.game > 0)
				xp += gameData.xp.game

			if (gameData.xp.duration > 0)
				xp += gameData.xp.duration

			if (gameData.xp.streak > 0)
				xp += gameData.xp.streak

			if (gameData.xp.maxStreak > 0)
				xp += gameData.xp.maxStreak
		}

		//xpLabel.xp = xp
		_xp = xp
		state = "VISIBLED"
	}
}


