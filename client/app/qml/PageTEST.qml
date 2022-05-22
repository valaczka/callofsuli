import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import QtGraphicalEffects 1.0
import QtMultimedia 5.15
import QZXing 3.3



QTabPage {
	id: control

	title: "Test page"
	backFunction: null


	Component {
		id: cmp1
		QTabContainer {
			id: container
			title:  "Test"

			property int detectedTags: 0
			property string lastTag: ""


			Rectangle
			{
				id: bgRect
				color: "white"
				anchors.fill: videoOutput
			}

			Text
			{
				id: text1
				wrapMode: Text.Wrap
				font.pixelSize: 20
				anchors.top: parent.top
				anchors.left: parent.left
				z: 50
				text: "Tags detected: " + detectedTags
			}
			Text
			{
				id: fps
				font.pixelSize: 20
				anchors.top: parent.top
				anchors.right: parent.right
				z: 50
				text: (1000 / zxingFilter.timePerFrameDecode).toFixed(0) + "fps"
			}

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
				anchors.top: text1.bottom
				anchors.bottom: text2.top
				anchors.left: parent.left
				anchors.right: parent.right
				autoOrientation: true
				fillMode: VideoOutput.Stretch
				filters: [ zxingFilter ]

				property double captureRectStartFactorX: 0.2
				property double captureRectStartFactorY: 0.2
				property double captureRectFactorWidth: 0.6
				property double captureRectFactorHeight: 0.6


				Rectangle {
					id: blackRect
					color: "black"
					anchors.fill: parent
					visible: false
				}

				Item {
					id: captureRect
					anchors.fill: parent
					visible: false

					Rectangle {
						color: "white"
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
					console.debug("RRR", r)
					return r;
				}

				decoder {
					enabledDecoders: QZXing.DecoderFormat_QR_CODE

					onTagFound: {
						console.log(tag + " | " + decoder.charSet());

						container.detectedTags++;
						container.lastTag = tag;
					}

					tryHarder: true
				}


				onDecodingStarted:
				{
					//console.log("started");
				}

				property int framesDecoded: 0
				property real timePerFrameDecode: 0

				onDecodingFinished:
				{
					timePerFrameDecode = (decodeTime + framesDecoded * timePerFrameDecode) / (framesDecoded + 1);
					framesDecoded++;
					if(succeeded)
						console.log("frame finished: " + succeeded, decodeTime, timePerFrameDecode, framesDecoded);
				}
			}

			Text
			{
				id: text2
				wrapMode: Text.Wrap
				font.pixelSize: 20
				anchors.bottom: parent.bottom
				anchors.left: parent.left
				z: 50
				text: "Last tag: " + lastTag
			}


		}
	}


	onPageActivatedFirst: cosClient.checkMediaPermissions()


	Connections {
		target: cosClient

		function onMediaPermissionsDenied() {
			console.debug("MEDIA DENIED")
		}

		function onMediaPermissionsGranted() {
			console.debug("MEDIA GRANTED")
			pushContent(cmp1)
		}

	}

}
