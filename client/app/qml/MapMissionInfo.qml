import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.14
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSimpleContainer {
	id: panel

	maximumWidth: -1
	isPanelVisible: false

	property int selectedMissionIndex: -1
	property var selectedData: null
	property int _currentSelectedMissionIndex: -1

	property QBasePage basePage: null

	property bool _isCurrentPage: StackView.view ? StackView.status == StackView.Active : false

	on_IsCurrentPageChanged: if (basePage)
								 basePage._currenPageIsInfo = _isCurrentPage

	title: ""
	icon: "image://font/School/\uf1c4"


	Connections {
		target: studentMaps

		function onMissionSelected(index) {
			selectedMissionIndex = index
			loadMission()
		}

		function onMissionListChanged() {
			loadMission()
		}
	}


	function loadMission() {
		var _lValue = spinLevel.value
		var _mValue = spinMode.value

		panel.title = ""
		selectedData = null
		spinLevel.value = -1

		if (selectedMissionIndex == -1) {
			_currentSelectedMissionIndex = -1

			if (stackMode && pageStack)
				pageStack.pop()
		} else {
			var x = studentMaps.modelMissionList.get(selectedMissionIndex)
			if (Object.keys(x).length) {
				panel.title = x.name
				selectedData = x

				if (_currentSelectedMissionIndex == selectedMissionIndex) {
					spinLevel.value = _lValue
					spinMode.value = _mValue
				} else {
					spinLevel.value = 0
					spinMode.value = 0
				}

				_currentSelectedMissionIndex = selectedMissionIndex
			} else {
				_currentSelectedMissionIndex = -1
			}
		}
	}

	Image {
		id: bgImage
		anchors.fill: parent
		source: selectedData ? selectedData.backgroundImage : ""
		fillMode: Image.PreserveAspectCrop
		visible: false
	}

	Desaturate {
		id: bgDesaturate
		anchors.fill: bgImage
		source: bgImage
		desaturation: 0.7
		visible: false
	}

	BrightnessContrast {
		anchors.fill: bgImage
		source: bgDesaturate
		brightness: -0.7
	}

	QLabel {
		id: noLabel
		opacity: selectedData ? 0.0 : 1.0
		visible: opacity != 0

		anchors.centerIn: parent

		text: qsTr("Válassz küldetést")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}


	QLabel {
		id: titleLabel

		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right

		opacity: text.length ? 1.0 : 0.0
		visible: opacity != 0

		topPadding: parent.height > 500 ? 50 : 20
		bottomPadding: topPadding
		rightPadding: 100
		leftPadding: 100

		text: selectedData ? selectedData.name : ""

		font.family: "HVD Peace"
		font.pixelSize: CosStyle.pixelSize*1.6

		color: CosStyle.colorAccentLighter

		horizontalAlignment: Text.AlignHCenter

		wrapMode: Text.Wrap
	}



	Item {
		id: bottomItem

		opacity: selectedData ? 1.0 : 0.0
		visible: opacity != 0

		/*anchors.top: titleLabel.visible ? titleLabel.bottom : parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right*/

		anchors.fill: parent

		StackView {
			id: bottomStack
			anchors.fill: parent

			clip: true

			initialItem: emptyComponent
		}

		GridLayout {
			id: grid
			columns: 2

			anchors.centerIn: parent

			QLabel {
				text: qsTr("Szint")
				color: CosStyle.colorPrimary
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}

			QSpinBoxArrow {
				id: spinLevel
				from: selectedData && selectedData.levels ? 0 : -1
				to: selectedData && selectedData.levels ? selectedData.levels.length-1 : 0
				value: -1

				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

				textFromValue: function (value) {
					if (selectedData && selectedData.levels && selectedData.levels.length)
						return qsTr("Level %1").arg(selectedData.levels[value].level)
					else
						return ""
				}

				onValueChanged: {
					if (selectedData && selectedData.levels && value >= 0 && value < selectedData.levels.length) {
						spinMode.from = 0
						spinMode.to = selectedData.levels[value].modes.length-1
						spinMode.value = 0
						bottomStack.replace(levelComponent, { levelData: selectedData.levels[value], modeIndex: spinMode.value })
						labelXP.xp = selectedData.levels[spinLevel.value].modes[spinMode.value].xp
						labelXP.enabled = selectedData.levels[spinLevel.value].modes[spinMode.value].available
						if (basePage) {
							basePage._currentLevel = selectedData.levels[spinLevel.value].level
							basePage._currentDeathmatch = selectedData.levels[spinLevel.value].modes[spinMode.value].mode === "deathmatch"
						}
					} else {
						spinMode.from = -1
						spinMode.to = -1
						spinMode.value = -1
						bottomStack.replace(emptyComponent)
						labelXP.xp = 0
						labelXP.enabled = false
						if (basePage) {
							basePage._currentLevel = -1
							basePage._currentDeathmatch = false
						}
					}

				}
			}

			QLabel {
				text: qsTr("Mód")
				color: CosStyle.colorPrimary

				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}

			QSpinBoxArrow {
				id: spinMode
				from: -1
				to: -1
				value: -1

				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

				textFromValue: function (value) {
					if (selectedData && selectedData.levels && selectedData.levels.length && spinLevel.value > -1)
						return selectedData.levels[spinLevel.value].modes[value].type
					else
						return ""
				}


				onValueChanged: {
					if (value > -1) {
						bottomStack.replace(levelComponent, { levelData: selectedData.levels[spinLevel.value], modeIndex: spinMode.value })
						labelXP.xp = selectedData.levels[spinLevel.value].modes[spinMode.value].xp
						labelXP.enabled = selectedData.levels[spinLevel.value].modes[spinMode.value].available
						if (basePage) {
							basePage._currentLevel = selectedData.levels[spinLevel.value].level
							basePage._currentDeathmatch = selectedData.levels[spinLevel.value].modes[spinMode.value].mode === "deathmatch"
						}
					} else {
						bottomStack.replace(emptyComponent)
						labelXP.xp = 0
						labelXP.enabled = false
						if (basePage) {
							basePage._currentLevel = -1
							basePage._currentDeathmatch = false
						}
					}
				}

			}

			QLabel {
				id: labelXP
				Layout.columnSpan: 2
				Layout.alignment: Qt.AlignCenter

				property int xp: 0

				topPadding: 15
				bottomPadding: 15
				horizontalAlignment: Text.AlignHCenter
				wrapMode: Text.Wrap
				width: parent.width
				text: qsTr("Megszerezhető: <b>%1 XP</b>").arg(xp)
				color: enabled ? CosStyle.colorOKLighter : CosStyle.colorPrimaryDarker
				font.pixelSize: CosStyle.pixelSize*1.2
				opacity: xp > 0 ? 1.0 : 0.0
			}
		}





		Component {
			id: emptyComponent
			Item {}
		}


		Component {
			id: levelComponent

			Flickable {
				id: levelItem

				contentWidth: row2.width
				contentHeight: row2.y+row2.height

				clip: true

				property var levelData: null
				property int modeIndex: -1

				boundsBehavior: Flickable.StopAtBounds
				flickableDirection: Flickable.VerticalFlick

				Row {
					id: row2

					width: levelItem.width

					y: Math.max((levelItem.height-row2.height)/2, 0)

					Item {
						id: img1
						anchors.verticalCenter: parent.verticalCenter
						width: (parent.width-placeholderItem.width)/2
						height: btn.height*2


						QFontImage {
							id: solvedImage

							visible: icon != ""

							icon: modeIndex != -1 ?
									  (levelData.modes[modeIndex].solved ?
										   CosStyle.iconOK :
										   (levelData.modes[modeIndex].available ? "" : CosStyle.iconLock)) :
									  ""

							width: parent.width*0.8
							height: Math.min(parent.height*0.8, 100)

							size: height

							color: icon == CosStyle.iconOK ? CosStyle.colorOKLighter : CosStyle.colorPrimaryDarker
							opacity: icon == CosStyle.iconOK ? 1.0 : 0.7

							anchors.centerIn: parent
						}

						QMedalImage {
							id: medalImage
							level: levelData.level
							isDeathmatch: modeIndex !== -1 && levelData.modes[modeIndex].type === "deathmatch"
							image: selectedData ? selectedData.medalImage : ""

							visible: !solvedImage.visible

							width: parent.width*0.8
							height: Math.min(parent.height*0.8, 100)
							anchors.centerIn: parent
						}

					}


					Item {
						id: placeholderItem
						width: grid.width
						height: grid.height
						anchors.verticalCenter: parent.verticalCenter
					}

					Column {
						id: col2

						z: 2

						anchors.verticalCenter: parent.verticalCenter

						width: (parent.width-placeholderItem.width)/2


						QButton {
							id: btn

							anchors.horizontalCenter: parent.horizontalCenter

							themeColors: CosStyle.buttonThemeGreen
							text: qsTr("Play")
							enabled: levelData && modeIndex != -1 && levelData.modes[modeIndex].available
							icon.source: enabled ? CosStyle.iconPlay : CosStyle.iconLock
							font.pixelSize: CosStyle.pixelSize*1.4
							onClicked: {
								if (studentMaps.gamePage) {
									cosClient.sendMessageError(qsTr("Belső hiba"), qsTr("Már folyamatban van egy játék!"))
								} else {

									studentMaps.playGame({
															 uuid: selectedData.uuid,
															 level: levelData.level,
															 deathmatch: modeIndex != -1 && levelData.modes[modeIndex].type === "deathmatch"
														 })

								}
							}
						}

						Item {
							width: 20
							height: 20
							visible: btn.visible
						}

					}

				}
			}

		}

	}




	onPopulated: spinLevel.forceActiveFocus()
}



