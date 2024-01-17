import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	closeQuestion: _form.modified ? qsTr("Biztosan eldobod a módosításokat?") : ""

	property Campaign campaign: null
	property Task task: null
	property TeacherMapHandler mapHandler: null

	title: task ? qsTr("Kritérium #%1 szerkesztése").arg(task.taskid) : qsTr("Új kritérium")
	subtitle: campaign ? campaign.readableName : ""

	appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: task
		ToolTip.text: qsTr("Feladat törlése")
		icon.source: Qaterial.Icons.delete_
		onClicked: JS.questionDialog(
					   {
						   onAccepted: function()
						   {
							   Client.send(HttpConnection.ApiTeacher, "task/%1/delete".arg(task.taskid))
							   .done(control, function(r){
								   _form.modified = false
								   Client.stackPop(control)
							   })
							   .fail(control, JS.failMessage("Törlés sikertelen"))
						   },
						   text: qsTr("Biztosan törlöd a kritériumot?"),
						   title: campaign ? campaign.readableName : "",
						   iconSource: Qaterial.Icons.deleteCircle
					   })

	}

	QScrollable {
		anchors.fill: parent

		ButtonGroup {
			id: _radioGroup
		}

		QFormColumn {
			id: _form

			spacing: 5

			title: qsTr("Kritérium")

			Row {
				spacing: 5

				QFormRadioButton {
					id: _modeGrade
					text: qsTr("Jegy:")
					ButtonGroup.group: _radioGroup
					anchors.verticalCenter: parent.verticalCenter
					checked: true
					onToggled: _form.modified = true
				}

				QFormComboBox {
					id: _comboGrade

					spacing: 0

					anchors.verticalCenter: parent.verticalCenter

					enabled: _modeGrade.checked

					combo.width: Math.min(_form.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

					field: "gradeid"

					model: ListModel {
						id: _gradeModel
					}

					valueRole: "gradeid"
					textRole: "fullname"

					combo.onActivated: _form.modified = true
				}
			}


			Row {
				spacing: 5

				QFormRadioButton {
					id: _modeXP
					text: qsTr("XP:")
					ButtonGroup.group: _radioGroup
					anchors.verticalCenter: parent.verticalCenter

					onToggled: _form.modified = true
				}

				QSpinBox {
					id: _spinXP

					enabled: _modeXP.checked

					anchors.verticalCenter: parent.verticalCenter
					from: 0
					to: 1000
					stepSize: 10

					editable: true

					font: Qaterial.Style.textTheme.body1

					onValueModified: _form.modified = true
				}
			}


			QFormSwitchButton
			{
				id: _required
				text: qsTr("Szükséges teljesíteni a magasabb értékelésekhez is")

				field: "required"
			}


			QFormComboBox {
				id: _module
				text: qsTr("Típus:")

				combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

				valueRole: "value"
				textRole: "text"

				model: [
					{value: "xp", text: qsTr("XP összegyűjtése")},
					{value: "mission", text: qsTr("Küldetés (konkrét) teljesítése")},
					{value: "mapmission", text: qsTr("Küldetések (darab) teljesítése egy pályán")}
				]

			}




			QFormComboBox {
				id: _map
				text: qsTr("Pálya:")

				combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

				visible: _module.currentValue === "mission" || _module.currentValue === "mapmission"

				valueRole: "uuid"
				textRole: "name"

				model: mapHandler ? mapHandler.mapList : null

				onCurrentIndexChanged: {
					if (_module.currentValue !== "mission")
						return

					reloadMissionList()
				}


				function reloadMissionList() {
					let m = mapHandler.mapList.get(currentIndex)

					let list = []

					for (let i=0; i<m.cache.missions.length; ++i) {
						let mis = m.cache.missions[i]

						for (let j=0; j<mis.levels.length; ++j) {
							let level = mis.levels[j].l

							list.push({
										  uuid: mis.uuid,
										  name: mis.name,
										  display: mis.name+qsTr(" - Level %1").arg(level),
										  level: level,
										  deathmatch: false
									  })

							if (mis.levels[j].dm) {
								list.push({
											  uuid: mis.uuid,
											  name: mis.name,
											  display: mis.name+qsTr(" - Level %1 DM").arg(level),
											  level: level,
											  deathmatch: true
										  })
							}
						}


					}

					_mission.model = list
				}

			}


			QFormComboBox {
				id: _mission
				text: qsTr("Küldetés:")

				combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

				visible: _module.currentValue === "mission"

				textRole: "display"

				model: null

			}





			Row {
				id: _rowNum

				spacing: 5

				visible: _module.currentValue === "xp" || _module.currentValue === "mapmission"

				Qaterial.LabelBody2 {
					text: {
						if (_module.currentValue === "xp")
							return qsTr("XP:")
						else if (_module.currentValue === "mapmission")
							return qsTr("Küldetések:")
						else
							return ""
					}

					anchors.verticalCenter: parent.verticalCenter
				}

				QSpinBox {
					id: _spinNum

					anchors.verticalCenter: parent.verticalCenter
					from: {
						if (_module.currentValue === "xp")
							return 10
						else
							return 1
					}

					to: {
						if (_module.currentValue === "xp")
							return 10000
						else
							return 100
					}

					stepSize: {
						if (_module.currentValue === "xp")
							return 10
						else
							return 1
					}

					editable: true

					font: Qaterial.Style.textTheme.body1

					onValueModified: _form.modified = true
				}
			}




			QButton
			{
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Mentés")
				icon.source: Qaterial.Icons.contentSave
				enabled: campaign && _form.modified
				onClicked:
				{
					let d = {}

					d.gradeid = _modeGrade.checked ? _comboGrade.currentValue : -1
					d.xp = _modeXP.checked ? _spinXP.value : -1
					d.required = _required.checked

					if (_map.visible)
						d.mapuuid = _map.currentValue


					let criterion = {}

					criterion.module = _module.currentValue

					if (_mission.visible && _mission.currentIndex != -1) {
						let ml = _mission.model[_mission.currentIndex]

						criterion.mission = ml.uuid
						criterion.level = ml.level
						criterion.deathmatch = ml.deathmatch
					}

					if (_rowNum.visible)
						criterion.num = _spinNum.value


					d.criterion = criterion

					let path = ""

					if (task)
						path = "task/%1/update".arg(task.taskid)
					else
						path = "campaign/%1/task/create".arg(campaign.campaignid)

					_form.enabled = false

					Client.send(HttpConnection.ApiTeacher, path, d)
					.done(control, function(r){
						_form.modified = false
						Client.stackPop(control)
					})
					.fail(control, function(err) {
						Client.messageWarning(err, task ? qsTr("Kritérium módosítása sikertelen") : qsTr("Kritérium létrehozása sikertelen"))
						_form.enabled = true
					})
					.error(control, function(err) {
						_form.enabled = true
					})

				}
			}
		}

	}



	Component.onCompleted: {
		var m = Client.cache("gradeList")
		for (var i=0; i<m.length; ++i) {
			var o = m.get(i)
			_gradeModel.append({gradeid: o.gradeid, fullname: "%1 (%2)".arg(o.longname).arg(o.shortname)})
		}

		_comboGrade.currentIndex = 0

		if (task) {
			if (task.grade) {
				_modeGrade.checked = true
				_comboGrade.currentIndex = _comboGrade.combo.indexOfValue(task.grade.gradeid)
			} else {
				_modeXP.checked = true
				_spinXP.value = task.xp
			}

			_required.checked = task.required
			_module.currentIndex = _module.combo.indexOfValue(task.criterion.module)
			_map.currentIndex = _map.combo.indexOfValue(task.mapUuid)

			_map.reloadMissionList()

			if (task.criterion.mission !== undefined) {
				for (let j=0; j<_mission.model.length; ++j) {
					let ml = _mission.model[j]

					if (ml.uuid === task.criterion.mission && ml.level === task.criterion.level && ml.deathmatch === task.criterion.deathmatch) {
						_mission.currentIndex = j
						break
					}
				}
			}

			if (task.criterion.num !== undefined)
				_spinNum.value = task.criterion.num

			_form.modified = false

		} else {
			_map.reloadMissionList()
		}
	}

}
