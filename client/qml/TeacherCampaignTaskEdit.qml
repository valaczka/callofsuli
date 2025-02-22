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
					to: 10000
					stepSize: 10

					editable: true

					font: Qaterial.Style.textTheme.body1

					onValueModified: _form.modified = true
				}
			}

			Row {
				spacing: 5

				QFormRadioButton {
					id: _modePts
					text: qsTr("Pont:")
					ButtonGroup.group: _radioGroup
					anchors.verticalCenter: parent.verticalCenter

					onToggled: _form.modified = true
				}

				QSpinBox {
					id: _spinPts

					enabled: _modePts.checked

					anchors.verticalCenter: parent.verticalCenter
					from: 1
					to: 1000
					stepSize: 1

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

				model: ListModel {
					ListElement {value: "levels"; text: qsTr("Szintek teljesítése egy küldetésen belül")}
					ListElement {value: "mission"; text: qsTr("Küldetés (konkrét) teljesítése")}
					ListElement {value: "xp"; text: qsTr("XP összegyűjtése")}
					ListElement {value: "mapmission"; text: qsTr("Küldetések (darab) teljesítése egy pályán")}
				}

				onCurrentIndexChanged: _map.refresh()
			}




			QFormComboBox {
				id: _map
				text: qsTr("Pálya:")

				combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

				visible: _module.currentValue === "mission" || _module.currentValue === "mapmission" || _module.currentValue === "levels"

				valueRole: "uuid"
				textRole: "name"

				model: mapHandler ? mapHandler.mapList : null

				onCurrentIndexChanged: refresh()


				function refresh() {
					if (_module.currentValue !== "mission" && _module.currentValue !== "levels") {
						_mission.currentIndex = -1
						_missionModel.clear()
						return
					}

					reloadMissionList(_module.currentValue !== "levels")
				}


				function reloadMissionList(_withLevels) {
					let m = mapHandler.mapList.get(currentIndex)

					_mission.currentIndex = -1
					_missionModel.clear()

					for (let i=0; i<m.cache.missions.length; ++i) {
						let mis = m.cache.missions[i]

						if (_withLevels) {
							for (let j=0; j<mis.levels.length; ++j) {
								let level = mis.levels[j].l

								_missionModel.append({
														 uuid: mis.uuid,
														 name: mis.name,
														 display: mis.name+qsTr(" - Level %1").arg(level),
														 level: level
														 /*,
										  deathmatch: false*/
													 })

								/*if (mis.levels[j].dm) {
								_missionModel.append({
											  uuid: mis.uuid,
											  name: mis.name,
											  display: mis.name+qsTr(" - Level %1 DM").arg(level),
											  level: level,
											  deathmatch: true
										  })
							}*/
							}

						} else {
							_missionModel.append({
													 uuid: mis.uuid,
													 name: mis.name,
													 display: mis.name,
													 levelCount: mis.levels.length
												 })
						}
					}
				}

			}


			QFormComboBox {
				id: _mission
				text: qsTr("Küldetés:")

				combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

				visible: _module.currentValue === "mission" || _module.currentValue === "levels"

				textRole: "display"

				model: ListModel {
					id: _missionModel
				}

				onCurrentIndexChanged: {
					if (_module.currentValue !== "levels" || currentIndex === -1)
						return

					_spinNum.value = _missionModel.get(currentIndex).levelCount
				}

			}





			Row {
				id: _rowNum

				spacing: 5

				visible: _module.currentValue === "xp" || _module.currentValue === "mapmission" || _module.currentValue === "levels"

				Qaterial.LabelBody2 {
					text: {
						if (_module.currentValue === "xp")
							return qsTr("XP:")
						else if (_module.currentValue === "mapmission")
							return qsTr("Küldetések:")
						else if (_module.currentValue === "levels")
							return qsTr("Teljesítendő szintek")
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
						let ml = _missionModel.get(_mission.currentIndex)

						criterion.mission = ml.uuid

						if (criterion.module !== "levels")
							criterion.level = ml.level

						//criterion.deathmatch = ml.deathmatch
					}

					if (_rowNum.visible)
						criterion.num = _spinNum.value

					if (_modePts.checked)
						criterion.pts = _spinPts.value

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
			if (task.criterion.pts !== undefined && task.criterion.pts > 0) {
				_modePts.checked = true
				_spinPts.value = task.criterion.pts
			} else if (task.grade) {
				_modeGrade.checked = true
				_comboGrade.currentIndex = _comboGrade.combo.indexOfValue(task.grade.gradeid)
			} else {
				_modeXP.checked = true
				_spinXP.value = task.xp
			}

			_required.checked = task.required
			_module.currentIndex = _module.combo.indexOfValue(task.criterion.module)
			_map.currentIndex = _map.combo.indexOfValue(task.mapUuid)

			_map.reloadMissionList(task.criterion.module !== "levels")

			if (task.criterion.mission !== undefined) {
				for (let j=0; j<_missionModel.count; ++j) {
					let ml = _missionModel.get(j)

					if (ml.uuid === task.criterion.mission &&
							(task.criterion.module === "levels" || ml.level === task.criterion.level)
							/*&& ml.deathmatch === task.criterion.deathmatch*/) {
						_mission.currentIndex = j
						break
					}
				}
			}

			if (task.criterion.num !== undefined)
				_spinNum.value = task.criterion.num

			_form.modified = false

		} else {
			_map.reloadMissionList(true)
		}
	}

}
