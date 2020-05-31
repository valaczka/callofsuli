import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import QtQuick.Layouts 1.14
import "."
import "Style"
import "JScript.js" as JS

Column {
	id: panel

	property MapEditor map: null
	property int introId: -1
	property bool isOutro: false
	property int parentId: -1
	property int parentType: Map.IntroUndefined

	width: parent.width

	signal introRemoved()

	QCollapsible {
		title: isOutro ? qsTr("Outro beállításai") : qsTr("Intro beállításai")

		collapsed: true

		QGridLayout {
			id: grid
			width: parent.width
			watchModification: false

			QGridButton {
				text: isOutro ? qsTr("Outro hozzáadása") : qsTr("Intro hozzáadása")
				visible: introId === -1
				enabled: parentType !== Map.IntroUndefined

				onClicked: {
					var intId = map.introAdd({ttext: ""})
					if (intId !== -1) {
						introId = intId
						switch (parentType) {
						case Map.IntroCampaign:
							map.undoLogBegin(qsTr("Hadjárat intro/outro hozzáadása"))
							map.campaignIntroAdd(parentId, intId, isOutro)
							map.undoLogEnd()
							break

						case Map.IntroMission:
							map.undoLogBegin(qsTr("Küldetés inro/outro hozzáadása"))
							map.missionIntroAdd(parentId, intId, isOutro)
							map.undoLogEnd()
							break

						case Map.IntroSummary:
							map.undoLogBegin(qsTr("Összegzés intro/outro hozzáadása"))
							map.summaryIntroAdd(parentId, intId, isOutro)
							map.undoLogEnd()
							break

						case Map.IntroChapter:
							map.undoLogBegin(qsTr("Célpont intro/outro hozzáadása"))
							map.chapterIntroAdd(parentId, intId)
							map.undoLogEnd()
							break
						}
					}
				}
			}


			QGridLabel {
				visible: introId !== -1
				text: qsTr("Időtartam:")
			}

			QGridTextField {
				id: introSec
				visible: introId !== -1
				Layout.fillHeight: true
				Layout.fillWidth: false
				placeholderText: qsTr("MM:SS")

				onTextModified: introUpdate()

				validator: RegExpValidator { regExp: /^(\d\d:\d\d|)$/ }
			}

			QGridLabel {
				visible: introId !== -1
				text: qsTr("Minimum szint:")
			}

			QGridSpinBox {
				id: introLevelMin
				sqlField: "levelMin"
				visible: introId !== -1

				onValueModified: introUpdate()

				from: 0
				to: 99
			}


			QGridLabel {
				visible: introId !== -1
				text: qsTr("Maximum szint:")
			}

			QGridSpinBox {
				id: introLevelMax
				sqlField: "levelMax"
				visible: introId !== -1

				onValueModified: introUpdate()

				from: 0
				to: 99
			}

			QGridButton {
				text: isOutro ? qsTr("Outro törlése") : qsTr("Intro törlése")
				visible: introId !== -1

				icon.source: CosStyle.iconDelete
				themeColors: CosStyle.buttonThemeDelete

				onClicked: {
					var d = JS.dialogCreateQml("YesNo")
					d.item.title = isOutro ? qsTr("Biztosan törlöd az outrot?") : qsTr("Biztosan törlöd az introt?")
					d.item.text = introText.text
					d.accepted.connect(function () {
						map.undoLogBegin(qsTr("Intro/Outro törlése"))
						if (map.introRemove(introId)) {
							introId = -1
							introRemoved()
						}
						map.undoLogEnd()
					})
					d.open()
				}
			}

		}
	}


	QCollapsible {
		title: isOutro ? qsTr("Outro szövege") : qsTr("Intro szövege")

		visible: introId !== -1

		QGridLayout {
			width: parent.width
			watchModification: false

			QGridLabel { text: qsTr("Szöveg") }

			QGridTextArea {
				id: introText
				sqlField: "ttext"

				onTextModified: introUpdate()
			}
		}
	}

	QCollapsible {
		title: isOutro ? qsTr("Outro médiatartalma") : qsTr("Intro médiatartalma")

		visible: introId !== -1

		QGridLayout {
			width: parent.width
			watchModification: false


			QGridLabel { field: introImg }

			QGridTextField {
				id: introImg
				fieldName: qsTr("Kép URL")
				sqlField: "img"

				onTextModified: introUpdate()

				validator: RegExpValidator { regExp: /^((ftp|http|https):\/\/[^ "]+|)$/ }
			}

			QGridLabel { field: introMedia }

			QGridTextField {
				id: introMedia
				fieldName: qsTr("Média URL")
				sqlField: "media"

				onTextModified: introUpdate()

				validator: RegExpValidator { regExp: /^((ftp|http|https):\/\/[^ "]+|)$/ }
			}

		}
	}




	Connections {
		target: map
		onIntroUpdated: if (id===introId) get()
		onUndone: get()
	}

	onIntroIdChanged: get()


	function introUpdate() {
		if (introId === -1)
			return

		if (!introImg.acceptableInput ||
				!introMedia.acceptableInput ||
				!introSec.acceptableInput)
			return

		var m = JS.getSqlFields([introText, introImg, introMedia, introLevelMin, introLevelMax])
		m.sec = JS.mmSStoSec(introSec.text)

		map.undoLogBegin(qsTr("Intro/outro módosítása"))
		map.introUpdate(introId, m, parentId, parentType)
		map.undoLogEnd()
	}



	function get() {
		var p

		if (map) {
			p = map.introGet(introId)
			introId = p.id
		} else
			introId = -1

		if (introId == -1) {
			introText.text = ""
			introImg.text = ""
			introMedia.text = ""
			introSec.text = ""
			introLevelMin.value = 0
			introLevelMax.value = 0

			return
		}

		if (!p)
			return

		introText.text = p.ttext
		introImg.text = p.img
		introMedia.text = p.media
		introSec.text = JS.secToMMSS(p.sec)
		introLevelMin.value = Number(p.levelMin)
		introLevelMax.value = Number(p.levelMax)
	}
}
