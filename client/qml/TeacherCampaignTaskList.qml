import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QScrollable {
	id: root

	property TeacherGroup group: null
	property Campaign campaign: null
	property TeacherMapHandler mapHandler: null
	property Action actionTaskCreate: null

	signal reloadRequest()

	Row {
		width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
		anchors.horizontalCenter: parent.horizontalCenter

		spacing: 5

		Qaterial.LabelBody1 {
			text: qsTr("Alapértelmezett jegy:")
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.ComboBox {
			id: gradeCombo

			flat: false
			editable: false

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
					gradeCombo.setFromCampaign()
				}
			}


			function setFromCampaign() {
				if (!campaign) {
					comboGradeInPlaceButtons.set(-1)
					return
				}

				if (!campaign.defaultGrade) {
					comboGradeInPlaceButtons.set(0)
					return
				}

				for (var i=1; i<model.count; ++i) {
					if (model.get(i).gradeid === campaign.defaultGrade.gradeid) {
						comboGradeInPlaceButtons.set(i)
						return
					}
				}
			}
		}


		QComboBoxInPlaceButtons {
			id: comboGradeInPlaceButtons
			combo: gradeCombo

			visible: true
			enabled: active
			opacity: active ? 1.0 : 0.0

			anchors.verticalCenter: parent.verticalCenter

			onSaveRequest: {
				Client.send(WebSocket.ApiTeacher, "campaign/%1/update".arg(campaign.campaignid),
							{
								defaultGrade: gradeCombo.currentValue
							})
				.done(function(r){
					reloadRequest()
					saved()
				})
				.fail(function(err) {
					Client.messageWarning(err, qsTr("Módosítás sikertelen"))
					revert()
				})
			}
		}
	}

	QIconLabel {
		text: qsTr("Értékelési kritériumok")
		anchors.left: _colTaskList.left
		font: Qaterial.Style.textTheme.headline5
		topPadding: 20
		bottomPadding: 10
		icon.source: Qaterial.Icons.bullseyeArrow
	}



	onCampaignChanged: _colTaskList.reload()

	Connections {
		target: campaign

		function onTaskListReloaded() {
			_colTaskList.reload()
		}
	}



	Column {
		id: _colTaskList

		width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
		anchors.horizontalCenter: parent.horizontalCenter

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
						let o = Client.stackPushPage("TeacherCampaignTaskEdit.qml", {
														 campaign: campaign,
														 mapHandler: mapHandler,
														 task: task
													 })

						if (o)
							o.Component.destruction.connect(reloadRequest)
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

				}


			}
		}


		function reload() {
			if (campaign)
				_rptr.model = campaign.getOrderedTaskListModel()
		}
	}


	StackView.onStatusChanged: if (actionTaskCreate) actionTaskCreate.enabled = (StackView.status == StackView.Active)

	Component.onDestruction: if (actionTaskCreate) actionTaskCreate.enabled = false


}
