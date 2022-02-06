import QtQuick 2.15
import QtQuick.Controls 2.15
//import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QListView {
	id: view

	property bool selectorSet: false
	property bool autoSelectorChange: true
	property bool autoUnselectorChange: autoSelectorChange

	readonly property ObjectListModel sourceObjectListModel: model && model.sourceModel && (model.sourceModel instanceof ObjectListModel) ?
																 model.sourceModel :
																 null

	readonly property ObjectListModel objectModel: sourceObjectListModel ? sourceObjectListModel :
																		   (model instanceof ObjectListModel) ? model : null

	focus: true


	function onDelegateClicked(index, withShift) {
		if (view.selectorSet) {
			if (withShift && view.currentIndex != -1) {
				var i = Math.min(view.currentIndex, index)
				var j = Math.max(view.currentIndex, index)

				for (var n=i; n<=j; ++n)
					objectModel.select(normalizedIndex(n))

			} else {
				objectModel.selectToggle(normalizedIndex(index))
			}

			view.currentIndex = index
		}
	}


	function onDelegateLongClicked(index) {
		if (autoSelectorChange && !selectorSet) {
			objectModel.select(normalizedIndex(index))
			selectorSet = true
			view.currentIndex = index
		}
	}




	Connections {
		target: objectModel

		function onSelectedCountChanged() {
			if (objectModel.selectedCount === 0 && autoUnselectorChange)
				selectorSet=false
		}
	}



	function normalizedIndex(index) {
		if (sourceObjectListModel) {
			return model.mapToSource(index)
		}

		return index
	}

	function modelObject(index) {
		return objectModel ? objectModel.object(normalizedIndex(index)) : null
	}
}
