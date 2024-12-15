import QtQuick
import QtQuick.Controls
//import QtQuick.VectorImage
import Qt.labs.animation
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Flickable {
	id: _flick

	property RpgUserWorld world: null
	property RpgWorldLandData selectedLand: null
	property bool _loaded: false

	anchors.fill: parent

	contentWidth: _container.width
	contentHeight: _container.height

	boundsBehavior: Flickable.StopAtBounds
	flickableDirection: Flickable.HorizontalAndVerticalFlick

	signal landSelected(RpgWorldLandData land)

	QFetchLoaderGroup {
		id: _loaderGroup
		onAllLoadersLoaded: {
			_loaded = true
			Qaterial.DialogManager.closeBusyIndicator()
		}
	}

	Item {
		id: _container
		width: Math.max(_scene.width * _scene.scale, _flick.width)
		height: Math.max(_scene.height * _scene.scale, _flick.height)

		enabled: !_loaderGroup.showPlaceholders

		Item {
			id: _scene

			width: world ? world.worldSize.width : implicitWidth
			height: world ? world.worldSize.height : implicitHeight

			anchors.centerIn: parent

			/*VectorImage {
				source: world ? world.imageBackground : ""
				fillMode: VectorImage.NoResize
			}*/
			Image {
				fillMode: Image.PreserveAspectFit
				visible: false
				source: world ? world.imageBackground : ""
				width: sourceSize.width
				height: sourceSize.height
				asynchronous: true
				cache: true
			}

			Repeater {
				model: world ? world.landList : null

				delegate: Loader {
					id: _loader
					asynchronous: true

					sourceComponent: RpgUserWorldLand {
						id: _land
						landData: model.qtObject
						selected: landData && _flick.selectedLand == landData
						onClicked: _flick.selectedLand = landData
						onDoubleClicked: {
							_flick.landSelected(landData)
						}
					}

					Component.onCompleted: _loaderGroup.add(_loader)
					onLoaded: _loaderGroup.remove(_loader)
				}

			}

			Repeater {
				model: world ? world.landList : null

				delegate: RpgUserWorldLandOver {
					id: _landOver
					landData: model.qtObject
					selected: landData && _flick.selectedLand == landData
				}
			}

			/*VectorImage {
				source: world ? world.imageOver : ""
				fillMode: VectorImage.NoResize
			}*/

			Image {
				fillMode: Image.PreserveAspectFit
				visible: false
				source: world ? world.imageOver : ""
				width: sourceSize.width
				height: sourceSize.height
				asynchronous: true
				cache: true
			}

			BoundaryRule on scale {
				minimum: 0.3
				maximum: 5.0
				minimumOvershoot: 0.05
				maximumOvershoot: 0.05
			}

		}

		PinchHandler {
			target: _scene
			rotationAxis.enabled: false
			xAxis.enabled: false
			yAxis.enabled: false
			scaleAxis.enabled: true
		}

		WheelHandler {
			target: _scene
			acceptedModifiers: Qt.ControlModifier
			property: "scale"
			acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
		}
	}


	onWidthChanged: fitZoom()
	onHeightChanged: fitZoom()

	function fitZoom() {
		if (!world || world.worldSize.width == 0 || world.worldSize.height == 0)
			return

		_scene.scale = Math.min(1.0, _flick.width/world.worldSize.width, _flick.height/world.worldSize.height)

	}

	function updateSelectedLand() {
		if (world)
			selectedLand = world.selectedLand
	}

	onWorldChanged: {
		updateSelectedLand()
		fitZoom()
	}
}

