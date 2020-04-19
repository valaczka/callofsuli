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


	Label {
		id: noLabel
		opacity: introId == -1
		visible: opacity != 0

		text: qsTr("Válassz introt/outrot")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}


	QAccordion {
		id: accordion

		anchors.fill: parent

		opacity: introId != -1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			title: qsTr("Kapcsolatok")

			Column {
				width: parent.width

				QTag {
					id: tagCampaigns
					title: qsTr("Hadjárat:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"
					readOnly: true
					visible: tags.length
				}

				QTag {
					id: tagMissions
					title: qsTr("Küldetés:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"
					readOnly: true
					visible: tags.length
				}

				QTag {
					id: tagSummaries
					title: qsTr("Összegzés:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"
					readOnly: true
					visible: tags.length
				}

				QTag {
					id: tagChapters
					title: qsTr("Célpont:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"
					readOnly: true
					visible: tags.length
				}
			}

		}

		MapEditorIntroWidget {
			id: mIntro

			visible: panel.introId !== -1

			map: panel.map
			parentId: panel.parentId
			parentType: panel.parentType
			introId: panel.introId

			onIntroRemoved: {
				panel.introId = -1
			}
		}
	}


	Connections {
		target: pageEditor
		onIntroSelected: {
			introId = id
			mIntro.introId = id
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
			tagCampaigns.tags = []
			tagMissions.tags = []
			tagSummaries.tags = []
			tagChapters.tags = []
			return
		}

		var p = map.introGet(introId)

		tagCampaigns.tags = p.campaigns
		tagMissions.tags = p.missions
		tagSummaries.tags = p.summaries
		tagChapters.tags = p.chapters
	}
}
