import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import QtQuick.Layouts 1.12
import "."
import "Style"
import "JScript.js" as JS

Column {
	id: panel

	property Map map: null
	property int introId: -1
	property bool isOutro: false
	property int parentId: -1
	property int parentType: Map.IntroUndefined

	width: parent.width

	signal introRemoved()

	QCollapsible {
		title: isOutro ? qsTr("Outro beállításai") : qsTr("Intro beállításai")

		QGridLayout {
			id: grid
			width: parent.width
			watchModification: false

			QGridButton {
				label: isOutro ? qsTr("Outro hozzáadása") : qsTr("Intro hozzáadása")
				visible: introId === -1
				enabled: parentType !== Map.IntroUndefined

				onClicked: {
					var intId = map.introAdd({ttext: ""})
					if (intId !== -1) {
						introId = intId
						switch (parentType) {
						case Map.IntroCampaign:
							map.campaignIntroAdd(parentId, intId, isOutro)
							break

						case Map.IntroMission:
							map.missionIntroAdd(parentId, intId, isOutro)
							break

						case Map.IntroSummary:
							map.summaryIntroAdd(parentId, intId, isOutro)
							break

						case Map.IntroChapter:
							map.chapterIntroAdd(parentId, intId)
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

				onEditingFinished: introUpdate()

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
				label: isOutro ? qsTr("Outro törlése") : qsTr("Intro törlése")
				visible: introId !== -1

				icon: "M\ue5cd"
				bgColor: CosStyle.colorErrorDarker
				borderColor: CosStyle.colorErrorDark

				onClicked: {
					var d = JS.dialogCreateQml("YesNo")
					d.item.title = isOutro ? qsTr("Biztosan törlöd az outrot?") : qsTr("Biztosan törlöd az introt?")
					d.item.text = introText.text
					d.accepted.connect(function () {
						if (map.introRemove(introId)) {
							introId = -1
							introRemoved()
						}
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

				onEditingFinished: introUpdate()
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

				onEditingFinished: introUpdate()

				validator: RegExpValidator { regExp: /^((ftp|http|https):\/\/[^ "]+|)$/ }
			}

			QGridLabel { field: introMedia }

			QGridTextField {
				id: introMedia
				fieldName: qsTr("Média URL")
				sqlField: "media"

				onEditingFinished: introUpdate()

				validator: RegExpValidator { regExp: /^((ftp|http|https):\/\/[^ "]+|)$/ }
			}

		}
	}




	Connections {
		target: map
		onIntroUpdated: if (id===introId) get()
		onUndone: get()
	}

	Component.onCompleted: get()

	onIntroIdChanged: get()


	function introUpdate() {
		if (introId === -1)
			return

		if (!introImg.acceptableInput ||
				!introMedia.acceptableInput ||
				!introSec.acceptableInput)
			return

		var m = JS.getSqlFields([introText, introImg, introMedia, introLevelMin, introLevelMax],
								true)
		m.sec = JS.mmSStoSec(introSec.text)

		map.introUpdate(introId, m, parentId, parentType)
	}



	function get() {
		if (introId == -1 || !map) {
			introText.text = ""
			introImg.text = ""
			introMedia.text = ""
			introSec.text = ""
			introLevelMin.value = 0
			introLevelMax.value = 0

			return
		}

		var p = map.introGet(introId)

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
