import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.14
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	layoutFillWidth: true

	property int selectedMissionIndex: -1
	property var selectedData: null

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
		panel.title = ""
		selectedData = null
		spinLevel.value = -1

		if (selectedMissionIndex == -1) {
			if (stackMode && pageStack)
				pageStack.pop()
		} else {
			var x = studentMaps.modelMissionList.get(selectedMissionIndex)
			if (Object.keys(x).length) {
				panel.title = x.name
				selectedData = x
				spinLevel.value = 0
				spinMode.value = 0
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

		topPadding: 50
		bottomPadding: 50
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

		anchors.top: titleLabel.visible ? titleLabel.bottom : parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right


		GridLayout {
			id: bottomCol
			columns: 2

			anchors.top: parent.top
			anchors.horizontalCenter: parent.horizontalCenter

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
					} else {
						spinMode.from = -1
						spinMode.to = -1
						spinMode.value = -1
						bottomStack.replace(emptyComponent)
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
					if (value > -1)
						bottomStack.replace(levelComponent, { levelData: selectedData.levels[spinLevel.value], modeIndex: spinMode.value })
					else
						bottomStack.replace(emptyComponent)
				}

			}
		}


		StackView {
			id: bottomStack
			anchors.left: parent.left
			anchors.top: bottomCol.bottom
			anchors.right: parent.right
			anchors.bottom: parent.bottom

			clip: true

			initialItem: emptyComponent
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

					width: bottomStack.width

					y: Math.max((levelItem.height-row2.height)/2, 0)

					Item {
						id: img1
						anchors.verticalCenter: parent.verticalCenter
						width: levelItem.width*0.25
						height: col2.height*0.9

						opacity: modeIndex != -1 && levelData.modes[modeIndex].available && levelData.solvedNormal ? 1.0 : 0.2



						QMedalImage {
							id: medalImage
							level: levelData.level
							isDeathmatch: modeIndex !== -1 && levelData.modes[modeIndex].type === "deathmatch"
							image: selectedData ? selectedData.medalImage : ""

							visible: !studentMaps.demoMode && modeIndex != -1 && levelData.modes[modeIndex].available

							//opacity: !levelData.solved ? 1.0 : 0.2

							width: parent.width*0.8
							height: Math.min(parent.height*0.8, 100)
							anchors.centerIn: parent
						}

					}

					Column {
						id: col2

						z: 2

						anchors.verticalCenter: parent.verticalCenter

						width: row2.width-img1.width-img2.width


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

									/*var d = JS.dialogCreateQml("ImageList", {
														   roles: ["name", "dir"],
														   icon: CosStyle.iconUser,
														   title: qsTr("Válassz karaktert"),
														   selectorSet: false,
														   delegateHeight: 80,
														   modelImageHeight: 50,
														   modelImageWidth: 100,
														   modelImageRole: "dir",
														   modelImagePattern: "qrc:/character/%1/thumbnail.png",
														   sourceModel: studentMaps.modelCharacterList
													   })

							d.accepted.connect(function(data) {
								if (data === -1)
									return

								var p = d.item.sourceModel.get(data)
*/
									studentMaps.playGame({
															 uuid: selectedData.uuid,
															 level: levelData.level,
															 deathmatch: modeIndex != -1 && levelData.modes[modeIndex].type === "deathmatch"
														 })
									/*									})
							d.open()
*/
								}
							}
						}

						Item {
							width: 20
							height: 20
							visible: btn.visible
						}



						QLabel {
							topPadding: 15
							bottomPadding: 15
							anchors.horizontalCenter: parent.horizontalCenter
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							width: parent.width
							text: levelData && modeIndex != -1 ? qsTr("Megszerezhető: <b>%1 XP</b>").arg(levelData.modes[modeIndex].xp) : ""
							color: btn.enabled ? CosStyle.colorOKLighter : CosStyle.colorPrimaryDarker
							font.pixelSize: CosStyle.pixelSize*1.2
						}

					}

					Item {
						id: img2
						anchors.verticalCenter: parent.verticalCenter
						width: levelItem.width*0.25
						height: col2.height*0.9

						//opacity: modeIndex != -1 && levelData.modes[modeIndex].available ? 1.0 : 0.2

						QImageInnerShadow {
							width: trophyImage.width
							height: trophyImage.height

							anchors.centerIn: parent

							image: "qrc:/internal/trophy/trophyt1.png"
							contentItem: panel.metalBgTexture

							brightness: -0.5

							visible: !studentMaps.demoMode && modeIndex != -1 && !levelData.modes[modeIndex].available
						}

						QTrophyImage {
							id: trophyImage
							level: levelData.level
							isDeathmatch: modeIndex !== -1 && levelData.modes[modeIndex].type === "deathmatch"

							visible: !studentMaps.demoMode && modeIndex != -1 && levelData.modes[modeIndex].available

							width: parent.width*0.8
							height: Math.min(parent.height*0.8, 75)
							anchors.centerIn: parent
						}
					}
				}
			}

		}

	}


	onPopulated: spinLevel.forceActiveFocus()
}



