import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: root

	closeQuestion: teacherExam && teacherExam.hasPendingCorrection ? qsTr("Biztosan eldobod a módosításokat?") : ""

	onPageClose: function() {
		teacherExam.clearPendingCorrections()
	}

	property TeacherExam teacherExam: null
	property ExamUser examUser: null
	readonly property Exam _exam: teacherExam ? teacherExam.exam : null
	readonly property alias _currentUser: _comboUser.currentValue

	title: _exam ? (_exam.description != "" ? _exam.description : qsTr("Dolgozat #%1").arg(_exam.examId)) : ""
	subtitle: teacherExam && teacherExam.teacherGroup ? teacherExam.teacherGroup.fullName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: Row {
		Qaterial.AppBarButton
		{
			action: _actionCancelAll
			visible: enabled
			display: AbstractButton.IconOnly
		}

		Qaterial.AppBarButton
		{
			action: _actionSaveAll
			visible: enabled
			display: AbstractButton.IconOnly
		}
	}

	SortFilterProxyModel {
		id: _model
		sourceModel: teacherExam ? teacherExam.examUserList : null

		sorters: [
			StringSorter {
				roleName: "fullName"
			}
		]

		proxyRoles: [	// FIX: wasm
			ExpressionRole {
				name: "fullNamePending"
				expression: model.fullName + (model.pendingCorrection.length ? qsTr("*") : "")
			}
		]
	}


	Row {
		id: _titleRow
		width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
		anchors.horizontalCenter: parent.horizontalCenter

		spacing: 5 * Qaterial.Style.pixelSizeRatio

		Qaterial.ToolButton {
			id: _btnLeft
			checkable: false
			outlined: true
			action: _actionPrev
			width: height
			height: Math.max(implicitHeight, _comboUser.height)
		}

		Qaterial.ComboBox {
			id: _comboUser
			width: parent.width
				   -_btnLeft.width-_btnRight.width
				   -_btnCancel.width-_btnSave.width
				   -_placeholder.width
				   -5*parent.spacing

			model: _model

			contentItem: Qaterial.LabelHeadline5 {
				leftPadding: rightPadding
				rightPadding: _comboUser.indicator.width + _comboUser.spacing

				text: _comboUser.displayText
				color: Qaterial.Style.accentColor
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter
				elide: Text.ElideRight
			}

			textRole: "fullNamePending"
			valueRole: "qtObject"
		}

		Qaterial.ToolButton {
			id: _btnRight
			checkable: false
			outlined: true
			action: _actionNext
			width: height
			height: Math.max(implicitHeight, _comboUser.height)
		}

		Item {
			id: _placeholder
			width: 2*parent.spacing
		}

		Qaterial.SquareButton {
			id: _btnCancel
			action: _actionCancel
			width: height
			height: Math.max(implicitHeight, _comboUser.height)
		}

		Qaterial.SquareButton {
			id: _btnSave
			action: _actionSave
			width: height
			height: Math.max(implicitHeight, _comboUser.height)
		}
	}

	Qaterial.LabelHeadline5 {
		id: _labelResult
		visible: _currentUser && _currentUser.examData.length

		anchors.top: _titleRow.bottom
		anchors.left: _titleRow.left
		width: _titleRow.width

		color: Qaterial.Style.iconColor()

		text: qsTr("Eredmény: %1%").arg(_currentUser ? Math.floor(_currentUser.result*100) : 0) +
			  (_currentUser.grade ? " - %1 (%2)".arg(_currentUser.grade.longname).arg(_currentUser.grade.shortname) : "")

		topPadding: 5*Qaterial.Style.pixelSizeRatio
		bottomPadding: 10*Qaterial.Style.pixelSizeRatio
		rightPadding: 5*Qaterial.Style.pixelSizeRatio

		horizontalAlignment: Text.AlignRight

		Qaterial.HorizontalLineSeparator {
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 5 * Qaterial.Style.pixelSizeRatio
		}
	}

	QScrollable {
		anchors.top: _labelResult.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		visible: _currentUser && _currentUser.examData.length && _exam && _exam.mode != Exam.ExamVirtual

		spacing: 5 * Qaterial.Style.pixelSizeRatio

		Repeater {
			id: _rptr

			readonly property real rowWidth: Math.min(parent.width, Qaterial.Style.maxContainerSize)

			model: _currentUser ? _currentUser.examData : null

			delegate: TeacherExamCorrector {
				width: _rptr.rowWidth
				anchors.horizontalCenter: parent.horizontalCenter
				user: _currentUser
				questionIndex: index
			}
		}
	}

	Item {
		anchors.top: _titleRow.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		visible: _currentUser && (!_currentUser.examData.length || (_exam && _exam.mode == Exam.ExamVirtual))

		Qaterial.IconLabel {
			anchors.centerIn: parent
			icon.source: Qaterial.Icons.accountOffOutline
			text: qsTr("Nincs hozzárendelve dolgozat")
		}

	}




	Action {
		id: _actionPrev
		icon.source: Qaterial.Icons.arrowLeftBold
		enabled: _comboUser.currentIndex > 0
		onTriggered: _comboUser.decrementCurrentIndex()
		shortcut: "Shift+F3"
	}

	Action {
		id: _actionNext
		icon.source: Qaterial.Icons.arrowRightBold
		enabled: _comboUser.currentIndex < _model.count-1
		onTriggered: _comboUser.incrementCurrentIndex()
		shortcut: "F3"
	}

	Action {
		id: _actionCancel
		icon.source: Qaterial.Icons.cancel
		enabled: _currentUser && _currentUser.pendingCorrection.length
		onTriggered: _currentUser.pendingCorrection = []
	}

	Action {
		id: _actionSave
		icon.source: Qaterial.Icons.contentSave
		enabled: _currentUser && _currentUser.pendingCorrection.length
		shortcut: "Ctrl+S"
		onTriggered: teacherExam.savePendingCorrections([_currentUser])
	}


	Action {
		id: _actionCancelAll
		icon.source: Qaterial.Icons.cancel
		enabled: teacherExam && teacherExam.hasPendingCorrection
		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								teacherExam.clearPendingCorrections()
							},
							text: qsTr("Biztosan törlöd a változtatásokat?"),
							title: qsTr("Változtatások törlése"),
							iconSource: Qaterial.Icons.delete_
						})
		}
	}

	Action {
		id: _actionSaveAll
		icon.source: Qaterial.Icons.contentSave
		enabled: teacherExam && teacherExam.hasPendingCorrection
		onTriggered: teacherExam.savePendingCorrections([])
		shortcut: "F10"
	}


	Component.onCompleted: {
		if (!teacherExam)
			return

		if (!examUser) {
			if (!_model.count)
				return

			examUser = _model.sourceModel.get(0)
		}

		for (let i=0; i<_model.count; ++i) {
			let u = _model.sourceModel.get(_model.mapToSource(i))
			if (u === examUser) {
				_comboUser.currentIndex = i
			}
		}
	}

}
