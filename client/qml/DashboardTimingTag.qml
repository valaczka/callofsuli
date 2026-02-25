import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS


QTagList {
    required property Campaign campaign
    required property date referenceDate

    readonly property int msecLeft: campaign && campaign.endTime.getTime() ?
                                        campaign.endTime - referenceDate.getTime():
                                        0

    visible: msecLeft > 0 && msecLeft < 8 * 24 * 60 * 60 * 1000

    readonly property string stateString: {
        let d = Math.floor(msecLeft / (24*60*60*1000))

        if (d > 5)
            return qsTr(">5 nap")
        else if (d > 0)
            return qsTr("%1 nap").arg(d)
        else if (msecLeft > 60*60*1000) {
            let h = Math.floor(msecLeft / (60*60*1000))
            return qsTr("%1 óra").arg(h)
        } else {
            return qsTr("<1 óra")
        }
    }

    readonly property color stateColor: {
        if (msecLeft > 4 * 24 * 60 * 60 * 1000)
            return Qaterial.Colors.green600
        else if (msecLeft > 2 * 24 * 60 * 60 * 1000)
            return Qaterial.Colors.orange800
        else
            return Qaterial.Colors.red500
    }

    model: [
        {
            "text": stateString,
            "color": stateColor,
            "textColor": Qaterial.Colors.white
        }

    ]
}
