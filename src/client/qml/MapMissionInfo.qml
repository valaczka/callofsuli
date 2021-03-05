import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
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
			if (selectedMissionIndex == -1)
				selectedData = null
			else {
				if (swipeMode)
					parent.parentPage.swipeToPage(1)

				var x = studentMaps.modelMissionList.get(selectedMissionIndex)
				if (Object.keys(x).length)
					selectedData = x
				else
					selectedData = null
			}
		}

		function onMissionListChanged() {
			if (selectedMissionIndex == -1)
				selectedData = null
			else {
				var x = studentMaps.modelMissionList.get(selectedMissionIndex)
				if (Object.keys(x).length)
					selectedData = x
				else {
					selectedData = null
					selectedMissionIndex = -1
				}
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
		columns: (panel.parent.width >= panel.parent.height) ? 2 : 1
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
					width: topItem.width*0.75

					wrapMode: Text.Wrap
					text: selectedData ? selectedData.description : ""

					font.family: "Special Elite"
					color: CosStyle.colorAccent
					font.pixelSize: CosStyle.pixelSize
				}
			}
		}


		Item {
			id: bottomItem

			width: (grid.columns > 1) ? grid.width/2 : grid.width
			height: (grid.columns > 1) ? grid.height : grid.height-topItem.height

			QTabBar {
				id: tabBar
				anchors.top: parent.top
				width: parent.width

				currentIndex: swipe.currentIndex

				Repeater {
					model: selectedData ? selectedData.levels : []

					QTabButton {
						text: qsTr("Level %1").arg(modelData.level)
						icon.source: modelData.available ?
										 (modelData.solved ? CosStyle.iconOK : "") :
										 CosStyle.iconLock
						display: AbstractButton.TextBesideIcon
						font.pixelSize: CosStyle.pixelSize
						font.capitalization: Font.MixedCase
						iconColor: modelData.available ?
									   (modelData.solved ? CosStyle.colorOK : CosStyle.colorPrimary) :
									   CosStyle.colorPrimaryDark
					}
				}
			}


			SwipeView {
				id: swipe

				anchors.left: parent.left
				anchors.right: parent.right
				anchors.top: tabBar.bottom
				anchors.bottom: parent.bottom

				clip: true

				currentIndex: tabBar.currentIndex

				Repeater {
					model: selectedData ? selectedData.levels : []

					Flickable {
						id: levelItem

						contentWidth: col2.width
						contentHeight: col2.y+col2.height

						clip: true

						boundsBehavior: Flickable.StopAtBounds
						flickableDirection: Flickable.VerticalFlick

						Column {
							id: col2

							width: levelItem.width

							y: Math.max((levelItem.height-col2.height)/2, 0)

							Item {
								width: 20
								height: 20
								visible: btn.visible
							}

							QButton {
								id: btn

								anchors.horizontalCenter: parent.horizontalCenter

								themeColors: CosStyle.buttonThemeGreen
								text: qsTr("Play level %1").arg(modelData.level)
								icon.source: CosStyle.iconPlay
								visible: modelData.available
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
																 level: modelData.level,
																 hasSolved: modelData.solved,
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
								text: qsTr("Rendelkezésre álló idő: <b>%1</b>").arg(JS.secToMMSS(modelData.duration))
								color: modelData.available ? CosStyle.colorPrimaryLighter : CosStyle.colorPrimaryDarker
							}

							QLabel {
								anchors.horizontalCenter: parent.horizontalCenter
								horizontalAlignment: Text.AlignHCenter
								wrapMode: Text.Wrap
								width: levelItem.width*0.9
								text: qsTr("Célpontok száma: <b>%1</b>").arg(modelData.enemies)
								color: modelData.available ? CosStyle.colorPrimaryLighter : CosStyle.colorPrimaryDarker
							}

							QLabel {
								anchors.horizontalCenter: parent.horizontalCenter
								horizontalAlignment: Text.AlignHCenter
								wrapMode: Text.Wrap
								width: levelItem.width*0.9
								text: qsTr("HP: <b>%1</b>").arg(modelData.startHP)
								color: modelData.available ? CosStyle.colorPrimaryLighter : CosStyle.colorPrimaryDarker
							}

							QLabel {
								topPadding: 15
								bottomPadding: 15
								anchors.horizontalCenter: parent.horizontalCenter
								horizontalAlignment: Text.AlignHCenter
								wrapMode: Text.Wrap
								width: levelItem.width*0.9
								text: qsTr("Megszerezhető: <b>%1 XP</b>").arg(modelData.xp)
								color: modelData.available ? CosStyle.colorOKLighter : CosStyle.colorPrimaryDarker
								font.pixelSize: CosStyle.pixelSize*1.2
							}

							Row {
								visible: modelData.solved
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
	}
}



