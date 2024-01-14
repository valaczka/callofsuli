import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

Item {
	id: control

	property TeacherGroup group: null
	property Campaign campaign: null
	property TeacherMapHandler mapHandler: null

	QScrollable {
		anchors.fill: parent

		QFormColumn {
			id: _form

			spacing: 3

			QExpandableHeader {
				width: parent.width
				text: qsTr("Alapadatok")
				icon: Qaterial.Icons.serverOutline
				button.visible: false
				topPadding: 10 * Qaterial.Style.pixelSizeRatio
			}

			Qaterial.TextField {
				id: _tfName

				width: parent.width
				leadingIconSource: Qaterial.Icons.renameBox
				leadingIconInline: true
				title: qsTr("A kihívás neve")
				readOnly: !campaign || campaign.state >= Campaign.Finished
				helperText: campaign && campaign.state >= Campaign.Finished ? qsTr("A kihívás véget ért, a név már nem módosítható") : ""

				trailingContent: Qaterial.TextFieldButtonContainer
				{
					QTextFieldInPlaceButtons {
						setTo: campaign ? campaign.description : ""
						onSaveRequest: text => {
							Client.send(HttpConnection.ApiTeacher, "campaign/%1/update".arg(campaign.campaignid),
										{
											description: text
										})
							.done(control, function(r){
								reloadCampaign()
								saved()
							})
							.fail(control, function(err) {
								Client.messageWarning(err, qsTr("Módosítás sikertelen"))
								revert()
							})
						}
					}
				}
			}


			QDateTimePicker {
				width: parent.width
				visible: campaign && (campaign.state < Campaign.Running || campaign.startTime.getTime())
				canEdit: campaign && campaign.state < Campaign.Running
				title: campaign && campaign.state < Campaign.Running ? qsTr("Automatikus indítás") : qsTr("Indítás ideje")
				helperText: campaign && campaign.state >= Campaign.Running ? qsTr("A kihívás már elindult, az időpont nem módosítható") : ""
				onSaveRequest: text => {
					Client.send(HttpConnection.ApiTeacher, "campaign/%1/update".arg(campaign.campaignid),
								{
									starttime: hasDate ? Math.floor(date.getTime()/1000) : -1
								})
					.done(control, function(r){
						reloadCampaign()
						saved()
					})
					.fail(control, function(err) {
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
				width: parent.width
				visible: campaign && (campaign.state < Campaign.Finished || campaign.endTime.getTime())
				canEdit: campaign && campaign.state < Campaign.Finished
				title: campaign && campaign.state < Campaign.Finished ? qsTr("Automatikus befejezés") : qsTr("Befejezés ideje")
				helperText: campaign && campaign.state >= Campaign.Finished ? qsTr("A kihívás már befejeződött, az időpont nem módosítható") : ""
				hour: 23
				minute: 59
				onSaveRequest: text => {
					Client.send(HttpConnection.ApiTeacher, "campaign/%1/update".arg(campaign.campaignid),
								{
									endtime: hasDate ? Math.floor(date.getTime()/1000) : -1
								})
					.done(control, function(r){
						reloadCampaign()
						saved()
					})
					.fail(control, function(err) {
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



			QExpandableHeader {
				width: parent.width
				text: qsTr("Értékelés")
				icon: Qaterial.Icons.bullseyeArrow
				button.visible: false
				topPadding: 20 * Qaterial.Style.pixelSizeRatio
			}




			Row {
				anchors.left: parent.left

				spacing: 5

				Qaterial.LabelBody1 {
					anchors.verticalCenter: parent.verticalCenter
					text: qsTr("Alapértelmezett jegy:")
				}

				Qaterial.ComboBox {
					id: _combo

					flat: false
					enabled: campaign && campaign.state < Campaign.Finished

					font: Qaterial.Style.textTheme.body1

					width: Math.max(implicitWidth, Qaterial.Style.pixelSizeRatio * 250)

					anchors.verticalCenter: parent.verticalCenter

					model: ListModel {
						id: comboModel
					}

					valueRole: "gradeid"
					textRole: "fullname"

					Component.onCompleted: {
						comboModel.append({gradeid: -1, fullname: qsTr("-- nincs --")})

						var m = Client.cache("gradeList")
						for (var i=0; i<m.length; ++i) {
							var o = m.get(i)
							comboModel.append({gradeid: o.gradeid, fullname: "%1 (%2)".arg(o.longname).arg(o.shortname)})
						}

						setFromCampaign()
					}


					Connections {
						target: campaign

						function onDefaultGradeChanged() {
							_combo.setFromCampaign()
						}
					}


					function setFromCampaign() {
						if (!campaign) {
							_buttons.set(-1)
							return
						}

						if (!campaign.defaultGrade) {
							_buttons.set(0)
							return
						}

						for (var i=1; i<model.count; ++i) {
							if (model.get(i).gradeid === campaign.defaultGrade.gradeid) {
								_buttons.set(i)
								return
							}
						}
					}
				}


				QComboBoxInPlaceButtons {
					id: _buttons
					combo: _combo

					visible: true
					enabled: active
					opacity: active ? 1.0 : 0.0

					anchors.verticalCenter: parent.verticalCenter

					onSaveRequest: text => {
						if (!campaign)
							return

						if (campaign.state >= Campaign.Finished) {
							Client.messageWarning(qsTr("A kihívás már véget ért, az alapértelmezett jegy nem módosítható"),
												  qsTr("Alapértelmezett jegy"))
							return
						} else if (campaign.state >= Campaign.Running) {

							JS.questionDialog(
										{
											onAccepted: function()
											{
												_save()
											},
											onRejected: function() {
												revert()
											},
											text: qsTr("A kihívás már elindult, biztosan megváltoztatod az alapértelmezett jegyet?"),
											title: qsTr("Alapértelmezett jegy"),
											iconSource: Qaterial.Icons.progressQuestion
										})
							return
						}

						_save()

					}

					function _save() {
						Client.send(HttpConnection.ApiTeacher, "campaign/%1/update".arg(campaign.campaignid),
									{
										defaultGrade: _combo.currentValue
									})
						.done(control, function(r){
							reloadCampaign()
							saved()
						})
						.fail(control, function(err) {
							Client.messageWarning(err, qsTr("Módosítás sikertelen"))
							revert()
						})
					}
				}
			}


			QLabelInformative {
				text: qsTr("A kihívás értékeléséhez vegyél fel kritériumokat. A játékosok csak olyan pályákon tudnak játszani, melyek valamelyik kritériumhoz tartoznak. Legalább egy pályára/küldetésre vonatkozó kritérium szükséges a játékhoz!")
				visible: !_rptr.model || !_rptr.model.length

				topPadding: 30 * Qaterial.Style.pixelSizeRatio
				bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
			}

			QButton {
				action: actionAddTask
				visible: !_rptr.model || !_rptr.model.length
				anchors.horizontalCenter: parent.horizontalCenter
			}

			Column {
				width: parent.width

				Repeater {
					id: _rptr

					delegate: Item {
						width: parent.width
						height: task ? _delegate.implicitHeight : _section.implicitHeight

						readonly property Task task: modelData.task

						Qaterial.ListSectionTitle {
							id: _section
							width: parent.width
							text: modelData.section
							visible: !task
						}

						Qaterial.ItemDelegate {
							id: _delegate

							width: parent.width
							visible: task

							icon.source: task && task.required ? Qaterial.Icons.accessPointCheck : Qaterial.Icons.accessPoint
							highlightedIcon: task && task.required

							onClicked: {
								if (!campaign)
									return

								if (campaign.state >= Campaign.Finished) {
									Client.messageWarning(qsTr("A kihívás már véget ért, a kritérium nem módosítható"),
														  qsTr("Kritérium szerkesztése"))
									return
								} else if (campaign.state >= Campaign.Running) {
									JS.questionDialog(
												{
													onAccepted: function()
													{
														editTask()
													},
													text: qsTr("A kihívás már elindult, biztosan módosítod a kritériumot?"),
													title: qsTr("Kritérium szerkesztése"),
													iconSource: Qaterial.Icons.progressQuestion
												})
									return
								}

								editTask()
							}

							Connections {
								target: task

								function onCriterionChanged() { _delegate.getText() }
								function onMapUuidChanged() { _delegate.getText() }
							}

							Component.onCompleted: getText()

							function getText() {
								text = task ? task.readableCriterion(mapHandler ? mapHandler.mapList : null) : ""
							}

							function editTask() {
								let o = Client.stackPushPage("TeacherCampaignTaskEdit.qml", {
																 campaign: campaign,
																 mapHandler: mapHandler,
																 task: task
															 })

								if (o)
									o.Component.destruction.connect(reloadCampaign)
							}

						}


					}
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
		enabled: campaign && campaign.state < Campaign.Finished
		onTriggered: {
			if (!campaign || campaign.state >= Campaign.Finished)
				return

			if (campaign.state >= Campaign.Running) {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									addTask()
								},
								text: qsTr("A kihívás már elindult, biztosan létrehozol egy új kritériumot?"),
								title: qsTr("Új kritérium"),
								iconSource: Qaterial.Icons.progressQuestion
							})
				return
			}

			addTask()
		}

		function addTask() {
			let o = Client.stackPushPage("TeacherCampaignTaskEdit.qml", {
											 campaign: campaign,
											 mapHandler: mapHandler
										 })

			if (o)
				o.Component.destruction.connect(reloadCampaign)
		}
	}


	onCampaignChanged: reloadTaskList()

	Connections {
		target: campaign

		function onTaskListReloaded() {
			reloadTaskList()
		}
	}



	function reloadCampaign() {
		if (!campaign)
			return

		Client.send(HttpConnection.ApiTeacher, "campaign/%1".arg(campaign.campaignid))
		.done(control, function(r){
			if (r.id !== campaign.campaignid) {
				Client.messageWarning(qsTr("Érvénytelen kihívás"), qsTr("Belső hiba"))
				return
			}

			campaign.loadFromJson(r, true)
		})
		.fail(control, JS.failMessage(qsTr("Kihívás letöltése sikertelen")))
	}


	function reloadTaskList() {
		if (campaign)
			_rptr.model = campaign.getOrderedTaskListModel()
		else
			_rptr.model = null
	}
}
