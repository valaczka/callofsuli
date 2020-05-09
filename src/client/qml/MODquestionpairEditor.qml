import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


MODabstractEditor {
	id: control


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

				readonly property bool isNew: index == listPairs.model.count-1
				readonly property int sumWidth: flipable.width+buttonRemove.width

				Row {
					id: row

					width: parent.width

					anchors.verticalCenter: parent.verticalCenter

					QFlipable {
						id: flipable
						width: height
						height: textQuestion.height
						anchors.verticalCenter: parent.verticalCenter

						contentVisible: !delegateitem.isNew

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
						placeholderText: delegateitem.isNew ? qsTr("Kérdés hozzáadása") : qsTr("Kérdés")
						text: model.question

						width: (delegateitem.width-delegateitem.sumWidth)/2

						onTextModified: {
							model.question = text
							if (!delegateitem.isNew)
								saveJson()
						}
					}


					QTextField {
						placeholderText: delegateitem.isNew ? qsTr("Válasz hozzáadása") : qsTr("Válasz")
						text: model.answer
						width: (delegateitem.width-delegateitem.sumWidth)/2
						anchors.verticalCenter: parent.verticalCenter

						onTextModified: {
							model.answer = text
							if (!delegateitem.isNew)
								saveJson()
						}

						onAccepted: {
							model.answer = text
							if (delegateitem.isNew && model.question.length && model.answer.length) {
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
						buttonVisible: !delegateitem.isNew && delegateitem.mouseArea.containsMouse
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
		title: qsTr("Plusz helytelen válaszok")

		QListView {
			id: listWrongs

			width: parent.width

			model: ListModel { }

			delegate: QRectangleBg {
				id: delegatewrongitem
				width: listWrongs.width
				height: rowWrong.height+6

				readonly property bool isNew: index == listWrongs.model.count-1

				Row {
					id: rowWrong

					width: parent.width

					anchors.verticalCenter: parent.verticalCenter

					QTextField {
						id: textWrong
						anchors.verticalCenter: parent.verticalCenter
						placeholderText: delegatewrongitem.isNew ? qsTr("Helytelen válasz hozzáadása") : qsTr("Helytelen válasz")
						text: model.wrong

						width: delegatewrongitem.width-buttonWrongRemove.width

						onTextModified: {
							model.wrong = text
							if (!delegatewrongitem.isNew)
								saveJson()
						}

						onAccepted: {
							model.wrong = text
							if (delegatewrongitem.isNew && model.wrong.length) {
								listWrongs.model.insert(listWrongs.model.count-1, {
															wrong: model.wrong
														})
								model.wrong = ""

								saveJson()
							}
						}
					}

					QRemoveButton {
						id: buttonWrongRemove
						buttonVisible: !delegatewrongitem.isNew && delegatewrongitem.mouseArea.containsMouse
						anchors.verticalCenter: parent.verticalCenter

						onClicked: {
							listWrongs.model.remove(index)
							saveJson()
						}

					}

				}


			}
		}
	}


	onJsonDataChanged: {
		listPairs.model.clear()

		var pl = jsonData.pairs
		var wl = jsonData.wrongs

		for (var i=0; pl && i<pl.length; ++i) {
			listPairs.model.append(pl[i])
		}

		listPairs.model.append({
								   favorite: false,
								   question: "",
								   answer: ""
							   })


		for (i=0; wl && i<wl.length; ++i) {
			listWrongs.model.append({wrong: wl[i]})
		}

		listWrongs.model.append({
									wrong: ""
								})

	}


	function saveJsonData() {
		var pl = []
		var wl = []
		var d = {}

		for (var i=0; i<listPairs.model.count-1; ++i) {
			var o = listPairs.model.get(i)
			pl.push(o)
		}

		d.pairs = pl


		for (i=0; i<listWrongs.model.count-1; ++i) {
			o = listWrongs.model.get(i)
			wl.push(o.wrong)
		}

		d.wrongs = wl

		return d
	}
}

