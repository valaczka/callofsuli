import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import QtQuick.Layouts 1.12
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null
	property int introId: -1
	property int parentId: -1
	property int parentType: Map.IntroUndefined


	title: qsTr("Intro/Outro")

	rightLoader.sourceComponent: QCloseButton {
		onClicked: if (view) {
					   view.model.remove(modelIndex)
				   }
	}


	QAccordion {
		id: accordion

		anchors.fill: parent

		QCollapsible {
			title: qsTr("Szöveg")

			QGridLayout {
				width: parent.width
				watchModification: false

				QGridLabel { text: qsTr("Bevezető szövege") }

				QGridTextArea {
					id: introText
					sqlField: "ttext"

					onEditingFinished: grid.introUpdate()
				}
			}
		}




		QCollapsible {
			title: qsTr("Médiatartalom")

			QGridLayout {
				width: parent.width
				watchModification: false


				QGridLabel { field: introImg }

				QGridTextField {
					id: introImg
					fieldName: qsTr("Kép URL")
					sqlField: "img"

					onEditingFinished: grid.introUpdate()

					validator: RegExpValidator { regExp: /^((ftp|http|https):\/\/[^ "]+|)$/ }
				}

				QGridLabel { field: introMedia }

				QGridTextField {
					id: introMedia
					fieldName: qsTr("Média URL")
					sqlField: "media"

					onEditingFinished: grid.introUpdate()

					validator: RegExpValidator { regExp: /^((ftp|http|https):\/\/[^ "]+|)$/ }
				}

			}
		}


		QCollapsible {
			title: qsTr("Beállítások")

			QGridLayout {
				id: grid
				width: parent.width
				watchModification: false


				QGridLabel { text: qsTr("Időtartam:") }

				QGridTextField {
					id: introSec
					Layout.fillHeight: true
					Layout.fillWidth: false
					placeholderText: qsTr("MM:SS")

					onEditingFinished: grid.introUpdate()

					validator: RegExpValidator { regExp: /^(\d\d:\d\d|)$/ }
				}

				QGridLabel { text: qsTr("Minimum szint:") }

				QGridSpinBox {
					id: introLevelMin
					sqlField: "levelMin"

					onValueModified: grid.introUpdate()

					from: 0
					to: 99
				}


				QGridLabel { text: qsTr("Maximum szint:") }

				QGridSpinBox {
					id: introLevelMax
					sqlField: "levelMax"

					onValueModified: grid.introUpdate()

					from: 0
					to: 99
				}


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
			}
		}

		QCollapsible {
			title: qsTr("Kapcsolatok")

			Column {
				width: parent.width

				QTag {
					id: tagCampaigns
					title: qsTr("Hadjáratok:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"
				}

				QTag {
					id: tagMissions
					title: qsTr("Küldetések:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"
				}

				QTag {
					id: tagSummaries
					title: qsTr("Összegzések:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"
				}

				QTag {
					id: tagChapters
					title: qsTr("Célpontok:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"
				}
			}

		}
	}


	Connections {
		target: pageEditor
		onIntroSelected: {
			introId = id
		}
	}

	Connections {
		target: map
		onIntroUpdated: if (id===introId) get()
	}

	Component.onCompleted: get()

	onIntroIdChanged: get()

	function get() {
		if (introId == -1 || !map) {
			introText.text = ""
			introImg.text = ""
			introMedia.text = ""
			introSec.text = ""
			introLevelMin.value = 0
			introLevelMax.value = 0

			tagCampaigns.tags = []
			tagMissions.tags = []
			tagSummaries.tags = []
			tagChapters.tags = []
			return
		}

		var p = map.introGet(introId)

		introText.text = p.ttext
		introImg.text = p.img
		introMedia.text = p.media
		introSec.text = JS.secToMMSS(p.sec)
		introLevelMin.value = Number(p.levelMin)
		introLevelMax.value = Number(p.levelMax)

		tagCampaigns.tags = p.campaigns
		tagMissions.tags = p.missions
		tagSummaries.tags = p.summaries
		tagChapters.tags = p.chapters
	}
}
