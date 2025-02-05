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

			//ScrollIndicator.vertical: ScrollIndicator { active: flick.movingVertically || flick.contentHeight > flick.height }
			ScrollBar.vertical: ScrollBar {
				id: _scrollBar
				parent: containerItem
				anchors.top: flick.top
				anchors.right: flick.right
				anchors.bottom: flick.bottom
				policy: flick.contentHeight > flick.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
			}


			Column {
				id: col

				readonly property int _padding: containerItem.isHorizontal ? 10 : 20

				width: flick.width - _padding - Math.max(_padding, _scrollBar.visible ? _scrollBar.width : 0)
				x: _padding
				spacing: 3

				Repeater {
					id: rptr
					model: questionData.options
					delegate: GameQuestionCheckButton {
						id: btn
						text: modelData
						width: col.width

						fontFamily: questionData.monospace ? "Ubuntu Mono" : ""

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

		else if (key === Qt.Key_1)
			i = 0
		else if (key === Qt.Key_2)
			i = 1
		else if (key === Qt.Key_3)
			i = 2
		else if (key === Qt.Key_4)
			i = 3
		else if (key === Qt.Key_5)
			i = 4
		else if (key === Qt.Key_6)
			i = 5
		else if (key === Qt.Key_7)
			i = 6
		else if (key === Qt.Key_8)
			i = 7
		else if (key === Qt.Key_9)
			i = 8

		var idx = selectedItems.indexOf(i)
		if (idx != -1)
			selectedItems.splice(idx, 1)
		else
			selectedItems.push(i)

		selectedItemsChanged()

	}
}


