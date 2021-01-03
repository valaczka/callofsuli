import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitWidth: 500
	implicitHeight: 200

	anchors.fill: parent


	property var questionData: null

	signal succeed()
	signal failed()

	property color borderColor: CosStyle.colorPrimaryDarker
	property color titleColor: CosStyle.colorAccentLighter

	property int horizontalPadding: 20
	property int verticalPadding: 10

	property int maximumWidth: 0
	property int maximumHeight: 0

	Audio {
		id: openSound
		volume: CosStyle.volumeSfx
		source: "qrc:/sound/sfx/question.ogg"
		autoPlay: true
	}


	Item {
		id: panel

		width: control.width < 500 ? control.width-18 : (maximumWidth ? Math.min(maximumWidth, control.width-2*horizontalPadding) : control.width-2*horizontalPadding)
		height: control.width < 500 ? control.height-18 : (maximumHeight ? Math.min(maximumHeight, control.height-2*verticalPadding) : control.height-2*verticalPadding)
		x: (control.width-width)/2
		y: (control.height-height)/2


		DropShadow {
			anchors.fill: panel
			horizontalOffset: 3
			verticalOffset: 3
			color: JS.setColorAlpha("black", 0.75)
			source: border2
		}

		BorderImage {
			id: border2
			source: "qrc:/internal/img/border2.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			anchors.fill: panel
			border.top: 10
			border.left: 5
			border.right: 80
			border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}


		Image {
			id: metalbg
			source: "qrc:/internal/img/metalbg.png"
			visible: false
			fillMode: Image.Tile
			anchors.fill: panel
		}

		OpacityMask {
			id: opacity1
			anchors.fill: panel
			source: metalbg
			maskSource: border2
		}


		Item {
			id: realContent
			anchors.fill: parent
			visible: true

			anchors.topMargin: 5
			anchors.leftMargin: 0
			anchors.rightMargin: 0
			anchors.bottomMargin: 10



			/*Row {
				anchors.centerIn: parent
				spacing: 10

				QButton {
					text: "HELYES"
					onClicked: control.succeed()
				}

				QButton {
					text: "HELYTELEN"
					onClicked: control.failed()
				}
			}*/

			Loader {
				id: contentLoader
				anchors.fill: parent
			}

			Connections {
				target: contentLoader.item

				function onSucceed() {
					succeed()
				}

				function onFailed() {
					failed()
				}
			}

		}





		// BORDER

		BorderImage {
			id: border1
			source: "qrc:/internal/img/border1.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			anchors.fill: panel
			border.top: 15
			border.left: 10
			border.right: 60
			border.bottom: 25

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}

		ColorOverlay {
			anchors.fill: border1
			source: border1
			color: borderColor
		}

	}

	onQuestionDataChanged: {
		if (questionData && questionData.module.length) {
			contentLoader.setSource("GameQuestion_"+questionData.module+".qml", {
										questionData: questionData
									})
		}
	}
}
