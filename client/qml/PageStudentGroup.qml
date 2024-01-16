import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPageGradient {
	id: control

	stackPopFunction: function() {
		var item = swipeView.currentItem

		if (item && item.stackPopFunction !== undefined) {
			return item.stackPopFunction()
		}

		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		return true
	}


	//title: group ? group.name : ""

	property StudentGroup group: null
	property StudentMapHandler mapHandler: null

	appBar.backButtonVisible: true

	progressBarEnabled: true


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		StudentGroupCampaignList {
			group: control.group
			mapHandler: control.mapHandler
			topPadding: control.paddingTop
		}

		StudentGroupExamList {
			group: control.group
			mapHandler: control.mapHandler
			topPadding: control.paddingTop
		}

		StudentGroupMemberList {
			group: control.group
			topPadding: control.paddingTop
		}



	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Kihívások"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			model.append({ text: qsTr("Dolgozatok"), source: Qaterial.Icons.fileDocumentMultiple, color: "red" })
			model.append({ text: qsTr("Résztvevők"), source: Qaterial.Icons.accountSupervisor, color: "green" })
			/*model.append({ text: qsTr("Hadjáratok"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			*/
		}
	}

}
