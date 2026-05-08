import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ModalDialog
{
	id: control

	property TeacherMapHandler handler: null
	property TeacherMap baseMapObject: null


	horizontalPadding: 0
	standardButtons: itemNoMap.visible ? DialogButtonBox.Close : DialogButtonBox.Cancel | DialogButtonBox.Ok

	dialogImplicitWidth: 600

	signal tagSelected(var tags)


	ListModel {
		id: _model
	}


	contentItem: Item
	{

		implicitHeight: Math.min(control.parent.height-150, Math.max(view.contentHeight, 350 * Qaterial.Style.pixelSizeRatio))

		Qaterial.IconLabelWithCaption {
			id: itemNoMap

			visible: !handler || !handler.tagList || handler.tagList.count === 0

			anchors.centerIn: parent
			icon.source: Qaterial.Icons.tagOffOutline
			icon.color: Qaterial.Colors.orange500
			textColor: Qaterial.Colors.orange500
			text: qsTr("Nincsenek címkék")
			caption: qsTr("A szerveren még egyetlen címke sem található")
		}



		QListView {
			id: view

			width: parent.width


			visible: !itemNoMap.visible
			selectEnabled: true
			autoSelectChange: false

			model: _model

			delegate: QItemDelegate {
				required property TeacherMapTag tag
				required property int depth
				required property int index

				selectableObject: tag

				highlighted: view.selectEnabled ? tag.selected : ListView.isCurrentItem
				//iconSource: Qaterial.Icons.tag

				icon.source: tag.selected ? Qaterial.Icons.checkCircle : Qaterial.Icons.circleOutline

				//textColor: Qaterial.Style.accentColor
				/*secondaryTextColor: exam && exam.state === Exam.Finished ?
										Qaterial.Style.disabledTextColor() : Qaterial.Style.colorTheme.secondaryText*/


				text: tag.name

				leftPadding: Math.max(Qaterial.Style.delegate.leftPadding(control.type, control.lines), Client.safeMarginLeft)
							 + Qaterial.Style.delegate.iconWidth * depth
			}
		}
	}


	function loadChildren(parentId, depth) {
		let result = []

		if (!handler || !handler.tagList)
			return

		for (let i=0; i<handler.tagList.count; ++i) {
			const t = handler.tagList.get(i)

			if (t.parentId === parentId)
				result.push(t)
		}

		result.sort(function(a, b) {
			return String(a.name).localeCompare(String(b.name))
		})

		for (let j=0; j<result.length; ++j) {
			const t = result[j]

			_model.append({
							  tag: t,
							  depth: depth
						  })

			loadChildren(t.tagId, depth+1)
		}

	}


	function loadSelection() {
		if (!handler || !handler.tagList || !baseMapObject)
			return

		for (let i=0; i<handler.tagList.count; ++i) {
			const t = handler.tagList.get(i)
			t.selected = baseMapObject && baseMapObject.tags.includes(t.tagId)
		}
	}


	function loadModel() {
		_model.clear()

		if (!handler || !handler.tagList)
			return

		if (handler.tagList.count === 0)
			return

		loadChildren(0, 0)
	}


	onHandlerChanged: {
		loadModel()
		loadSelection()
	}

	onBaseMapObjectChanged: {
		loadSelection()
	}

	Component.onCompleted: {
		open()
	}


	onOpened: {
		view.forceActiveFocus()
	}

	onAccepted: {
		let l = []

		if (!handler || !handler.tagList)
			return l

		for (let i=0; i<handler.tagList.count; ++i) {
			const t = handler.tagList.get(i)
			if (t.selected)
				l.push(t.tagId)
		}

		console.info("SEL", l)

		tagSelected(l)
	}

}

