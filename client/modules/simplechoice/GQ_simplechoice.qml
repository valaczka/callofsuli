import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: imageButtons || img.visible ? 500 : titleRow.implicitHeight+grid.implicitHeight+35
	implicitWidth: 700

	readonly property bool imageButtons: questionData.imageAnswers === true



	GameQuestionTitle {
		id: titleRow

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		buttons: false

		title: questionData.question
	}

	Item {
		id: containerItem

		readonly property bool isHorizontal: control.width > control.height

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: titleRow.top
		anchors.topMargin: 15

		Image {
			id: img
			source: questionData.image !== undefined ? questionData.image : ""
			visible: questionData.image !== undefined

			width: (containerItem.isHorizontal ? parent.width/2 : parent.width)
			height: (containerItem.isHorizontal ? parent.height : parent.height/2)-10

			anchors.top: parent.top
			anchors.left: parent.left
			anchors.topMargin: 5
			anchors.leftMargin: 5

			fillMode: Image.PreserveAspectFit
			cache: false
		}

		Flickable {
			id: flick

			/*width: img.visible && containerItem.isHorizontal ? parent.width/2 : parent.width
			height: Math.min(!img.visible || containerItem.isHorizontal ? parent.height : parent.height/2,
							 flick.contentHeight)
			x: img.visible && containerItem.isHorizontal ?
				   img.width+(parent.width-img.width-width)/2 :
				   (parent.width-width)/2
			y: img.visible && !containerItem.isHorizontal ?
				   img.height+(parent.height-img.height-height)/2 :
				   (parent.height-height)/2*/

			anchors.top: img.visible && !containerItem.isHorizontal ? img.bottom : parent.top
			anchors.left: img.visible && containerItem.isHorizontal ? img.right: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.topMargin: img.visible && !containerItem.isHorizontal ? 10 : 0
			anchors.leftMargin: img.visible && containerItem.isHorizontal ? 10 : 20
			anchors.rightMargin: img.visible && containerItem.isHorizontal ? 10 : 20

			clip: true

			contentWidth: grid.width
			contentHeight: grid.height

			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick

			ScrollIndicator.vertical: ScrollIndicator { }



			Grid {
				id: grid

				width: flick.width

				y: Math.max((flick.height-height)/2, 0)
				rowSpacing: 0
				columnSpacing: 0

				columns: imageButtons ? 2 : 1

				Repeater {
					id: rptr
					model: questionData.options
					delegate: cmpNormal
				}
			}
		}
	}


	Component {
		id: cmpNormal

		GameQuestionButton {
			id: btn
			text: modelData
			width: imageButtons ? (grid.width-grid.columnSpacing)/2 : grid.width
			height: imageButtons ? (flick.height-grid.rowSpacing)/2 : implicitHeight

			onClicked: answer(index)

			Connections {
				target: control
				function onAnswerReveal(answer) {
					if (index === questionData.answer)
						btn.buttonType = GameQuestionButton.Correct
					else if (answer.index === index)
						btn.buttonType = GameQuestionButton.Wrong
				}
			}
		}
	}


	Component {
		id: cmpToggle

		GameQuestionCheckButton {
			id: btn
			text: modelData
			width: imageButtons ? (grid.width-grid.columnSpacing)/2 : grid.width
			height: imageButtons ? (flick.height-grid.rowSpacing)/2 : implicitHeight

			/*


			//image: imageButtons ? modelData : ""

			onToggled: {
				for (var i=0; i<rptr.model.length; i++) {
					var b = rptr.itemAt(i)
					if (b !== btn)
						b.selected = false
				}
			}

			Connections {
				target: control
				function onButtonReveal(original) {
					btn.interactive = false

					if (original === btn && index !== questionData.answer)
						btn.type = GameQuestionButton.Wrong

					if (index === questionData.answer)
						btn.type = GameQuestionButton.Correct
				}

				function onButtonPressByKey(num) {
					if (num-1 == index && btn.interactive) {
						if (isToggle) {
							btn.selected = true
							btn.toggled()
						} else {
							btn.clicked()
						}
					}
				}
			}

			*/
		}
	}

	function answer(idx) {
		if (idx === questionData.answer)
			question.onSuccess({"index": idx})
		else
			question.onFailed({"index": idx})
	}

	/*function answer(btnIndex, btn) {
		if (mode == GameMatch.ModeExam) {
			var idx = -1
			for (var i=0; i<rptr.model.length; i++) {
				var b = rptr.itemAt(i)
				if (b.selected)
					idx = i
			}
			answered({index: idx})
		} else {
			buttonReveal(btn)

			if (btnIndex === questionData.answer)
				succeed()
			else
				failed()
		}
	}*/

	Keys.onPressed: {
		var key = event.key

		if (key === Qt.Key_1 || key === Qt.Key_A)
			answer(0)
		else if (key === Qt.Key_2 || key === Qt.Key_B)
			answer(1)
		else if (key === Qt.Key_3 || key === Qt.Key_C)
			answer(2)
		else if (key === Qt.Key_4 || key === Qt.Key_D)
			answer(3)
		else if (key === Qt.Key_5 || key === Qt.Key_E)
			answer(4)
		else if (key === Qt.Key_6 || key === Qt.Key_F)
			answer(5)
	}
}

