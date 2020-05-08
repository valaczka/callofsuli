import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


MODabstractEditor {
	id: control

	QAccordion {
		id: accordion

		anchors.fill: parent

		QCollapsible {
			title: qsTr("Kérdések-válaszok")

			QListView {
				id: listPairs

				width: parent.width

				model: ListModel { }

				delegate: QRectangleBg {
					id: delegateitem
					width: listPairs.width
					height: row.height+6

					readonly property int sumWidth: flipable.width+labelSep.width+buttonRemove.width

					Row {
						id: row

						width: parent.width

						anchors.verticalCenter: parent.verticalCenter

						QFlipable {
							id: flipable
							width: height
							height: textQuestion.height
							anchors.verticalCenter: parent.verticalCenter

							contentVisible: !model.isNew

							frontIcon: CosStyle.iconUnchecked
							backIcon: CosStyle.iconChecked
							color: CosStyle.colorAccent
							flipped: model.favorite

							mouseArea.onClicked: {
								model.favorite = !model.favorite
								saveJson()
							}
						}

						QTextField {
							id: textQuestion
							anchors.verticalCenter: parent.verticalCenter
							placeholderText: qsTr("Kérdés")
							text: model.question

							width: (delegateitem.width-delegateitem.sumWidth)/2

							onTextModified: {
								model.question = text
								if (!model.isNew)
									saveJson()
							}
						}

						Label {
							id: labelSep
							text: qsTr(":")
							verticalAlignment: Text.AlignVCenter
							horizontalAlignment: Text.AlignHCenter
							anchors.verticalCenter: parent.verticalCenter
							leftPadding: 5
							rightPadding: 15

						}

						QTextField {
							placeholderText: qsTr("Válasz")
							text: model.answer
							width: (delegateitem.width-delegateitem.sumWidth)/2
							anchors.verticalCenter: parent.verticalCenter

							onTextModified: {
								model.answer = text
								if (!model.isNew)
									saveJson()
							}

							onAccepted: {
								model.answer = text
								if (model.isNew && model.question.length && model.answer.length) {
									listPairs.model.insert(listPairs.model.count-1, {
															   favorite: false,
															   question: model.question,
															   answer: model.answer,
														   })
									model.question = ""
									model.answer = ""

									saveJson()
								}
							}
						}

						QRemoveButton {
							id: buttonRemove
							buttonVisible: !model.isNew
							anchors.verticalCenter: parent.verticalCenter

							onClicked: {
								listPairs.model.remove(index)
								saveJson()
							}

						}

					}


				}
			}
		}


		QCollapsible {
			QRectangleBg {
				width: 300
				height: 200
			}
		}
	}


	onJsonDataChanged: {
		listPairs.model.clear()

		var pl = data.pair
		for (var i=0; pl && i<pl.length; ++i) {
			listPairs.model.append(pl[i])
		}

		listPairs.model.append({
								   favorite: false,
								   question: "",
								   answer: "",
								   isNew: true
							   })

	}


	function saveJson() {
		var pl = []
		var d = {}

		for (var i=0; i<listPairs.model.count-1; ++i) {
			var o = listPairs.model.get(i)
			pl.push(o)
		}

		d.pairs = pl

		control.save(d)
	}
}

