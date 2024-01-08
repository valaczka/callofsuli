import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

Item {
	id: root

	property TeacherExam teacherExam: null
	readonly property Exam _exam: teacherExam ? teacherExam.exam : null

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
				title: qsTr("A dolgozat neve")
				//readOnly: !exam || exam.state >= Exam.Finished
				//helperText: exam && exam.state >= Exam.Finished ? qsTr("A dolgozat véget ért, a név már nem módosítható") : ""

				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldClearButton { }

					QTextFieldInPlaceButtons {
						setTo: _exam ? _exam.description : ""
						onSaveRequest: text => {
										   Client.send(HttpConnection.ApiTeacher, "exam/%1/update".arg(_exam.examId),
													   {
														   description: text
													   })
										   .done(root, function(r){
											   reloadExam()
											   saved()
										   })
										   .fail(root, function(err) {
											   Client.messageWarning(err, qsTr("Módosítás sikertelen"))
											   revert()
										   })
									   }
					}
				}
			}


			QDateTimePicker {
				width: parent.width
				canEdit: _exam && _exam.state < Exam.Finished
				title: qsTr("Időpont")

				onSaveRequest: text => {
								   Client.send(HttpConnection.ApiTeacher, "exam/%1/update".arg(_exam.examId),
											   {
												   timestamp: hasDate ? Math.floor(date.getTime()/1000) : -1
											   })
								   .done(root, function(r){
									   reloadExam()
									   saved()
								   })
								   .fail(root, function(err) {
									   Client.messageWarning(err, qsTr("Módosítás sikertelen"))
									   revert()
								   })
							   }

				Component.onCompleted: {
					hasDate = _exam.timestamp.getTime()
					if (hasDate)
						setFromDateTime(_exam.timestamp)
				}
			}

			QFormComboBox {
				id: _comboMode

				text: qsTr("Mód:")

				spacing: 0

				//anchors.verticalCenter: parent.verticalCenter

				enabled: _exam && _exam.state < Exam.Active

				combo.width: Math.min(_form.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

				inPlaceButtons.setTo: _exam ? _exam.mode : null

				valueRole: "value"
				textRole: "text"

				model: [
					{ value: 0, text: qsTr("Papír") },
					{ value: 1, text: qsTr("Digitális") },
					{ value: 2, text: qsTr("Virtuális") }
				]

				inPlaceButtonsVisible: true
				inPlaceButtons.onSaveRequest: text => {
												  Client.send(HttpConnection.ApiTeacher, "exam/%1/update".arg(_exam.examId),
															  {
																  mode: _comboMode.currentValue
															  })
												  .done(root, function(r){
													  reloadExam()
													  inPlaceButtons.saved()
												  })
												  .fail(root, function(err) {
													  Client.messageWarning(err, qsTr("Módosítás sikertelen"))
													  inPlaceButtons.revert()
												  })
											  }
			}




			QFormComboBox {
				id: _comboMap
				text: qsTr("Pálya:")

				combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

				valueRole: "uuid"
				textRole: "name"

				model: teacherExam && teacherExam.mapHandler ? teacherExam.mapHandler.mapList : null

				inPlaceButtonsVisible: true
				inPlaceButtons.onSaveRequest: text => {
												  Client.send(HttpConnection.ApiTeacher, "exam/%1/update".arg(_exam.examId),
															  {
																  mapuuid: _comboMap.currentValue
															  })
												  .done(root, function(r){
													  reloadExam()
													  inPlaceButtons.saved()
												  })
												  .fail(root, function(err) {
													  Client.messageWarning(err, qsTr("Módosítás sikertelen"))
													  inPlaceButtons.revert()
												  })
											  }

			}

		}
	}

	function reloadExam() {
		if (!_exam)
			return

		Client.send(HttpConnection.ApiTeacher, "exam/%1".arg(_exam.examId))
		.done(root, function(r){
			if (r.id !== _exam.examId) {
				Client.messageWarning(qsTr("Érvénytelen dolgozat"), qsTr("Belső hiba"))
				return
			}

			_exam.loadFromJson(r, true)
		})
		.fail(root, JS.failMessage(qsTr("Dolgozat letöltése sikertelen")))
	}


	Component.onCompleted: {
		if (_exam)  {
			_comboMap.inPlaceButtons.set(_comboMap.combo.indexOfValue(_exam.mapUuid))
		} else
			_comboMap.inPlaceButtons.set(-1)
	}
}
