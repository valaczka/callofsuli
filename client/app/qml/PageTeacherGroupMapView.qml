import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: page

	defaultTitle: ""
	defaultSubTitle: ""

	property TeacherGroups teacherGroups: null
	property string mapUuid: ""

	property int delegateHeight: CosStyle.twoLineHeight
	property int delegateWidth: 60

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}


	/*mainMenuFunc: function (m) {
		m.addAction(actionMapAdd)
	}*/


	GameMapModel {
		id: mapModel
	}

	TableView {
		id: topHeaderView
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.left: leftHeaderView.right
		height: 150

		syncDirection: Qt.Horizontal
		syncView: view
		clip: true

		boundsBehavior: Flickable.StopAtBounds

		model: mapModel.headerModelTop

		topMargin: delegateHeight

		Row {
			y: -height
			spacing: view.columnSpacing

			Repeater {
				model: mapModel.missionsData()

				Rectangle {
					width: modelData.levels*delegateWidth+view.columnSpacing*(modelData.levels-1)
					height: delegateHeight
					color: "yellow"

					QLabel {
						anchors.centerIn: parent
						text: modelData.name
					}
				}
			}
		}


		delegate: Rectangle {
			implicitHeight: delegateHeight
			implicitWidth: delegateWidth
			color: "black"

			QLabel {
				anchors.centerIn: parent
				text: display.level+(display.deathmatch ? "D" : "")
			}
		}
	}





	TableView {
		id: leftHeaderView
		anchors.left: parent.left
		anchors.top: topHeaderView.bottom
		anchors.bottom: parent.bottom

		width: 400

		boundsBehavior: Flickable.StopAtBounds

		syncDirection: Qt.Vertical
		syncView: view
		clip: true

		model: mapModel.headerModelLeft

		delegate: Rectangle {
			implicitHeight: delegateHeight
			implicitWidth: leftHeaderView.width
			color: "blue"

			QLabel {
				anchors.centerIn: parent
				text: display.firstname+" "+display.lastname+" "+display.xp
			}
		}
	}



	TableView {
		id: view

		anchors.top: topHeaderView.bottom
		anchors.left: leftHeaderView.right
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		model: mapModel

		columnSpacing: 2
		rowSpacing: 5

		boundsBehavior: Flickable.StopAtBounds

		clip: true

		ScrollIndicator.vertical: ScrollIndicator { }
		ScrollIndicator.horizontal: ScrollIndicator { }

		delegate: Rectangle {
			implicitHeight: delegateHeight
			implicitWidth: delegateWidth
			color: success ? "green" : "black"

			QLabel {
				anchors.centerIn: parent
				text: display
			}
		}

	}


	onMapUuidChanged: {
		if (teacherGroups)
			teacherGroups.loadMapDataToModel(mapUuid, mapModel)
	}

	onTeacherGroupsChanged: {
		if (mapUuid.length && teacherGroups)
			teacherGroups.loadMapDataToModel(mapUuid, mapModel)
	}


	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}

