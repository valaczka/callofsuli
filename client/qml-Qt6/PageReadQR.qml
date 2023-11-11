import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtMultimedia
import QZXing
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

QPage {
	id: control

	title: qsTr("QR-kód beolvasás")

	appBar.backButtonVisible: true

	property bool captureEnabled: true

	signal tagFound(string tag)

	Camera
	{
		id: camera

		active: true
		focusMode: Camera.FocusModeAutoNear
	}

	CaptureSession {
		camera: camera
		videoOutput: videoOutput
	}

	VideoOutput
	{
		id: videoOutput

		anchors.fill: parent

		property double captureRectStartFactorX: 0.1
		property double captureRectStartFactorY: 0.1
		property double captureRectFactorWidth: 0.8
		property double captureRectFactorHeight: 0.8


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

		Component.onCompleted: { camera.active = false; camera.active = true; }
	}

	QZXingFilter
	{
		id: zxingFilter

		videoSink: videoOutput.videoSink
		orientation: videoOutput.orientation
		captureRect: {
			videoOutput.sourceRect;
			var r = Qt.rect(videoOutput.sourceRect.width * videoOutput.captureRectStartFactorX,
							videoOutput.sourceRect.height * videoOutput.captureRectStartFactorY,
							videoOutput.sourceRect.width * videoOutput.captureRectFactorWidth,
							videoOutput.sourceRect.height * videoOutput.captureRectFactorHeight)
			return r;
		}

		decoder {
			enabledDecoders: QZXing.DecoderFormat_QR_CODE

			onTagFound: {
				if (!captureEnabled)
					return

				console.debug("QR: "+tag);

				tagFound(tag)
			}

			tryHarder: false
		}
	}

}
