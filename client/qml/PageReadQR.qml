import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtMultimedia 5.15
import QZXing 3.3
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

QPage {
	id: control

	title: qsTr("QR-kód beolvasás")

	appBar.backButtonVisible: true

	property bool captureEnabled: true

	signal tagFound(string tag)

	Camera
	{
		id:camera

		focus {
			focusMode: CameraFocus.FocusContinuous
			focusPointMode: CameraFocus.FocusPointCenter
		}
	}

	VideoOutput
	{
		id: videoOutput
		source: camera
		/*anchors.top: hdr.bottom
		   anchors.bottom: parent.bottom
		   anchors.left: parent.left
		   anchors.right: parent.right*/
		anchors.fill: parent
		autoOrientation: true
		fillMode: VideoOutput.Stretch
		filters: [ zxingFilter ]

		property double captureRectStartFactorX: 0.1
		property double captureRectStartFactorY: 0.1
		property double captureRectFactorWidth: 0.8
		property double captureRectFactorHeight: 0.8


		focus: visible
		//fillMode: VideoOutput.PreserveAspectCrop

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
	}

	QZXingFilter
	{
		id: zxingFilter
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
