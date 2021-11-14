import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitHeight: labelQuestion.height+col.height+35
	implicitWidth: 700

	required property var questionData
	required property var answerData

	property real buttonWidth: width-60


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

		color: CosStyle.colorAccent

		textFormat: Text.RichText

		text: questionData.question
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



	GameQuestionTileLayout {
		id: grid
		anchors.bottom: labelQuestion.top
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top

		flick.contentWidth: col.width
		flick.contentHeight: col.height

		Column {
			id: col
			width: grid.flick.width
			parent: grid.flick.contentItem
			spacing: 5

		}
	}



	Component {
		id: componentTileDrop

		Item {
			implicitWidth: labelField.implicitWidth+drop.implicitWidth
			implicitHeight: Math.max(labelField.height, drop.height)

			width: parent.width
			height: implicitHeight
			anchors.horizontalCenter: parent.horizontalCenter

			property alias text: labelField.text
			property alias drop: drop

			QLabel {
				id: labelField

				anchors.verticalCenter: parent.verticalCenter
				anchors.left: parent.left
				width: (parent.width-labelSeparator.width)/2

				color: CosStyle.colorPrimaryLighter
				font.weight: Font.Medium

				leftPadding: 5

				horizontalAlignment: Text.AlignRight
				verticalAlignment: Text.AlignVCenter

				wrapMode: Text.Wrap
			}

			QLabel {
				id: labelSeparator

				anchors.centerIn: parent

				text: "—"

				color: CosStyle.colorPrimaryLighter
				font.weight: Font.Medium

				rightPadding: 10
				leftPadding: 10

				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
			}

			GameQuestionTileDrop {
				id: drop
				implicitHeight: 40
				autoResize: false
				width: (parent.width-labelSeparator.width)/2

				anchors.verticalCenter: parent.verticalCenter
				anchors.right: parent.right

				onCurrentDragChanged: recalculate()
			}
		}
	}

	Component {
		id: componentTileDrag

		GameQuestionTileDrag {
			dropFlow: grid.container.flow
			mainContainer: control
			interactive: _dragInteractive
		}
	}


	Component.onCompleted:  {
		if (!questionData || !questionData.list)
			return

		for (var i=0; i<questionData.list.length; i++) {
			var p = questionData.list[i]


			var o = componentTileDrop.createObject(col, {text: p})
			var d = {}
			d.item = o
			d.correct = null

			if (answerData && answerData.list && answerData.list.length>i)
				d.correct = answerData.list[i]

			_drops.push(d)
		}

		if (!questionData.options)
			return

		for (i=0; i<questionData.options.length; i++) {
			var t = questionData.options[i]
			componentTileDrag.createObject(grid.container.flow, {
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
			if (!p.item.drop.currentDrag) {
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
				var drag = p.item.drop.currentDrag

				if (drag) {
					var data = drag.tileData

					if (data === p.correct) {
						drag.type = GameQuestionButton.Correct
					} else {
						drag.type = GameQuestionButton.Wrong
						success = false
					}
				} else {
					p.item.drop.isWrong = true
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

