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

	Grid {
		id: grid
		anchors.fill: parent
		columns: (panel.width >= panel.height) ? 2 : 1
		spacing: 0

		opacity: selectedData ? 1.0 : 0.0
		visible: opacity != 0


		Flickable {
			id: topItem

			width: (grid.columns > 1) ? grid.width/2 : grid.width
			height: (grid.columns > 1) ? grid.height : grid.height*0.4

			contentWidth: col.width
			contentHeight: col.y+col.height

			clip: true

			flickableDirection: Flickable.VerticalFlick
			boundsBehavior: Flickable.StopAtBounds

			ScrollIndicator.vertical: ScrollIndicator { }

			Column {
				id: col
				width: topItem.width

				y: Math.max((topItem.height-col.height)/2, 0)

				QLabel {
					id: titleLabel

					anchors.horizontalCenter: parent.horizontalCenter
					width: topItem.width*0.85

					opacity: text.length ? 1.0 : 0.0
					visible: opacity != 0

					topPadding: 20
					bottomPadding: 20

					text: selectedData ? selectedData.name : ""

					font.family: "HVD Peace"
					font.pixelSize: CosStyle.pixelSize*1.6

					color: CosStyle.colorAccentLighter

					horizontalAlignment: Text.AlignHCenter

					wrapMode: Text.Wrap
				}


				QLabel {
					id: detailsLabel

					anchors.horizontalCenter: parent.horizontalCenter
					width: topItem.width*0.85

					wrapMode: Text.Wrap
					text: selectedData ? selectedData.description : ""

					font.family: "Special Elite"
					color: CosStyle.colorAccent
					font.pixelSize: CosStyle.pixelSize*0.95
				}
			}
		}


		Item {
			id: bottomItem

			width: (grid.columns > 1) ? grid.width/2 : grid.width
			height: (grid.columns > 1) ? grid.height : grid.height-topItem.height

			GridLayout {
				id: bottomCol
				columns: 2

				anchors.top: parent.top
				anchors.topMargin: (grid.columns > 1) ? 10 : 0
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

					contentWidth: col2.width
					contentHeight: col2.y+col2.height

					clip: true

					property var levelData: null
					property int modeIndex: -1

					boundsBehavior: Flickable.StopAtBounds
					flickableDirection: Flickable.VerticalFlick

					Column {
						id: col2

						width: levelItem.width

						y: Math.max((levelItem.height-col2.height)/2, 0)

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
															 hasSolved: levelData.solved,
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
							anchors.horizontalCenter: parent.horizontalCenter
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							width: levelItem.width*0.9
							text: qsTr("Rendelkezésre álló idő: <b>%1</b>").arg(JS.secToMMSS(levelData.duration))
							color: levelData.available ? CosStyle.colorPrimaryLighter : CosStyle.colorPrimaryDarker
						}

						QLabel {
							anchors.horizontalCenter: parent.horizontalCenter
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							width: levelItem.width*0.9
							text: qsTr("Célpontok száma: <b>%1</b>").arg(levelData.enemies)
							color: levelData.available ? CosStyle.colorPrimaryLighter : CosStyle.colorPrimaryDarker
						}

						QLabel {
							anchors.horizontalCenter: parent.horizontalCenter
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							width: levelItem.width*0.9
							text: qsTr("HP: <b>%1</b>").arg(levelData.startHP)
							color: levelData.available ? CosStyle.colorPrimaryLighter : CosStyle.colorPrimaryDarker
						}

						QLabel {
							topPadding: 15
							bottomPadding: 15
							anchors.horizontalCenter: parent.horizontalCenter
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							width: levelItem.width*0.9
							text: levelData && modeIndex != -1 ? qsTr("Megszerezhető: <b>%1 XP</b>").arg(levelData.modes[modeIndex].xp) : ""
							color: levelData.available ? CosStyle.colorOKLighter : CosStyle.colorPrimaryDarker
							font.pixelSize: CosStyle.pixelSize*1.2
						}

						Row {
							visible: levelData.solved
							anchors.horizontalCenter: parent.horizontalCenter

							spacing: 5

							QFontImage {
								anchors.verticalCenter: parent.verticalCenter

								height: CosStyle.pixelSize*2
								width: height
								size: CosStyle.pixelSize*0.9

								icon: CosStyle.iconOK

								color: CosStyle.colorOKLighter
							}

							QLabel {
								anchors.verticalCenter: parent.verticalCenter
								text: qsTr("Megoldva")
								color: CosStyle.colorOK
								font.weight: Font.Medium
							}
						}

					}
				}

			}

		}
	}

	onPopulated: spinLevel.forceActiveFocus()
}



