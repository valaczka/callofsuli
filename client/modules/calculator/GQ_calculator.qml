import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: titleRow.titleLabel.implicitHeight+(implicitButtonSize*4)+25
	implicitWidth: Math.max(500, (titleRow.titleLabel.implicitWidth+labelSuffix.implicitWidth+200))			// 200 -> labelNumber1.width

	property bool twoLineMode: questionData.twoLine
	property bool decimalEnabled: questionData.decimalEnabled
	property int activeLine: 1

	property bool interactive: true

	readonly property real implicitButtonSize: 75
	property real buttonSize: parent.height < implicitHeight ?
								  Math.floor((parent.height-25-titleRow.titleLabel.implicitHeight)/4) :
								  implicitButtonSize


	RowLayout {
		id: row
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottomMargin: 5

		GameQuestionTitle {
			id: titleRow

			Layout.fillWidth: true
			Layout.fillHeight: true

			buttons: false
			buttonOkEnabled: false

			title: questionData.question
		}


		Column {
			id: col
			width: Math.min(control.width*0.4, 200)
			spacing: 4

			Rectangle {
				border.width: 1
				border.color: !twoLineMode || activeLine != 1 ? Qaterial.Colors.cyan900 : Qaterial.Colors.cyan500
				color: Qaterial.Colors.black

				width: parent.width
				height: labelNumber1.implicitHeight

				Label {
					id: labelNumber1
					anchors.fill: parent
					horizontalAlignment: Text.AlignRight
					verticalAlignment: Text.AlignVCenter
					font.family: "Renegade Master"
					font.pixelSize: 35*(col.width/200)
					leftPadding: 10
					rightPadding: 10
					topPadding: 2
					bottomPadding: 2
					color: Qaterial.Colors.orange400
					text: "0"
				}

				MouseArea {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton
					onClicked: activeLine = 1
				}
			}

			Rectangle {
				width: parent.width
				color: Qaterial.Colors.orange400
				height: 2
				visible: twoLineMode
			}

			Rectangle {
				border.width: 1
				border.color: !twoLineMode || activeLine != 2 ? Qaterial.Colors.cyan900 : Qaterial.Colors.cyan500
				color: Qaterial.Colors.black
				visible: twoLineMode

				width: parent.width
				height: labelNumber2.implicitHeight

				Label {
					id: labelNumber2
					anchors.fill: parent
					horizontalAlignment: Text.AlignRight
					verticalAlignment: Text.AlignVCenter
					font.family: "Renegade Master"
					font.pixelSize: 35*(col.width/200)
					leftPadding: 10
					rightPadding: 10
					topPadding: 2
					bottomPadding: 2
					color: Qaterial.Colors.orange400
					text: "0"
				}

				MouseArea {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton
					onClicked: activeLine = 2
				}
			}
		}


		Label {
			id: labelSuffix

			Layout.fillWidth: false
			Layout.fillHeight: true

			Layout.alignment: Qt.AlignCenter

			horizontalAlignment: Text.AlignHCenter

			font: titleRow.titleLabel.font

			wrapMode: Text.Wrap
			topPadding: titleRow.titleLabel.topPadding
			bottomPadding: titleRow.titleLabel.bottomPadding
			leftPadding: titleRow.titleLabel.leftPadding
			rightPadding: titleRow.titleLabel.rightPadding

			color: titleRow.titleLabel.color

			textFormat: Text.RichText

			text: questionData.suffix
		}
	}





	Item {
		id: containerItem

		anchors.bottom: row.top
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottomMargin: 20
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		anchors.topMargin: 20

		GridLayout {
			id: grid
			width: Math.min(parent.width, implicitButtonSize*1.5*4)
			height: Math.min(parent.height, implicitButtonSize*1.5*4)
			anchors.centerIn: parent
			columns: 4

			readonly property real btnSize: Math.min(width, height)/4

			rowSpacing: 3
			columnSpacing: 3

			ListModel {
				id: modelPostpone
				ListElement { type: "normal"; buttonText: "7" }
				ListElement { type: "normal"; buttonText: "8" }
				ListElement { type: "normal"; buttonText: "9" }
				ListElement { type: "back"; buttonText: "BACK" }
				ListElement { type: "normal"; buttonText: "4" }
				ListElement { type: "normal"; buttonText: "5" }
				ListElement { type: "normal"; buttonText: "6" }
				ListElement { type: "normal"; buttonText: "C" }
				ListElement { type: "normal"; buttonText: "1" }
				ListElement { type: "normal"; buttonText: "2" }
				ListElement { type: "normal"; buttonText: "3" }
				ListElement { type: "postpone"; buttonText: "POSTPONE" }
				ListElement { type: "normal"; buttonText: "±" }
				ListElement { type: "normal"; buttonText: "0" }
				ListElement { type: "normal"; buttonText: "," }
				ListElement { type: "enter"; buttonText: "=" }
			}


			ListModel {
				id: modelNormal

				ListElement { type: "normal"; buttonText: "7" }
				ListElement { type: "normal"; buttonText: "8" }
				ListElement { type: "normal"; buttonText: "9" }
				ListElement { type: "back"; buttonText: "BACK" }
				ListElement { type: "normal"; buttonText: "4" }
				ListElement { type: "normal"; buttonText: "5" }
				ListElement { type: "normal"; buttonText: "6" }
				ListElement { type: "normal"; buttonText: "C" }
				ListElement { type: "normal"; buttonText: "1" }
				ListElement { type: "normal"; buttonText: "2" }
				ListElement { type: "normal"; buttonText: "3" }
				ListElement { type: "enter"; buttonText: "=" }
				ListElement { type: "normal"; buttonText: "±" }
				ListElement { type: "normal"; buttonText: "0" }
				ListElement { type: "normal"; buttonText: "," }
			}

			Repeater {
				model: question.postponeEnabled ? modelPostpone : modelNormal


				delegate: chooser

				DelegateChooser {
					id: chooser
					role: "type"
					DelegateChoice { roleValue: "back"; delegate: cmpBack }
					DelegateChoice { roleValue: "enter"; delegate: cmpEnter }
					DelegateChoice { roleValue: "postpone"; delegate: cmpPostpone }
					DelegateChoice { roleValue: "normal"; delegate: cmpNormal }
				}


			}
		}


	}


	Component {
		id: cmpNormal

		GameQuestionButton {
			required property string buttonText
			Layout.fillHeight: true
			Layout.fillWidth: true
			text: buttonText
			display: AbstractButton.TextOnly
			onClicked: clickBtn(buttonText)
			font.family: "Rajdhani"
			font.weight: Font.Light
			font.pixelSize: grid.btnSize*0.75
			enabled: buttonText == "," ? control.interactive && control.decimalEnabled : control.interactive
			opacity: enabled ? 1.0 : 0.5
		}
	}


	Component {
		id: cmpBack

		GameQuestionButton {
			Layout.fillHeight: true
			Layout.fillWidth: true
			text: ""
			display: AbstractButton.IconOnly
			icon.source: Qaterial.Icons.arrowLeft
			icon.width: grid.btnSize*0.6
			icon.height: grid.btnSize*0.6
			onClicked: clickBtn(buttonText)
			enabled: control.interactive
		}
	}


	Component {
		id: cmpEnter

		GameQuestionButtonOk {
			required property string buttonText
			Layout.fillHeight: true
			Layout.fillWidth: true
			Layout.rowSpan: question.postponeEnabled ? 1 : 2
			text: buttonText
			icon.source: ""
			display: AbstractButton.TextOnly
			onClicked: clickBtn(buttonText)
			font.family: "Rajdhani"
			font.weight: Font.Light
			font.pixelSize: grid.btnSize*0.75
			enabled: control.interactive
		}
	}

	Component {
		id: cmpPostpone

		GameQuestionButtonPostpone {
			Layout.fillHeight: true
			Layout.fillWidth: true

			icon.width: grid.btnSize*0.6
			icon.height: grid.btnSize*0.6

			display: AbstractButton.IconOnly

			onClicked: question.onPostpone()
		}
	}


	onAnswerReveal: {
		if (Number(labelNumber1.text) === questionData.answer.first) {
			labelNumber1.color = Qaterial.Colors.green400
		} else {
			labelNumber1.text = questionData.answer.first
			labelNumber1.color = Qaterial.Colors.red500
		}

		if (Number(labelNumber2.text) === questionData.answer.second) {
			labelNumber2.color = Qaterial.Colors.green400
		} else {
			labelNumber2.text = questionData.answer.second
			labelNumber2.color = Qaterial.Colors.red500
		}
	}


	onQuestionChanged: {
		if (storedAnswer.first !== undefined)
			labelNumber1.text = Number(storedAnswer.first)
		if (storedAnswer.second !== undefined)
			labelNumber2.text = Number(storedAnswer.second)
	}






	function clickBtn(t) {
		var origStr = twoLineMode && activeLine == 2 ? labelNumber2.text : labelNumber1.text
		var newStr = origStr

		if (!interactive)
			return

		if (t === "BACK") {			// Backspace
			newStr = origStr.length < 2 || origStr === "-0" ? "0" : String(origStr).substring(0, origStr.length-1)

			if (twoLineMode && activeLine == 2)
				labelNumber2.text = newStr
			else
				labelNumber1.text = newStr
		} else if (t === "C") {
			if (twoLineMode && activeLine == 2)
				labelNumber2.text = "0"
			else
				labelNumber1.text = "0"
		} else if (t === "=") {
			answer()
		} else if (t === "±") {
			var t1 = String(labelNumber1.text)
			if (t1.startsWith("-"))
				newStr = t1.substr(1)
			else
				newStr = "-"+t1

			labelNumber1.text = newStr
		}

		var maxLength = 6

		if (String(origStr).startsWith("-"))
			maxLength++

		if (String(origStr).indexOf(".") != -1)
			maxLength++

		if (origStr.length >= maxLength)
			return

		if (t === "," && decimalEnabled) {
			if (String(origStr).indexOf(".") === -1) {
				newStr = origStr+"."

				if (twoLineMode && activeLine == 2)
					labelNumber2.text = newStr
				else
					labelNumber1.text = newStr
			}
		}  else if (t === "1" || t === "2" || t === "3" || t === "4" || t === "5" ||
					t === "6" || t === "7" || t === "8" || t === "9" || t === "0") {

			if (origStr === "0")
				newStr = t
			else if (origStr === "-0")
				newStr = "-"+t
			else
				newStr = origStr+t

			if (twoLineMode && activeLine == 2)
				labelNumber2.text = newStr
			else
				labelNumber1.text = newStr
		}
	}




	function answer() {
		var ret = {}

		interactive = false

		var correct = true

		var n1 = Number(labelNumber1.text)

		if (n1 !== questionData.answer.first)
			correct = false

		ret.first = n1

		if (twoLineMode) {
			var n2 = Number(labelNumber2.text)

			if (n2 !== questionData.answer.second)
				correct = false

			ret.second = n2
		}

		if (correct)
			question.onSuccess(ret)
		else
			question.onFailed(ret)
	}


	Keys.onPressed: {
		var key = event.key

		if (key === Qt.Key_Enter || key === Qt.Key_Return)
			clickBtn("=")
		else if (key === Qt.Key_0)
			clickBtn("0")
		else if (key === Qt.Key_1)
			clickBtn("1")
		else if (key === Qt.Key_2)
			clickBtn("2")
		else if (key === Qt.Key_3)
			clickBtn("3")
		else if (key === Qt.Key_4)
			clickBtn("4")
		else if (key === Qt.Key_5)
			clickBtn("5")
		else if (key === Qt.Key_6)
			clickBtn("6")
		else if (key === Qt.Key_7)
			clickBtn("7")
		else if (key === Qt.Key_8)
			clickBtn("8")
		else if (key === Qt.Key_9)
			clickBtn("9")
		else if (key === Qt.Key_Comma || key === Qt.Key_Period)
			clickBtn(",")
		else if (key === Qt.Key_Plus || key === Qt.Key_Minus)
			clickBtn("±")
		else if (key === Qt.Key_Delete)
			clickBtn("C")
		else if (key === Qt.Key_Backspace)
			clickBtn("BACK")
		else if (key === Qt.Key_Down)
			activeLine = 2
		else if (key === Qt.Key_Up)
			activeLine = 1
	}
}






