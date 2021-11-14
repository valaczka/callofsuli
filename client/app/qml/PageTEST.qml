import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: control

	defaultTitle: qsTr("Test")

	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		initialItem: QSimpleContainer {
			id: panel

			title: "Test"
			icon: CosStyle.iconSetup


			GameQuestionTileLayout {
				id: grid
				width: panel.panelWidth
				height: panel.panelHeight*0.7

				flick.contentWidth: col.width
				flick.contentHeight: col.height

				Column {
					id: col
					width: grid.flick.width
					parent: grid.flick.contentItem

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
						width: parent.width/2

						color: CosStyle.colorPrimaryLighter
						font.weight: Font.Medium

						rightPadding: 10
						leftPadding: 5

						horizontalAlignment: Text.AlignRight
						verticalAlignment: Text.AlignVCenter

						wrapMode: Text.Wrap
					}

					GameQuestionTileDrop {
						id: drop
						implicitHeight: 40
						autoResize: false
						width: parent.width/2

						anchors.verticalCenter: parent.verticalCenter
						anchors.right: parent.right

						//onCurrentDragChanged: recalculate()
					}
				}
			}

			Component {
				id: componentTileDrag

				GameQuestionTileDrag {
					dropFlow: grid.container.flow
					mainContainer: control
				}
			}


			Component.onCompleted:  {
				for (var i=0; i<6; i++) {
					var o = componentTileDrop.createObject(col, {text: "szoveg "+i+" —"})
					/*var d = {}
					d.item = o
					d.correct = null

					if (answerData && answerData.list && answerData.list.length>=i)
						d.correct = answerData[i]

					_drops.push(d)*/
				}

				var s = [
							"text1",
							"aksdf owei alkdfjé aldfiowe éakldjf aklésf asf",
							"lkjalwieruwoeiasdklfjla",
							"lkj asd,.fa ksdfj",
							"jkk",
							"12",
							"oiaueoriau dfkéal dflakédfio euwr éadkfj wek adkfj dfoaeiwr"
						]

				for (i=0; i<7; i++) {
					var oo = componentTileDrag.createObject(grid.container.flow, {
													   tileData: s[i],
													   text: s[i]
												   })
				}
			}

		}

	}



	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}
