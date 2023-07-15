import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionImpl {
	id: control

	implicitWidth: 200
	implicitHeight: 200

	width: implicitWidth
	height: implicitHeight

	property alias panel: panel
	property alias realContent: realContent
	property alias contentLoader: contentLoader

	property color borderColor: Qaterial.Colors.cyan700
	property color titleColor: Qaterial.Colors.amber200

	loader: contentLoader

	focus: true

	layer.enabled: true


	//-----------------------------------
	// Required states: "started", "finished"
	// Required method calls: onShowAnimationFinished(), onHideAnimationFinished()
	//-----------------------------------

	Item {
		id: panel

		width: control.width
		height: control.height

		opacity: 0.1
		scale: 0.1

		DropShadow {
			anchors.fill: panel
			horizontalOffset: 4
			verticalOffset: 4
			color: Client.Utils.colorSetAlpha("black", 0.4)
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


		FocusScope {
			id: realContent
			anchors.fill: parent
			visible: true

			anchors.topMargin: 5
			anchors.leftMargin: 0
			anchors.rightMargin: 0
			anchors.bottomMargin: 10

			opacity: 0.0

			Loader {
				id: contentLoader
				anchors.fill: parent
				asynchronous: true

				onLoaded: control.onLoaderLoaded(item)

				onStatusChanged: if (contentLoader.status == Loader.Error) {
									 Client.messageError(qsTr("Nem lehet betölteni a kérdést!"), qsTr("Belső hiba"))
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

}
