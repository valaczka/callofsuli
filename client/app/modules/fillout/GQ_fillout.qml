import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Item {
	id: control

	implicitWidth: 650
	implicitHeight: 500

	required property var questionData
	required property var answerData

	signal succeed()
	signal failed()


	property var _drops: []
	property bool _dragInteractive: true

	QLabel {
		id: labelQuestion

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize*1.4
		wrapMode: Text.Wrap
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: btnOk.left
		height: Math.max(implicitHeight, btnOk.height)
		topPadding: 30
		bottomPadding: 30
		leftPadding: 20
		rightPadding: 20

		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter

		color: CosStyle.colorAccent

		text: qsTr("Egészítsd ki a szöveget!")
	}

	QButton {
		id: btnOk
		enabled: false
		anchors.verticalCenter: labelQuestion.verticalCenter
		anchors.right: parent.right
		anchors.rightMargin: 20
		icon.source: CosStyle.iconOK
		text: qsTr("Kész")
		themeColors: CosStyle.buttonThemeGreen
		onClicked: answer()
	}




	GridLayout {
		id: grid
		anchors.bottom: labelQuestion.top
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.topMargin: 20


		columns: control.width > control.height ? 2 : 1

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			implicitHeight: 50
			implicitWidth: 50

			Flickable {
				id: flick

				width: parent.width-20
				height: Math.min(parent.height-10, flick.contentHeight)
				anchors.centerIn: parent

				clip: true

				contentWidth: wordFlow.width
				contentHeight: wordFlow.height

				boundsBehavior: Flickable.StopAtBounds
				flickableDirection: Flickable.VerticalFlick


				Flow {
					id: wordFlow
					width: flick.width

					spacing: 5

					Behavior on height {
						SmoothedAnimation { duration: 125 }
					}

					Behavior on width {
						SmoothedAnimation { duration: 125 }
					}

					move: Transition {
						NumberAnimation { properties: "x,y"; duration: 125; easing.type: Easing.OutQuad }
					}
				}
			}

			Rectangle {
				id: rectLine
				anchors.top: grid.columns > 1 ? parent.top : parent.bottom
				anchors.left: grid.columns > 1 ? parent.right : parent.left
				height: grid.columns > 1 ? parent.height : 1
				width: grid.columns > 1 ? 1 : parent.width

				gradient: Gradient {
					orientation: grid.columns > 1 ? Gradient.Vertical : Gradient.Horizontal
					GradientStop { position: 0.0; color: "transparent" }
					GradientStop { position: 0.1; color: CosStyle.colorAccent }
					GradientStop { position: 0.9; color: CosStyle.colorAccent }
					GradientStop { position: 1.0; color: "transparent" }
				}
			}
		}



		GameQuestionTileContainer {
			id: container
			Layout.fillWidth: !(grid.columns > 1)
			Layout.maximumWidth: (grid.columns > 1) ? Math.min(Math.max(implicitWidth, control.width*0.2), control.width*0.4) : -1
			Layout.fillHeight: (grid.columns > 1)
			Layout.maximumHeight: (grid.columns > 1) ? -1 : Math.min(Math.max(implicitHeight, control.height*0.3), control.height*0.4)
		}

	}

	Component {
		id: componentWord
		QLabel {
			height: 40
			verticalAlignment: Text.AlignVCenter
			color: CosStyle.colorPrimaryLighter
			font.weight: Font.Medium
		}
	}

	Component {
		id: componentTileDrop

		GameQuestionTileDrop {
			implicitHeight: 40

			onCurrentDragChanged: recalculate()
		}
	}

	Component {
		id: componentTileDrag

		GameQuestionTileDrag {
			dropFlow: container.flow
			mainContainer: control
			interactive: _dragInteractive
		}
	}

	Component.onCompleted:  {
		if (!questionData || !questionData.list)
			return

		for (var i=0; i<questionData.list.length; i++) {
			var p = questionData.list[i]

			if (p.w) {
				componentWord.createObject(wordFlow, { text: p.w })
			} else if (p.q) {
				var o = componentTileDrop.createObject(wordFlow)
				var d = {}
				d.item = o
				d.correct = null

				if (answerData)
					d.correct = answerData[p.q]

				_drops.push(d)
			}

		}

		if (!questionData.options)
			return

		for (i=0; i<questionData.options.length; i++) {
			var t = questionData.options[i]
			componentTileDrag.createObject(container.flow, {
											   tileData: t,
											   text: t
										   })

		}

	}


	function recalculate() {
		if (!_drops.length || btnOk.enabled)
			return

		var s = true

		for (var i=0; i<_drops.length; i++) {
			var p = _drops[i]
			if (!p.item.currentDrag) {
				s = false
				break
			}
		}

		if (s)
			btnOk.enabled = true
	}

	function answer() {
		btnOk.enabled = false
		_dragInteractive = false

		var success = true

		for (var i=0; i<_drops.length; i++) {
			var p = _drops[i]

			if (p.item && p.correct) {
				var drag = p.item.currentDrag

				if (drag) {
					var data = drag.tileData

					if (data === p.correct) {
						drag.type = GameQuestionButton.Correct
					} else {
						drag.type = GameQuestionButton.Wrong
						success = false
					}
				} else {
					p.item.isWrong = true
					success = false
				}

			}
		}

		if (success)
			succeed()
		else
			failed()

	}


	function keyPressed(key) {
		if (btnOk.enabled && (key === Qt.Key_Enter || key === Qt.Key_Return))
			btnOk.press()
	}

}
