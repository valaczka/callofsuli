import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	property TeacherGroup group: null
	property Campaign campaign: null
	property TeacherMapHandler mapHandler: null

	title: campaign ? campaign.readableName : ""
	subtitle: group ? group.fullName : ""

	appBar.backButtonVisible: true

	TeacherCampaignResult {
		id: _result
		anchors.fill: parent
		group: control.group
		campaign: control.campaign
		mapHandler: control.mapHandler
	}

	StackView.onActivated: _result.resultModel.reloadContent()
}
