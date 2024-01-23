import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import Qt.labs.animation
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Rectangle {
	id: root

	property real mapScale: width > height ? Math.min(1.0, width/2600) : Math.min(1.0, height/1200)
	property ConquestGame game: null

	color: Qaterial.Colors.black

	Flickable {
		id: _flick
		contentWidth: Math.max(_scene.width * _scene.scale, width)
		contentHeight: Math.max(_scene.height * _scene.scale, height)

		anchors.fill: parent

		boundsBehavior: Flickable.DragAndOvershootBounds
		flickableDirection: Flickable.HorizontalAndVerticalFlick

		Item {
			id: _scene

			onScaleChanged: {
				let w = _flick.width - width*scale
				let h = _flick.height - height*scale

				if (w>0)
					x = w/2

				if (h>0)
					y = h/2
			}


			width: game ? game.worldSize.width : implicitWidth
			height: game ? game.worldSize.height : implicitHeight

			transformOrigin: Item.TopLeft

			Image {
				source: game && game.config.world.name != "" ? "qrc:/conquest/"+game.config.world.name+"/bg.png" : ""
				anchors.fill: parent
				fillMode: Image.PreserveAspectFit
			}

			Repeater {
				model: game ? game.landDataList : null

				delegate: ConquestLand {
					landData: model.qtObject
				}
			}


			Image {
				source: game && game.config.world.name != "" ? "qrc:/conquest/"+game.config.world.name+"/over.png" : ""
				anchors.fill: parent
				fillMode: Image.PreserveAspectFit
			}

			/*Desaturate {
				id: gameSaturate

				anchors.fill: gameScene
				source: gameScene

				opacity: 0.0
				visible: desaturation

				desaturation: 1.0

				Behavior on opacity { NumberAnimation { duration: 750 } }
			}*/


			transform: Rotation {
				id: _rtr

				axis.x: 1
				axis.y: 0
				axis.z: 0
				origin.x: 0//_scene.width/2
				origin.y: 0//_scene.height/2
			}

			BoundaryRule on scale {
				minimum: 0.3
				maximum: 5.0
			}
		}
	}


	PinchArea {
		anchors.fill: parent

		MouseArea {								// Workaround (https://bugreports.qt.io/browse/QTBUG-77629)
			anchors.fill: parent

			acceptedButtons: Qt.NoButton

			onWheel: wheel => {
						 if (wheel.modifiers & Qt.ControlModifier) {
							 if (wheel.angleDelta.y > 0)
							 _scene.scale += 0.1
							 else
							 _scene.scale -= 0.1

							 wheel.accepted=true
						 } else if (wheel.modifiers & Qt.ShiftModifier) {
							 if (wheel.angleDelta.y > 0)
							 ++_rtr.angle
							 else
							 --_rtr.angle

							 wheel.accepted=true
						 }
					 }
		}

		onPinchUpdated: pinch => {
							_scene.scale *= pinch
						}
	}


}
