import QtQuick
import QtQuick.Controls
import QtQuick.VectorImage
import Qt.labs.animation
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Flickable {
	id: _flick

	property RpgUserWorld world: null
	property RpgWorldLandData selectedLand: null

	contentWidth: _container.width
	contentHeight: _container.height

	boundsBehavior: Flickable.StopAtBounds
	flickableDirection: Flickable.HorizontalAndVerticalFlick

	signal landSelected(RpgWorldLandData land)

	Item {
		id: _container
		width: Math.max(_scene.width * _scene.scale, _flick.width)
		height: Math.max(_scene.height * _scene.scale, _flick.height)

		Item {
			id: _scene

			width: world ? world.worldSize.width : implicitWidth
			height: world ? world.worldSize.height : implicitHeight

			anchors.centerIn: parent

			VectorImage {
				source: world ? world.imageBackground : ""
				fillMode: VectorImage.NoResize
			}

			Repeater {
				model: world ? world.landList : null

				delegate: RpgUserWorldLand {
					id: _land
					landData: model.qtObject
					selected: landData && _flick.selectedLand == landData
					onClicked: _flick.selectedLand = landData
					onDoubleClicked: {
						_flick.landSelected(landData)
					}
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

			VectorImage {
				source: world ? world.imageOver : ""
				fillMode: VectorImage.NoResize
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
			persistentTranslation: Qt.point(0,0)
			persistentRotation: 0

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

	Component.onCompleted: {
		if (world)
			selectedLand = world.selectedLand
	}

	onWorldChanged: {
		if (world)
			selectedLand = world.selectedLand

		fitZoom()
	}
}

