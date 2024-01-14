import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtMultimedia
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

QPage {
	id: control

	title: qsTr("QR-kód beolvasás")

	appBar.backButtonVisible: true

	property bool captureEnabled: true

	signal tagFound(string tag)

	/*Camera
	{
		id: camera

		active: true
		focusMode: Camera.FocusModeAutoNear
	}*/

	/*CaptureSession {
		camera: camera
		videoOutput: videoOutput

		imageCapture: ImageCapture {
			id: imageCapture
		}
	}*/

	VideoOutput
	{
		id: videoOutput

		anchors.fill: parent

		property double captureRectStartFactorX: 0.1
		property double captureRectStartFactorY: 0.1
		property double captureRectFactorWidth: 0.8
		property double captureRectFactorHeight: 0.8


		focus: visible
		fillMode: VideoOutput.PreserveAspectCrop

		Rectangle {
			id: blackRect
			color: Qaterial.Colors.black
			anchors.fill: parent
			visible: false
		}

		Item {
			id: captureRect
			anchors.fill: parent
			visible: false

			Rectangle {
				color: Qaterial.Colors.white
				width: parent.width * videoOutput.captureRectFactorWidth
				height: parent.height * videoOutput.captureRectFactorHeight
				x: parent.width * videoOutput.captureRectStartFactorX
				y: parent.height * videoOutput.captureRectStartFactorY
			}
		}

		OpacityMask {
			anchors.fill: blackRect
			source: blackRect
			maskSource: captureRect
			invert: true
			opacity: 0.5
		}

		/*Component.onCompleted: {
			camera.active = false
			camera.active = true
			scanner.setProcessing(true)
		}*/
	}

	SBarcodeScanner {
		id: scanner

		videoSink: videoOutput.videoSink
		captureRect: Qt.rect(videoOutput.width * videoOutput.captureRectStartFactorX,
							videoOutput.height * videoOutput.captureRectStartFactorY,
							videoOutput.width * videoOutput.captureRectFactorWidth,
							videoOutput.height * videoOutput.captureRectFactorHeight)

		onCaptureRectChanged: console.debug("-------", scanner.captureRect)



		onCameraAvailableChanged: console.debug("***", cameraAvailable)
		onErrorDescriptionChanged: console.error("!!!", errorDescription)

		onCapturedChanged: {
							   //if (!captureEnabled)
							   //return

							   console.debug("QR: "+captured);

							   tagFound(captured)

							   //imageCapture.capture()
						   }

	}

	Image {
		anchors.left: parent.left
		anchors.bottom: parent.bottom
		width: parent.width*0.25
		height: parent.height*0.25
		//source: imageCapture.preview
	}

	Component.onCompleted: {
		console.debug("1***", scanner.cameraAvailable)
		console.error("1!!!", scanner.errorDescription)
	}

}
