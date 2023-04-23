import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.TextField
{
	id: root

	property bool hasDate: false
	property date date: new Date()
	property date from: new Date(2022, 0, 1)
	property date to: new Date(2099, 11, 31)
	property int hour: 8
	property int minute: 0
	property bool canEdit: true


	property date _storedDate: new Date()
	property bool _storedHasDate: false
	property bool active: false

	signal saveRequest(bool hasDate, date date)

	onDateChanged: resetText()
	onHourChanged: resetText()
	onMinuteChanged: resetText()
	onHasDateChanged: resetText()

	implicitWidth: Qaterial.Style.pixelSizeRatio*450


	readOnly: true

	trailingVisible: true
	trailingInline: true
	trailingContent: Qaterial.TextFieldButtonContainer
	{
		visible: canEdit
		enabled: canEdit

		Qaterial.TextFieldIconButton
		{
			icon.source: Qaterial.Icons.calendar
			onClicked: Qaterial.DialogManager.openFromComponent(_datePickerDialogComponent)

			Component
			{
				id: _datePickerDialogComponent
				Qaterial.DatePickerDialog
				{
					date: root.date
					from: root.from
					to: root.to

					onAccepted: function()
					{
						root.date = date
						root.hasDate = true
						root.active = true
					}
					Component.onCompleted: open()
				}
			}
		}

		Qaterial.TextFieldClockButton
		{
			hour: root.hour
			minute: root.minute
			styleAm: false
			am: root.hour < 12 ? true : false

			onHourAccepted: function(hour, minute, am)
			{
				root.hour = hour
				root.minute = minute
				root.hasDate = true
				root.active = true
			}
		}

		Qaterial.TextFieldIconButton
		{
			icon.source: Qaterial.Icons.backspace
			visible: hasDate
			ToolTip.text: qsTr("Törlés")
			onClicked: {
				root.hasDate = false
				root.active = true
			}
		}


		Qaterial.TextFieldIconButton {
			id: saveButton
			icon.source: Qaterial.Icons.check
			foregroundColor: Qaterial.Colors.lightGreen400
			visible: root.active
			ToolTip.text: qsTr("Mentés")

			onClicked: {
				root.enabled = false
				root.saveRequest(hasDate, getDateTime())
			}
		}

		Qaterial.TextFieldIconButton {
			icon.source: Qaterial.Icons.close
			foregroundColor: Qaterial.Colors.red400
			visible: root.active
			ToolTip.text: qsTr("Mégsem")

			onClicked: revert()
		}
	}


	function set(d, has) {
		_storedDate = d
		_storedHasDate = has
		date = d
		hasDate = has
		active = false
	}


	function saved() {
		enabled = true
		_storedDate = date
		_storedHasDate = hasDate
		active = false
	}

	function revert() {
		date = _storedDate
		hasDate = _storedHasDate
		enabled = true
		active = false
	}


	function getDateTime() {
		var d = new Date()

		d.setFullYear(date.getFullYear())
		d.setMonth(date.getMonth())
		d.setDate(date.getDate())
		d.setHours(hour)
		d.setMinutes(minute)
		d.setSeconds(0)

		return d
	}


	function setFromDateTime(dt) {
		console.debug("SET FROM", dt, dt.getHours(), dt.getMinutes())
		date = dt
		hour = dt.getHours()
		minute = dt.getMinutes()
	}


	function resetText() {
		if (hasDate)
			text = getDateTime().toLocaleString(Qt.locale(), "yyyy. MMM d. ddd HH:mm")
		else
			text = ""
	}

	Component.onCompleted: resetText()
}
