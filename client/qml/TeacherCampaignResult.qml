import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS


QTableView {
	id: root

	property TeacherGroup group: null
	property Campaign campaign: null
	property TeacherMapHandler mapHandler: null

	columnWidthFunc: function(col) { return _model.isSection(col) ? 30 : 75 }


	model: TeacherGroupCampaignResultModel {
		id: _model
		teacherGroup: root.group
		campaign: root.campaign

		onModelReloaded: root.forceLayout()
	}

	delegate: Rectangle {
		color: "green"
		implicitWidth: 50
		implicitHeight: 50
		Qaterial.LabelCaption {
			anchors.centerIn: parent
			text: display
			color: "black"
		}
	}

	topHeaderDelegate: Rectangle {
		color: "white"
		implicitWidth: 50
		implicitHeight: 50
		Qaterial.LabelBody2 {
			anchors.centerIn: parent
			width: parent.height
			height: parent.width
			elide: Text.ElideRight
			wrapMode: Text.Wrap
			lineHeight: 0.9
			text: display
			color: "blue"
			rotation: -90
			maximumLineCount: 3
		}
	}

	leftHeaderDelegate: Rectangle {
		color: "white"
		implicitWidth: 50
		implicitHeight: 50
		Qaterial.LabelBody2 {
			anchors.fill: parent
			//elide: Text.ElideRight
			wrapMode: Text.Wrap
			maximumLineCount: 3
			lineHeight: 0.9
			text: display
			color: "red"
		}
	}

}
