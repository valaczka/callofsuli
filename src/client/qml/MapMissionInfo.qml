import QtQuick 2.15
import QtQuick.Controls 2.15
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
		opacity: 0.2
		fillMode: Image.PreserveAspectCrop
	}

	QLabel {
		id: noLabel
		opacity: selectedData ? 0.0 : 1.0
		visible: opacity != 0

		anchors.centerIn: parent

		text: qsTr("Válassz küldetést")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	Item {
		id: topItem
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.topMargin: 15
		height: Math.min(Math.max(parent.height*0.25, titleLabel.height+detailsLabel.height), parent.height*0.3)

		opacity: selectedData ? 1.0 : 0.0
		visible: opacity != 0

		Column {
			anchors.centerIn: parent
			spacing: 0

			QLabel {
				id: titleLabel

				anchors.horizontalCenter: parent.horizontalCenter
				width: topItem.width*0.8

				opacity: text.length ? 1.0 : 0.0
				visible: opacity != 0

				topPadding: 20
				bottomPadding: 20

				text: selectedData ? selectedData.name : ""

				font.family: "HVD Peace"
				font.pixelSize: CosStyle.pixelSize*1.4

				color: CosStyle.colorAccentLighter

				horizontalAlignment: Text.AlignHCenter

				wrapMode: Text.Wrap
			}

			Flickable {
				id: flick

				boundsBehavior: Flickable.StopAtBounds
				flickableDirection: Flickable.VerticalFlick
				width: topItem.width
				height: Math.min(contentHeight+2, topItem.height-40-titleLabel.height)

				anchors.verticalCenter: item.verticalCentered ? parent.verticalCenter : undefined

				clip: true

				contentWidth: detailsLabel.width
				contentHeight: detailsLabel.height

				QLabel {
					id: detailsLabel

					width: flick.width*0.85
					x: (flick.width-width)/2

					wrapMode: Text.Wrap
					text: selectedData ? selectedData.description : ""

					font.family: "Special Elite"
					color: CosStyle.colorAccent
					font.pixelSize: CosStyle.pixelSize
				}

				ScrollIndicator.vertical: ScrollIndicator { }
			}
		}
	}

	QTabBar {
		id: tabBar
		anchors.top: topItem.bottom
		width: parent.width

		currentIndex: swipe.currentIndex

		opacity: selectedData ? 1.0 : 0.0
		visible: opacity != 0

		Repeater {
			model: selectedData ? selectedData.levels : []

			QTabButton {
				text: qsTr("Level %1").arg(modelData.level)
				icon.source: modelData.available ?
								 (modelData.solved ? CosStyle.iconOK : "") :
								 CosStyle.iconLock
				display: AbstractButton.TextBesideIcon
				font.pixelSize: CosStyle.pixelSize
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

		opacity: selectedData ? 1.0 : 0.0
		visible: opacity != 0

		clip: true

		currentIndex: tabBar.currentIndex

		Repeater {
			model: selectedData ? selectedData.levels : []

			Item {
				id: levelItem

				Column {
					anchors.centerIn: parent

					QLabel {
						anchors.horizontalCenter: parent.horizontalCenter
						text: qsTr("Level %1").arg(modelData.level)
						color: CosStyle.colorAccentLighter
						font.pixelSize: CosStyle.pixelSize*1.5
						font.weight: Font.Light

						bottomPadding: 15
					}

					QLabel {
						anchors.horizontalCenter: parent.horizontalCenter
						text: qsTr("Megoldásra adott idő: <b>%1</b>").arg(JS.secToMMSS(modelData.duration))
						color: CosStyle.colorPrimaryLighter
					}

					QLabel {
						anchors.horizontalCenter: parent.horizontalCenter
						text: qsTr("Célpontok száma: <b>%1</b>").arg(modelData.enemies)
						color: CosStyle.colorPrimaryLighter
					}

					QLabel {
						anchors.horizontalCenter: parent.horizontalCenter
						text: qsTr("HP: <b>%1</b>").arg(modelData.startHP)
						color: CosStyle.colorPrimaryLighter
					}

					QLabel {
						topPadding: 15
						bottomPadding: 15
						anchors.horizontalCenter: parent.horizontalCenter
						text: qsTr("Megszerezhető XP: <b>%1</b>").arg(modelData.xp)
						color: CosStyle.colorAccentLighter
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

							color: CosStyle.colorOKLight
						}

						QLabel {
							anchors.verticalCenter: parent.verticalCenter
							text: qsTr("Megoldva")
							color: CosStyle.colorOKLight
							font.weight: Font.Medium
						}
					}

					Item {
						width: 20
						height: 20
						visible: btn.visible
					}

					QButton {
						id: btn

						anchors.horizontalCenter: parent.horizontalCenter

						text: qsTr("Start")
						icon.source: CosStyle.iconPlay
						visible: modelData.available
						onClicked: {
							studentMaps.playGame({
													 uuid: selectedData.uuid,
													 level: modelData.level,
													 hasSolved: modelData.solved
												 })
						}
					}

				}
			}
		}
	}

}



