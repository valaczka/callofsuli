import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitHeight: labelQuestion.implicitHeight+(implicitButtonSize*4)+25
	implicitWidth: Math.max(500, (labelQuestion.implicitWidth+labelSuffix.implicitWidth+200))			// 200 -> labelNumber1.width

	required property var questionData
	property bool canPostpone: false
	property int mode: GameMatch.ModeNormal

	property bool twoLineMode: questionData.twoLine
	property bool decimalEnabled: questionData.decimalEnabled
	property int activeLine: 1
	property bool interactive: true

	readonly property real implicitButtonSize: 75
	property real buttonSize: parent.height < implicitHeight ?
								  Math.floor((parent.height-25-labelQuestion.implicitHeight)/4) :
								  implicitButtonSize


	signal succeed()
	signal failed()
	signal postponed()
	signal answered(var answer)

	RowLayout {
		id: row
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottomMargin: 5


		QLabel {
			id: labelQuestion

			Layout.fillWidth: true
			Layout.fillHeight: true

			font.family: "Special Elite"
			font.pixelSize: CosStyle.pixelSize*1.4
			wrapMode: Text.Wrap
			topPadding: 25
			bottomPadding: 25
			leftPadding: 10
			rightPadding: 10

			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter

			color: CosStyle.colorAccent

			textFormat: Text.RichText

			text: questionData.question
		}

		Column {
			id: col
			width: Math.min(control.width*0.4, 200)
			spacing: 4

			Rectangle {
				border.width: 1
				border.color: !twoLineMode || activeLine != 1 ? CosStyle.colorPrimaryDark : CosStyle.colorPrimaryDarker
				color: "black"

				width: parent.width
				height: labelNumber1.implicitHeight

				QLabel {
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
					color: CosStyle.colorWarning
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
				color: CosStyle.colorWarning
				height: 2
				visible: twoLineMode
			}

			Rectangle {
				border.width: 1
				border.color: !twoLineMode || activeLine != 2 ? CosStyle.colorPrimaryDark : CosStyle.colorPrimaryDarker
				color: "black"
				visible: twoLineMode

				width: parent.width
				height: labelNumber2.implicitHeight

				QLabel {
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
					color: CosStyle.colorWarning
					text: "0"
				}

				MouseArea {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton
					onClicked: activeLine = 2
				}
			}
		}

		QLabel {
			id: labelSuffix

			Layout.fillWidth: false
			Layout.fillHeight: true

			font: labelQuestion.font
			wrapMode: Text.Wrap
			topPadding: labelQuestion.topPadding
			bottomPadding: labelQuestion.bottomPadding
			leftPadding: labelQuestion.leftPadding
			rightPadding: labelQuestion.rightPadding

			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter

			color: labelQuestion.color

			textFormat: Text.RichText

			text: questionData.suffix
		}
	}

	Item {
		anchors.bottom: row.top
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottomMargin: 20
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		anchors.topMargin: 20

		GridLayout {
			anchors.fill: parent
			columns: 4

			rowSpacing: 3
			columnSpacing: 3

			Repeater {
				model: control.mode == GameMatch.ModeExam && canPostpone ? [
																"7",
																"8",
																"9",
																"\ue14a",
																"4",
																"5",
																"6",
																"C",
																"1",
																"2",
																"3",
																"\ue01f",
																"±",
																"0",
																",",
																"="
															]
														  : [
																"7",
																"8",
																"9",
																"\ue14a",
																"4",
																"5",
																"6",
																"C",
																"1",
																"2",
																"3",
																"=",
																"±",
																"0",
																","
															]

				GameQuestionButton {
					Layout.fillHeight: true
					Layout.fillWidth: true
					Layout.rowSpan: (control.mode != GameMatch.ModeExam || !canPostpone) && modelData == "=" ? 2 : 1
					text: modelData
					onClicked: clickBtn(text)
					label.font.family: (modelData == "\ue14a" || modelData == "\ue01f") ? "Material Icons" : "Rajdhani"
					label.font.weight: Font.Light
					label.font.pixelSize: (modelData == "\ue14a" || modelData == "\ue01f") ? buttonSize*0.5 : buttonSize*0.75
					interactive: modelData == "," ? control.interactive && control.decimalEnabled : control.interactive

					themeColors: control.mode == GameMatch.ModeExam ?
									 (modelData == "\ue01f" ? CosStyle.buttonThemeOrange :
															  (modelData == "=" ? CosStyle.buttonThemeGreen :
																				  CosStyle.buttonThemeDefault)) :
									 CosStyle.buttonThemeDefault
				}
			}
		}
	}





	function clickBtn(t) {
		var origStr = twoLineMode && activeLine == 2 ? labelNumber2.text : labelNumber1.text
		var newStr = origStr

		if (t === "\ue01f") {			// Postpne
			postponed()
		} else if (t === "\ue14a") {			// Backspace
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


		if (mode == GameMatch.ModeExam) {
			answered(ret)
		} else {
			if (correct) {
				succeed()
				labelNumber1.color = CosStyle.colorOKLighter
				labelNumber2.color = CosStyle.colorOKLighter
			} else {
				failed()
				labelNumber1.color = CosStyle.colorErrorLighter
				labelNumber2.color = CosStyle.colorErrorLighter

				labelNumber1.text = questionData.answer.first
				labelNumber2.text = questionData.answer.second
			}
		}

		/*if (questionData.correct) {
			btnTrue.type = GameQuestionButton.Correct
			if (!correct) btnFalse.type = GameQuestionButton.Wrong
		} else {
			btnFalse.type = GameQuestionButton.Correct
			if (!correct) btnTrue.type = GameQuestionButton.Wrong
		}*/
	}


	function keyPressed(key) {
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
			clickBtn("\ue14a")
		else if (key === Qt.Key_Down)
			activeLine = 2
		else if (key === Qt.Key_Up)
			activeLine = 1
	}
}

