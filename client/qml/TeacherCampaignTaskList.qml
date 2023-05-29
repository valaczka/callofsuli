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
		text: qsTr("Kritériumok")
		anchors.left: _colTaskList.left
		font: Qaterial.Style.textTheme.headline5
		topPadding: 20
		bottomPadding: 10
		icon.source: Qaterial.Icons.abacus
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

					Connections {
						target: task

						function onCriterionChanged() { getText() }
						function onMapUuidChanged() { getText() }
					}

					Component.onCompleted: getText()

					function getText() {
						text = task ?
								(task.readableCriterion(mapHandler ? mapHandler.mapList : null)+" - "+task.gradeValue+" / "+task.xp + " : "+task.readableGradeOrXp) : ""
					}

				}


			}
		}


		function reload() {
			if (campaign)
				_rptr.model = campaign.getOrderedTaskListModel()
		}
	}

	/*
	QListView {
		id: taskList

		currentIndex: -1
		autoSelectChange: true


		model: _list

		delegate: QItemDelegate {
			id: _delegate
			property Task task: model.qtObject
			selectableObject: task

			highlighted: ListView.isCurrentItem

			iconSource: task.required ? Qaterial.Icons.accessPointCheck : Qaterial.Icons.accessPoint
			highlightedIcon: task.required

			Connections {
				target: task

				function onCriterionChanged() { getText() }
				function onMapUuidChanged() { getText() }
			}


			Component.onCompleted: getText()

			function getText() {
				text = task.readableCriterion(mapHandler ? mapHandler.mapList : null)+" - "+task.gradeValue+" / "+task.xp + " : "+task.readableGradeOrXp
			}

		}

	}
*/
	StackView.onStatusChanged: if (actionTaskCreate) actionTaskCreate.enabled = (StackView.status == StackView.Active)

	Component.onDestruction: if (actionTaskCreate) actionTaskCreate.enabled = false


}
