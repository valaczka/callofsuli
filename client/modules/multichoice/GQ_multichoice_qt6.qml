import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: titleRow.implicitHeight
					+(Qaterial.Style.gameButtonImplicitHeight*rptr.model.length)
					+(col.spacing*rptr.model.length)
					+35

	implicitWidth: 700 * Qaterial.Style.pixelSizeRatio

	property var selectedItems: []


	GameQuestionTitle {
		id: titleRow

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		buttons: true
		buttonOkEnabled: true

		title: questionData.question

		onButtonOkClicked: answer()

	}

	Item {
		id: containerItem

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: titleRow.top
		anchors.topMargin: 15

		readonly property bool isHorizontal: control.width > control.height

		Flickable {
			id: flick

			width: parent.width
			height: Math.min(parent.height, flick.contentHeight)
			anchors.centerIn: parent

			clip: true

			contentWidth: col.width
			contentHeight: col.height

			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick

			ScrollIndicator.vertical: ScrollIndicator { active: flick.movingVertically || flick.contentHeight > flick.height }


			Column {
				id: col

				width: flick.width - 2*(containerItem.isHorizontal ? 10 : 20)
				x: (flick.width-width)/2
				spacing: 3

				Repeater {
					id: rptr
					model: questionData.options
					delegate: GameQuestionCheckButton {
						id: btn
						text: modelData
						width: col.width

						checked: control.selectedItems.includes(_idx)

						readonly property int _idx: index

						onToggled: {
							if (!checked) {
								var idx = selectedItems.indexOf(_idx)
								if (idx != -1)
									selectedItems.splice(idx, 1)
							} else if (!selectedItems.includes(_idx)){
								selectedItems.push(_idx)
							}
						}


						Connections {
							target: control

							function onAnswerReveal(answer) {
								if (questionData.answer.includes(index))
									btn.buttonType = GameQuestionButton.Correct
								else if (answer.list.includes(index))
									btn.buttonType = GameQuestionButton.Wrong
							}

							/*function onSelectedItemsChanged() {
								console.debug("SELECTED ITEMS CHANGED", control.selectedItems)

								btn.checked = control.selectedItems.includes(btn._idx)
							}*/
						}
					}
				}
			}
		}
	}



	onQuestionChanged: {
		if (storedAnswer.list !== undefined) {
			selectedItems = storedAnswer.list
		}
	}



	function answer() {
		var success = true
		var idx = []

		for (var i=0; i<rptr.model.length; i++) {
			var p = rptr.itemAt(i)

			if (p.checked)
				idx.push(i)

			var correct = questionData.answer.includes(i)

			if ((correct && !p.checked) || (!correct && p.checked))
				success = false
		}

		if (success)
			question.onSuccess({"list": idx})
		else
			question.onFailed({"list": idx})
	}


	Keys.onPressed: event => {
		var key = event.key

		var i = -1

		if (key === Qt.Key_Return || key === Qt.Key_Enter) {
			titleRow.buttonOkClicked()
			return
		}

		else if (key === Qt.Key_1 || key === Qt.Key_A)
			i = 0
		else if (key === Qt.Key_2 || key === Qt.Key_B)
			i = 1
		else if (key === Qt.Key_3 || key === Qt.Key_C)
			i = 2
		else if (key === Qt.Key_4 || key === Qt.Key_D)
			i = 3
		else if (key === Qt.Key_5 || key === Qt.Key_E)
			i = 4
		else if (key === Qt.Key_6 || key === Qt.Key_F)
			i = 5
		else if (key === Qt.Key_7 || key === Qt.Key_G)
			i = 6
		else if (key === Qt.Key_8 || key === Qt.Key_H)
			i = 7
		else if (key === Qt.Key_9 || key === Qt.Key_I)
			i = 8

		var idx = selectedItems.indexOf(i)
		if (idx != -1)
			selectedItems.splice(idx, 1)
		else
			selectedItems.push(i)

		selectedItemsChanged()

	}
}


