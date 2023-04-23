import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

Qaterial.Page {
	id: control

	implicitWidth: 200
	implicitHeight: 200

	background: Rectangle { color: "transparent" }

	property TeacherGroup group: null
	property Campaign campaign: null
	property TeacherMapHandler mapHandler: null

	readonly property color _doneColor: Qaterial.Colors.green500
	readonly property real _stepperSize: 48
	readonly property real _stepperPadding: 24

	property QTextFieldInPlaceButtons _inPlaceButtons: null

	property var stackPopFunction: function() {
		if (_inPlaceButtons && _inPlaceButtons.active) {
			_inPlaceButtons.revert()
			return false
		}

		return true
	}

	Qaterial.TextField {
		id: textFieldCampaignDescription
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		font: Qaterial.Style.textTheme.headline4
		leadingIconSource: Qaterial.Icons.accountMultiple
		leadingIconInline: true
		placeholderText: qsTr("A hadjárat neve")
		backgroundBorderHeight: 1
		backgroundColor: "transparent"
		trailingContent: QTextFieldInPlaceButtons {
			id: textFieldDescriptionInPlaceButtons
			y: Math.max(textFieldCampaignDescription.height-height)/2
			setTo: campaign.description
			onSaveRequest: {
				Client.send(WebSocket.ApiTeacher, "campaign/%1/update".arg(campaign.campaignid),
							{
								description: text
							})
				.done(function(r){
					reloadCampaign()
					saved()
				})
				.fail(function(err) {
					Client.messageWarning(err, qsTr("Módosítás sikertelen"))
					revert()
				})
			}

			Component.onCompleted: control._inPlaceButtons = textFieldDescriptionInPlaceButtons

		}

	}

	Qaterial.Stepper {
		id: stepper

		anchors.top: textFieldCampaignDescription.bottom
		anchors.topMargin: _stepperPadding
		anchors.left: parent.left
		anchors.right: parent.right

		indicatorHeight: _stepperSize
		indicatorWidth: _stepperSize

		clickable: true


		onCurrentElementChanged: {
			if (currentElement)
				stack.replace(currentElement.cmp)
			else
				stack.clear()
		}

		model: Qaterial.StepperModel {
			Qaterial.StepperElement {
				text: qsTr("Előkészítés")
				done: campaign && campaign.state >= Campaign.Prepared
				property string iconName: Qaterial.Icons.battery10
				property Component cmp: cmpDetails
			}

			Qaterial.StepperElement {
				text: qsTr("Időzítés")
				done: campaign && campaign.state >= Campaign.Running
				property string iconName: Qaterial.Icons.battery30
				property Component cmp: cmpStart
			}

			Qaterial.StepperElement {
				text: qsTr("Befejezés")
				done: campaign && campaign.state >= Campaign.Finished
				property string iconName: Qaterial.Icons.battery60Bluetooth
				property Component cmp: cmpFinish
			}
		}

		separator: Rectangle
		{
			property Qaterial.StepperElement previous
			property Qaterial.StepperElement next
			property int index
			property bool highlighted:
			{
				if (previous == null)
					return next.done
				if (next == null)
					return previous.done
				return previous.done && next.done
			}
			color: highlighted ? _doneColor : Qaterial.Style.dividersColor()
			height: 8
			radius: 4
		}


		indicator: Qaterial.ColorIcon
		{
			anchors.centerIn: parent
			iconSize: _stepperSize

			property Qaterial.StepperElement element
			property bool done: element ? element.done : false

			color: done ? _doneColor : Qaterial.Style.dividersColor()
			source: element.iconName
		}

		contentItem: Qaterial.LabelBody2
		{
			width: 100
			height: 20

			property Qaterial.StepperElement element
			property int index
			property bool done: element ? element.done : false

			readonly property bool isCurrent: index === stepper.currentIndex

			text: element.text
			font.bold: isCurrent
			horizontalAlignment: stepper.vertical ? Text.AlignLeft : Text.AlignHCenter
			color: isCurrent ? Qaterial.Style.accentColor : done ? _doneColor : Qaterial.Style.primaryTextColor()

		}
	}

	Qaterial.StackView {
		id: stack
		anchors.top: stepper.bottom
		anchors.topMargin: _stepperPadding
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right
	}



	Component.onCompleted: {
		if (!campaign)
			return

		if (campaign.state == Campaign.Finished) {
			stepper.currentIndex = stepper.count-1
			return
		}

		for (var i=0; i<=stepper.count-1; i++) {
			var o = stepper.model.get(i)

			if (!o || !o.done) {
				stepper.currentIndex = i
				return
			}
		}
	}



	// ----- DETAILS COMPONENT

	Component {
		id: cmpDetails

		TeacherCampaignTaskList {
			group: control.group
			campaign: control.campaign
			mapHandler: control.mapHandler
			actionTaskCreate: actionAddTask
			onReloadRequest: reloadCampaign()
		}
	}

	// ----- START COMPONENT

	Component {
		id: cmpStart

		QScrollable {
			contentCentered: true

			spacing: 10

			QDateTimePicker {
				anchors.horizontalCenter: parent.horizontalCenter
				visible: campaign && (campaign.state < Campaign.Running || campaign.startTime.getTime())
				canEdit: campaign && campaign.state < Campaign.Running
				title: campaign && campaign.state < Campaign.Running ? qsTr("Automatikus indítás") : qsTr("Indítás ideje")
				onSaveRequest: {
					Client.send(WebSocket.ApiTeacher, "campaign/%1/update".arg(campaign.campaignid),
								{
									starttime: hasDate ? Math.floor(date.getTime()/1000) : -1
								})
					.done(function(r){
						reloadCampaign()
						saved()
					})
					.fail(function(err) {
						Client.messageWarning(err, qsTr("Módosítás sikertelen"))
						revert()
					})
				}
				Component.onCompleted: {
					hasDate = campaign.startTime.getTime()
					if (hasDate)
						setFromDateTime(campaign.startTime)
				}
			}

			QDateTimePicker {
				anchors.horizontalCenter: parent.horizontalCenter
				visible: campaign && (campaign.state < Campaign.Finished || campaign.endTime.getTime())
				canEdit: campaign && campaign.state < Campaign.Finished
				title: campaign && campaign.state < Campaign.Finished ? qsTr("Automatikus befejezés") : qsTr("Befejezés ideje")
				onSaveRequest: {
					Client.send(WebSocket.ApiTeacher, "campaign/%1/update".arg(campaign.campaignid),
								{
									endtime: hasDate ? Math.floor(date.getTime()/1000) : -1
								})
					.done(function(r){
						reloadCampaign()
						saved()
					})
					.fail(function(err) {
						Client.messageWarning(err, qsTr("Módosítás sikertelen"))
						revert()
					})
				}
				Component.onCompleted: {
					hasDate = campaign.endTime.getTime()
					if (hasDate)
						setFromDateTime(campaign.endTime)
				}
			}

			QButton {
				icon.source: Qaterial.Icons.play
				text: qsTr("Start")
				anchors.horizontalCenter: parent.horizontalCenter

				enabled: campaign
				visible: campaign && campaign.state < Campaign.Running
				onClicked: {
					Client.send(WebSocket.ApiTeacher, "campaign/%1/run".arg(campaign.campaignid))
					.done(function(r){
						reloadCampaign()
					})
					.fail(JS.failMessage(qsTr("Hadjárat indítása sikertelen")))
				}
			}

			Qaterial.LabelBody1 {
				id: _labelStart
				color: Qaterial.Colors.lightGreen400
				width: parent.width
				horizontalAlignment: Qt.AlignHCenter
				leftPadding: 20
				rightPadding: 20
				wrapMode: Text.Wrap
				visible: _startTimer.running

				Timer {
					id: _startTimer
					interval: 500
					triggeredOnStart: true
					running: campaign && campaign.startTime.getTime() && campaign.state < Campaign.Running
					repeat: true
					onTriggered: {
						var diff = campaign.startTime.getTime() - new Date().getTime()
						if (diff <= 0) {
							stop()
							return
						}

						diff = Math.floor(diff/1000)

						var s = diff%60
						diff = Math.floor(diff/60)
						var m = diff%60
						diff = Math.floor(diff/60)
						var h = diff%24
						diff = Math.floor(diff/24)

						var list = []

						if (diff > 0)
							list.push(qsTr("%1 nap").arg(diff))
						if (h > 0 || diff > 0)
							list.push(qsTr("%1 óra").arg(h))
						if (m > 0 || h > 0)
							list.push(qsTr("%1 perc").arg(m))
						if (s > 0 || m > 0)
							list.push(qsTr("%1 másodperc").arg(s))

						_labelStart.text = qsTr("Indításig hátralévő idő:\n")+list.join(" ")
					}
				}
			}
		}

	}


	// ----- FINISH COMPONENT

	Component {
		id: cmpFinish

		QScrollable {
			contentCentered: true

			spacing: 10

			Qaterial.IconLabel {
				anchors.horizontalCenter: parent.horizontalCenter
				visible: campaign && campaign.state >= Campaign.Running
				font: Qaterial.Style.textTheme.headline3
				color: Qaterial.Colors.lightGreen400
				text: campaign && campaign.state == Campaign.Finished ? qsTr("A hadjárat véget ért") : qsTr("A hadjárat folyamatban van")
				icon.source: campaign && campaign.state == Campaign.Finished ? Qaterial.Icons.checkBold : Qaterial.Icons.play
				icon.width: 48
				icon.height: 48
			}

			Qaterial.LabelBody1 {
				id: _labelEnd
				color: Qaterial.Colors.lightGreen400
				width: parent.width
				horizontalAlignment: Qt.AlignHCenter
				leftPadding: 20
				rightPadding: 20
				wrapMode: Text.Wrap
				visible: _endTimer.running

				Timer {
					id: _endTimer
					interval: 500
					triggeredOnStart: true
					running: campaign && campaign.endTime.getTime() && campaign.state == Campaign.Running
					repeat: true
					onTriggered: {
						var diff = campaign.endTime.getTime() - new Date().getTime()
						if (diff <= 0) {
							stop()
							return
						}

						diff = Math.floor(diff/1000)

						var s = diff%60
						diff = Math.floor(diff/60)
						var m = diff%60
						diff = Math.floor(diff/60)
						var h = diff%24
						diff = Math.floor(diff/24)

						var list = []

						if (diff > 0)
							list.push(qsTr("%1 nap").arg(diff))
						if (h > 0 || diff > 0)
							list.push(qsTr("%1 óra").arg(h))
						if (m > 0 || h > 0)
							list.push(qsTr("%1 perc").arg(m))
						if (s > 0 || m > 0)
							list.push(qsTr("%1 másodperc").arg(s))

						_labelEnd.text = qsTr("Leállításig hátralévő idő:\n")+list.join(" ")
					}
				}
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				icon.source: Qaterial.Icons.stop
				text: qsTr("Leállítás")
				enabled: campaign && campaign.state == Campaign.Running
				visible: campaign && campaign.state < Campaign.Finished
				onClicked: {
					JS.questionDialog({
										  onAccepted: function()
										  {
											  Client.send(WebSocket.ApiTeacher, "campaign/%1/finish".arg(campaign.campaignid))
											  .done(function(r){
												  reloadCampaign()
											  })
											  .fail(JS.failMessage(qsTr("Hadjárat befejezése sikertelen")))
										  },
										  title: qsTr("Hadjárat befejezése"),
										  text: qsTr("Biztosan befejezed a hadjáratot?")
									  })

				}
			}
		}

	}


	QFabButton {
		visible: actionAddTask.enabled
		action: actionAddTask
	}


	Action {
		id: actionAddTask
		text: qsTr("Új kritérium")
		icon.source: Qaterial.Icons.plus
		enabled: false
		//onTriggered: Client.stackPushPage("ServerEdit.qml")
		onTriggered: {
			Client.send(WebSocket.ApiTeacher, "campaign/%1/task/create".arg(campaign.campaignid), {
							criterion: {
								id: 244,
								name: "hello"
							},
							xp: 56
						})

			.done(function(r){
				reloadCampaign()
			})
			.fail(JS.failMessage(qsTr("Kritérium létrehozása sikertelen")))
		}
	}




	function reloadCampaign() {
		if (!campaign)
			return

		Client.send(WebSocket.ApiTeacher, "campaign/%1".arg(campaign.campaignid))
		.done(function(r){
			if (r.id !== campaign.campaignid) {
				Client.messageWarning(qsTr("Érvénytelen hadjárat"), qsTr("Belső hiba"))
				return
			}

			campaign.loadFromJson(r, true)
		})
		.fail(JS.failMessage(qsTr("Hadjárat letöltése sikertelen")))
	}

}
