import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS
import SortFilterProxyModel 0.2


QTabPage {
	id: control

	title: "Test page"
	backFunction: null


	Component {
		id: cmp1
		QTabContainer {
			title:  "Test"




			GameQuestionTileLayout {
				id: grid
				anchors.bottom: parent.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.top: parent.top
				anchors.topMargin: 50

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


					GameQuestionTileDrop {
						id: drop
						implicitHeight: 40
						autoResize: false
						width: parent.width

						anchors.fill: parent

						//onCurrentDragChanged: recalculate()

						QLabel {
							id: labelField

							visible: !drop.currentDrag

							anchors.centerIn: parent
							width: Math.min(parent.width, implicitWidth)
							wrapMode: Text.Wrap

							color: CosStyle.colorPrimaryDarkest
							font.weight: Font.Normal

							leftPadding: 5
							rightPadding: 5

							horizontalAlignment: Text.AlignHCenter
						}
					}
				}
			}

			Component {
				id: componentTileDrag

				GameQuestionTileDrag {
					dropFlow: grid.container.flow
					mainContainer: control
					interactive: true //_dragInteractive
				}
			}


			Component.onCompleted:  {
				for (var i=0; i<5; i++) {
					var p = ""

					if (i==0)
						p = "legkisebb"
					else if (i==4)
						p = "legnagyobb"

					var o = componentTileDrop.createObject(col, {text: p})
				}

				for (i=0; i<8; i++) {
					componentTileDrag.createObject(grid.container.flow, {
													   tileData: {},
													   text: "Próba "+i
												   })
				}
			}


/*

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

*/

		}
	}

	Component.onCompleted: pushContent(cmp1)




}
