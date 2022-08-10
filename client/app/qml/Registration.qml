import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
    id: control

    title: qsTr("Regisztráció")
    icon: CosStyle.iconRegistration

    property Servers servers: null

    property bool autoRegisterGoogle: false
    property alias code: textCode.text

    property bool _isFirst: true

    QGridLayoutFlickable {
        id: grid

        visible: false
        isFullscreen: control.compact

        watchModification: false

        QTabHeader {
            tabContainer: control
            Layout.columnSpan: parent.columns
            Layout.fillWidth: true
            flickable: grid.flickable
        }

        QGridButton {
            id: buttonGoogle
            text: qsTr("Regisztráció Google fiókkal")
            icon.source: "qrc:/internal/img/google.svg"

            enabled: false
            visible: false

            onClicked: {
                buttonGoogle.visible = false
                buttonPlain.visible = false
                textCode.visible = true
                buttonGoogle2.visible = true

                if (autoRegisterGoogle && textCode.text != "") {
                    buttonGoogle2.clicked()
                }
            }
        }

        QGridButton {
            id: buttonPlain
            text: qsTr("Regisztráció jelszóval")
            icon.source: CosStyle.iconUserWhite

            onClicked: {
                buttonGoogle.visible = false
                buttonPlain.visible = false

                textUser.visible = true
                textFirstname.visible = true
                textLastname.visible = true
                textPassword.visible = true
                textPassword2.visible = true
                textCode.visible = true

                buttonText2.visible = true
            }
        }



        QGridLabel {
            field: textUser
            visible: textUser.visible
        }

        QGridTextField {
            id: textUser
            visible: false
            fieldName: qsTr("Felhasználónév")
            inputMethodHints: Qt.ImhEmailCharactersOnly
        }

        QGridLabel {
            field: textFirstname
            visible: textFirstname.visible
        }

        QGridTextField {
            id: textFirstname
            visible: false
            fieldName: qsTr("Vezetéknév")
        }


        QGridLabel {
            field: textLastname
            visible: textLastname.visible
        }

        QGridTextField {
            id: textLastname
            visible: false
            fieldName: qsTr("Keresztnév")
        }


        QGridLabel {
            field: textPassword
            visible: textPassword.visible
        }

        QGridTextField {
            id: textPassword
            visible: false
            fieldName: qsTr("Jelszó")
            echoMode: TextInput.Password
            inputMethodHints: Qt.ImhSensitiveData
        }

        QGridLabel {
            field: textPassword2
            visible: textPassword2.visible
        }

        QGridTextField {
            id: textPassword2
            visible: false
            fieldName: qsTr("Jelszó ismét")
            echoMode: TextInput.Password
            inputMethodHints: Qt.ImhSensitiveData
        }

        QGridLabel {
            field: textCode
            visible: textCode.visible
        }

        QGridTextField {
            id: textCode
            visible: false
            fieldName: qsTr("Hitelesítő kód")
        }

        QGridButton {
            id: buttonText2
            text: qsTr("Regisztráció")
            icon.source: CosStyle.iconUserWhite

            visible: false

            enabled: textUser.text != "" && textFirstname.text != "" && textPassword.text != "" && textPassword.text == textPassword2.text

            onClicked: {
                grid.visible = false
                labelInfo.text = qsTr("Regisztráció...")
                labelInfo.visible = true

                servers.send(CosMessage.ClassUserInfo, "registrationRequest", {
                                 username: textUser.text,
                                 firstname: textFirstname.text,
                                 lastname: textLastname.text,
                                 code: textCode.text,
                                 password: textPassword.text
                             })
            }
        }


        QGridButton {
            id: buttonGoogle2
            text: qsTr("Tovább")
            icon.source: "qrc:/internal/img/google.svg"

            visible: false

            onClicked: {
                control.enabled = false
                servers.googleOAuth2.grant()
            }
        }
    }

    QLabel {
        id: labelInfo
        anchors.centerIn: parent

        text: qsTr("Betöltés...")
        wrapMode: Text.Wrap
        color: CosStyle.colorWarningLighter

        font.weight: Font.DemiBold

    }


    Component {
        id: oauthContainer
        OAuthContainer {  }
    }


    Connections {
        target: servers

        function onRegistrationRequest(jsonData) {
            if (jsonData.error !== undefined) {
                var e = jsonData.error
                if (e === "username exists")
                    e = qsTr("A felhasználónév már létezik!")
                else if (e === "invalid domain")
                    e = qsTr("A megadott domainről nem lehet regisztrálni!")
                else if (e === "invalid code")
                    e = qsTr("Érvénytelen hitelesítő kód!")
                else if (e === "missing code")
                    e = qsTr("Hitelesítő kód megadása szükséges!")

                cosClient.sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",qsTr("Hiba"), e)
                grid.visible = true
                labelInfo.visible = false
                control.enabled = true
                return
            }

            if (jsonData.status === true) {
                if (jsonData.registrationEnabled) {
                    labelInfo.text = ""
                    labelInfo.visible = false
                    grid.visible = true

                    buttonGoogle.enabled = jsonData.oauth2Enabled
                    buttonGoogle.visible = jsonData.oauth2Enabled

                    if (jsonData.oauth2Enabled && jsonData.oauth2Forced) {
                        buttonPlain.enabled = false
                        buttonPlain.visible = false
                    }

                    if (autoRegisterGoogle && jsonData.oauth2Enabled) {
                        buttonGoogle.clicked()
                    }
                } else {
                    labelInfo.text = qsTr("A szerveren a regisztráció nincs engedélyezve")
                    labelInfo.color = CosStyle.colorErrorLighter
                    labelInfo.visible = true
                    grid.visible = false
                }
            }

            if (jsonData.created !== undefined) {
                labelInfo.text = qsTr("Sikeres regisztráció: %1").arg(jsonData.created)
                labelInfo.color = CosStyle.colorOKLighter
                labelInfo.visible = true

                if (jsonData.token !== undefined) {
                    labelInfo.text = qsTr("Bejelentkezés: %1").arg(jsonData.created)
                    cosClient.oauth2Login(jsonData.token)
                } else {
                    cosClient.login(jsonData.created, "", textPassword.text)
                }
            }
        }
    }


    Connections {
        target: servers && servers.googleOAuth2 ? servers.googleOAuth2 : null

        function onBrowserRequest(url) {
            tabPage.pushContent(oauthContainer, {url: url})
        }

        function onAuthenticated(token, expiration, refreshToken) {
            if ((Qt.platform.os == "android"  || Qt.platform.os === "ios") && !control.isCurrentItem)
                mainStack.back()

            grid.visible = false
            labelInfo.text = qsTr("Regisztráció...")
            labelInfo.visible = true

            servers.send(CosMessage.ClassUserInfo, "registrationRequest", {
                             oauthToken: token,
                             oauthRefreshToken: refreshToken,
                             oauthExpiration: expiration,
                             code: textCode.text
                         })
        }
    }

    onPopulated: if (_isFirst) {
                     _isFirst = false
                     servers.send(CosMessage.ClassUserInfo, "registrationRequest", {})
                 }


}



