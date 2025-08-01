import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {


		return true
	}

	property TeacherGroup group: null
	property Pass pass: null
	property PassList passList: null


	SortFilterProxyModel {
		id: _sortedGroupListTeacher
		sourceModel: _groupModel
		sorters: [
			StringSorter {
				roleName: "text"
			}
		]

		function reload() {
			_groupModel.clear()

			if (!pass)
				return

			let l = Client.cache("teacherGroupList")

			for (let i=0; i<l.count; ++i) {
				let g = l.get(i)
				if (!g.active)
					continue
				_groupModel.append({
									   text: g.fullName,
									   id: g.groupid
								   })
			}
		}
	}

	ListModel {
		id: _groupModel
	}


	title: pass ? (pass.title != "" ? pass.title : qsTr("Call Pass #%1").arg(pass.passid)) : qsTr("Új Call Pass")
	subtitle: group ? group.fullName : ""


	appBar.backButtonVisible: true

	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: menuDetails.open()

		QMenu {
			id: menuDetails

			QMenuItem { action: _actionDuplicate }
			QMenuItem { action: _actionRemove }
		}
	}



	onPassChanged: _expGrading.loadData()


	SortFilterProxyModel {
		id: _sortedGradeList
		sourceModel: Client.cache("gradeList")
		sorters: [
			RoleSorter {
				roleName: "gradeid"
			}
		]
	}

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
				title: qsTr("A Call Pass neve")
				readOnly: !pass

				trailingContent: Qaterial.TextFieldButtonContainer
				{
					QTextFieldInPlaceButtons {
						setTo: pass ? pass.title : ""
						onSaveRequest: text => {
										   Client.send(HttpConnection.ApiTeacher, "pass/%1/update".arg(pass.passid),
													   {
														   title: text
													   })
										   .done(control, function(r){
											   reloadPass()
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
				canEdit: pass
				title: qsTr("Kezdete")
				onSaveRequest: text => {
								   Client.send(HttpConnection.ApiTeacher, "pass/%1/update".arg(pass.passid),
											   {
												   starttime: hasDate ? Math.floor(getDateTime()/1000) : -1
											   })
								   .done(control, function(r){
									   reloadPass()
									   saved()
								   })
								   .fail(control, function(err) {
									   Client.messageWarning(err, qsTr("Módosítás sikertelen"))
									   revert()
								   })
							   }
				Component.onCompleted: {
					hasDate = pass.startTime.getTime()
					if (hasDate)
						setFromDateTime(pass.startTime)
				}
			}

			QDateTimePicker {
				width: parent.width
				canEdit: pass
				title: qsTr("Vége")
				hour: 23
				minute: 59
				onSaveRequest: text => {
								   Client.send(HttpConnection.ApiTeacher, "pass/%1/update".arg(pass.passid),
											   {
												   endtime: hasDate ? Math.floor(getDateTime()/1000) : -1
											   })
								   .done(control, function(r){
									   reloadPass()
									   saved()
								   })
								   .fail(control, function(err) {
									   Client.messageWarning(err, qsTr("Módosítás sikertelen"))
									   revert()
								   })
							   }
				Component.onCompleted: {
					hasDate = pass.endTime.getTime()
					if (hasDate)
						setFromDateTime(pass.endTime)
				}
			}

		}




		Qaterial.Expandable {
			id: _expGrading
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter
			expanded: true


			property var gradingData: []
			property Repeater gradeModelRepeater: null
			property bool modified: false


			function loadData() {
				gradingData = []

				if (pass)
					gradingData = pass.getGradingFromData(pass.grading)

				modified = false
			}


			function getData(idx) {
				var d = []

				for (let i=0; i<gradeModelRepeater.model.length; ++i) {

					if (i === idx)
						continue

					let l = []

					let model = gradeModelRepeater.itemAt(i).model
					let key = gradeModelRepeater.itemAt(i).progress

					for (let j=0; j<model.length; ++j) {
						l.push(model[j])
					}

					d.push({
							   key: key,
							   list: l
						   })
				}

				return pass.getGradingFromUi(d)
			}

			header: QExpandableHeader {
				text: qsTr("Értékelés")
				icon: Qaterial.Icons.bullseyeArrow
				expandable: _expGrading
				topPadding: 20 * Qaterial.Style.pixelSizeRatio

				rightSourceComponent: Row {
					visible: _expGrading.modified

					Qaterial.AppBarButton {
						icon.source: Qaterial.Icons.check
						foregroundColor: Qaterial.Colors.lightGreen400
						anchors.verticalCenter: parent.verticalCenter

						onClicked: {
							Client.send(HttpConnection.ApiTeacher, "pass/%1/update".arg(pass.passid),
										{
											grading: pass.getMapFromUi(_expGrading.getData())
										})
							.done(control, function(r){
								reloadPass()
							})
							.fail(control, function(err) {
								Client.messageWarning(err, qsTr("Módosítás sikertelen"))
							})
						}
					}

					Qaterial.AppBarButton {
						icon.source: Qaterial.Icons.close
						foregroundColor: Qaterial.Colors.red400
						anchors.verticalCenter: parent.verticalCenter
						ToolTip.text: qsTr("Mégsem")

						onClicked: _expGrading.loadData()
					}
				}
			}

			delegate: Column {
				width: _expGrading.width

				Repeater {
					id: _gradeValueRptr

					Component.onCompleted: _expGrading.gradeModelRepeater = _gradeValueRptr

					model: _expGrading.gradingData

					delegate: Row {
						readonly property int progress: _spinGrade.value
						readonly property alias model: _gradeRptr.model

						spacing: 5

						Qaterial.SquareButton {
							icon.source: Qaterial.Icons.delete_
							icon.color: Qaterial.Colors.red400
							outlined: false

							anchors.verticalCenter: parent.verticalCenter

							onClicked: {
								let d = _expGrading.getData(index)

								_expGrading.modified = true
								_expGrading.gradingData = d
							}
						}

						QSpinBox {
							id: _spinGrade
							anchors.verticalCenter: parent.verticalCenter

							from: 0
							to: 100
							stepSize: 5
							editable: true

							font: Qaterial.Style.textTheme.body2

							value: modelData.key

							onValueModified: {
								_expGrading.modified = true
							}
						}


						Row {
							anchors.verticalCenter: parent.verticalCenter

							Repeater {
								id: _gradeRptr

								model: modelData.list

								delegate: QButton {
									property Grade grade: Client.findCacheObject("gradeList", modelData)

									anchors.verticalCenter: parent.verticalCenter
									text: grade ? grade.shortname : "???"

									onClicked: {
										_expGrading.modified = true
										_gradeRptr.model.splice(index, 1)
									}

								}
							}
						}

						Qaterial.SquareButton {
							id: _btnPlus

							icon.source: Qaterial.Icons.plus
							icon.color: Qaterial.Colors.green400
							foregroundColor: icon.color
							text: qsTr("Jegy")
							outlined: false
							anchors.verticalCenter: parent.verticalCenter

							onClicked: _menuGrade.popup(_btnPlus, 0, _btnPlus.height)

							QMenu {
								id: _menuGrade

								Instantiator {
									model: _sortedGradeList

									delegate: QMenuItem {
										text: model.longname + " (" + model.shortname + ")"
										onTriggered: {
											_expGrading.modified = true
											_gradeRptr.model.push(model.gradeid)
										}
									}

									onObjectAdded: (index, object) => _menuGrade.insertItem(index, object)
									onObjectRemoved: (index, object) => _menuGrade.removeItem(object)
								}
							}
						}

					}

				}

				Qaterial.SquareButton {
					icon.source: Qaterial.Icons.plus
					icon.color: Qaterial.Colors.green400
					foregroundColor: icon.color
					text: qsTr("Új értékelés")
					outlined: true
					anchors.horizontalCenter: parent.horizontalCenter

					topPadding: 2
					bottomPadding: 2
					leftPadding: 10
					rightPadding: 10

					onClicked: {
						let d = _expGrading.getData()

						d.push({
								   key: 0,
								   list: []
							   })

						_expGrading.gradingData = d
						_expGrading.modified = true
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
		text: qsTr("Új elem")
		icon.source: Qaterial.Icons.plus
		enabled: pass
		onTriggered: {
			if (!pass)
				return

			if (pass.isActive) {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									addTask()
								},
								text: qsTr("A Call Pas már elindult, biztosan hozzáadsz egy új elemet?"),
								title: qsTr("Új elem"),
								iconSource: Qaterial.Icons.progressQuestion
							})
				return
			}

			addTask()
		}

		function addTask() {
			/*let o = Client.stackPushPage("TeacherCampaignTaskEdit.qml", {
											 campaign: campaign,
											 mapHandler: mapHandler
										 })

			if (o)
				o.Component.destruction.connect(reloadPass)*/
		}
	}



	function reloadPass() {
		if (pass)
			pass.reload(HttpConnection.ApiTeacher)
	}




	Action {
		id: _actionRemove

		enabled: pass
		text: qsTr("Call Pass törlése")
		icon.source: Qaterial.Icons.delete_
		onTriggered: JS.questionDialog(
						 {
							 onAccepted: function()
							 {
								 Client.send(HttpConnection.ApiTeacher, "pass/%1/delete".arg(pass.passid))
								 .done(control, function(r){
									 passList.reload()
									 Client.stackPop(control)
								 })
								 .fail(control, JS.failMessage(qsTr("Törlés sikertelen")))
							 },
							 text: qsTr("Biztosan törlöd a Call Passt?"),
							 title: pass.title,
							 iconSource: Qaterial.Icons.closeCircle
						 })
	}


	Action {
		id: _actionDuplicate
		enabled: pass
		text: qsTr("Kettőzés")
		icon.source: Qaterial.Icons.contentDuplicate
		onTriggered: {
			_sortedGroupListTeacher.reload()

			/*Qaterial.DialogManager.openCheckListView(
						{
							onAccepted: function(indexList)
							{
								if (indexList.length === 0)
									return

								var l = []

								for (let i=0; i<indexList.length; ++i) {
									l.push(_sortedGroupListTeacher.get(indexList[i]).id)
								}

								Client.send(HttpConnection.ApiTeacher, "campaign/%1/duplicate".arg(campaign.campaignid), {
												list: l
											})
								.done(control, function(r){
									Client.snack(qsTr("Hadjárat megkettőzve %1x").arg(r.list ? r.list.length : 0))
								})
								.fail(control, JS.failMessage("Megkettőzés sikertelen"))
							},
							title: qsTr("Hadjárat megkettőzése"),
							standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
							model: _sortedGroupListTeacher
						})*/
		}
	}


	Connections {
		target: pass

		function onItemsLoaded() {
			_expGrading.loadData()
		}
	}


	Component.onCompleted: _expGrading.loadData()

}
