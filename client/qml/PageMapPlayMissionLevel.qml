import QtQuick 2.12
import QtQuick.Controls 2.12
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


	MapGameList {
		id: _mapGameList
	}

	QScrollable {
		anchors.fill: parent
		//contentCentered: true

		Item {
			width: parent.width
			height: root.paddingTop
		}

		Qaterial.LabelHeadline3 {
			width: parent.width
			horizontalAlignment: Text.AlignHCenter
			leftPadding: Math.max(20, Client.safeMarginLeft)
			rightPadding: Math.max(20, Client.safeMarginRight)
			font.family: "HVD Peace"
			font.pixelSize: Qaterial.Style.textTheme.headline3.pixelSize
			text: mission ? mission.name : ""
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


		Qaterial.GroupBox {
			title: qsTr("Játék mód kiválasztása")

			anchors.horizontalCenter: parent.horizontalCenter
			inlineTitle: true
			bottomPadding: 50


			ButtonGroup {
				id: _modeGroup
				onCheckedButtonChanged: {
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
			topPadding: 10
			bottomPadding: 10
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
			font.family: Qaterial.Style.textTheme.headline6.family
			font.pixelSize: Qaterial.Style.textTheme.headline6.pixelSize
			font.capitalization: Font.AllUppercase
			bgColor: Qaterial.Colors.green700
			textColor: Qaterial.Colors.white
			display: AbstractButton.TextUnderIcon
			topPadding: 10
			bottomPadding: 10
			leftPadding: 40
			rightPadding: 40
			enabled: missionLevel && missionLevel.lockDepth == 0 && map && _modeGroup.checkedButton
			text: qsTr("Play")

			outlined: !enabled

			onClicked: map.play(missionLevel, _modeGroup.checkedButton.gameMode)
		}

		Qaterial.LabelHeadline6 {
			text: "Idő"
		}

		ListView {
			width: parent.width
			height: contentHeight

			model: SortFilterProxyModel {
				sourceModel: _mapGameList

				sorters: [
					RoleSorter {
						roleName: "posDuration"
						sortOrder: Qt.AscendingOrder
					}
				]

				filters: [
					AnyOf {
						//id: successFilter
						RangeFilter {
							roleName: "posDuration"
							maximumValue: 5
						}
						ValueFilter {
							roleName: "username"
							value: Client.server.user.username
						}
					}

				]
			}

			delegate: QLoaderItemDelegate {
				property MapGame game: model.qtObject

				highlighted: game && game.user.username == Client.server.user.username
				text: game ? game.posDuration+". "+game.user.fullNickName : ""

				leftSourceComponent: UserImage {
					user: game ? game.user : null
					pictureEnabled: false
					sublevelEnabled: false
				}

				rightSourceComponent: Qaterial.LabelCaption {
					anchors.right: parent.right
					text: game ? Client.Utils.formatMSecs(game.durationMin) : ""
					color: Qaterial.Style.primaryTextColor()
				}
			}
		}

		Qaterial.LabelHeadline6 {
			text: "Megoldás"
		}

		ListView {
			id: _listSolved
			width: parent.width
			height: contentHeight

			model: SortFilterProxyModel {
				sourceModel: _mapGameList

				sorters: [
					RoleSorter {
						roleName: "posSolved"
						sortOrder: Qt.AscendingOrder
					}
				]

				filters: [
					AnyOf {
						RangeFilter {
							roleName: "posSolved"
							maximumValue: 5
						}
						ValueFilter {
							roleName: "username"
							value: Client.server.user.username
						}
					}

				]
			}

			delegate: QLoaderItemDelegate {
				property MapGame game: model.qtObject

				height: 30

				highlighted: game && game.user.username == Client.server.user.username
				text: game ? game.posSolved+". "+game.user.fullNickName : ""

				leftSourceComponent: UserImage {
					user: game ? game.user : null
					pictureEnabled: false
					sublevelEnabled: false
				}

				rightSourceComponent: Qaterial.LabelCaption {
					anchors.right: parent.right
					text: game ? game.solved : ""
					color: Qaterial.Style.primaryTextColor()
				}
			}
		}
	}


	function reload() {
		if (!map || !mission || !missionLevel || !_modeGroup.checkedButton)
			return

		Client.send(WebSocket.ApiUser, "game/info", {
						map: map.uuid,
						mission: mission.uuid,
						level: missionLevel.level,
						deathmatch: missionLevel.deathmatch,
						mode: GameMap.Action
					}).done(function(r) {
						Client.callReloadHandler("mapGame", _mapGameList, r.list)
					})
		.fail(JS.failMessage("Letöltés sikertelen"))
	}
}
