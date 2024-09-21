import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

ListView {
	id: root

	interactive: false

	clip: false

	height: contentHeight

	Behavior on height {
		NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
	}

	model: ListModel {
		id: _model
	}

	delegate: Item {
		id: _item

		width: root.width
		height: _label.height*_rect.scale+3

		required property string textColor
		required property string message
		required property int index

		Rectangle {
			id: _rect
			color: "#AACCCCCC"
			radius: 5

			width: _label.width
			height: _label.height

			anchors.centerIn: parent

			Qaterial.LabelBody2 {
				id: _label

				width: Math.min(implicitWidth, _item.width/1.5)
				anchors.centerIn: parent

				leftPadding: 8
				rightPadding: 8
				topPadding: 2
				bottomPadding: 2

				color: _item.textColor

				text: _item.message
				elide: Text.ElideRight
				style: Text.Outline
				styleColor: Qaterial.Colors.black
			}
		}

		SequentialAnimation {
			id: _baseAnimation
			running: true

			PropertyAnimation {
				target: _rect
				property: "scale"
				from: 1.0
				to: 1.5
				duration: 250
				easing.type: Easing.OutQuart
			}

			PauseAnimation {
				duration: 1250
			}

			ParallelAnimation {
				PropertyAnimation {
					target: _rect
					property: "scale"
					to: 1.0
					duration: 750
					easing.type: Easing.InQuart
				}


				ColorAnimation {
					target: _rect
					property: "color"
					to: "#44CCCCCC"
					duration: 750
				}
			}

			PauseAnimation {
				duration: 1250
			}

			ScriptAction {
				script: _model.remove(_item.index)
			}
		}



		SequentialAnimation {
			id: _clickAnimation
			running: false

			PropertyAnimation {
				target: _rect
				property: "scale"
				to: 0.0
				duration: 250
				easing.type: Easing.OutQuart
			}

			ScriptAction {
				script: _model.remove(_item.index)
			}
		}

		MouseArea {
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton
			onClicked: {
				_baseAnimation.stop()
				_clickAnimation.start()
			}
		}



	}

	remove: Transition {
		NumberAnimation { property: "opacity"; to: 0; duration: 250}
	}

	removeDisplaced: Transition {
		NumberAnimation { properties: "x,y"; duration: 600 }
	}


	function message(text, colorCode) {
		_model.append({
						  message: text,
						  textColor: String(colorCode)
					  })
	}
}
