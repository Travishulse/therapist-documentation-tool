import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

Window {
    id: companionWindow
    visible: true
    title: qsTr("Clinical Documentation Companion")

    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Window
    color: "transparent"

    property int widgetState: 2
    property bool customDockedPosition: false
    property real dockedX: Screen.width - 600
    property real dockedY: 40

    readonly property real screenW: Screen.width
    readonly property real screenH: Screen.height

    readonly property var stateDimensions: [
        { w: 88,  h: 88 },
        { w: 600, h: (companionWindow.widgetState === 1 && stripSearchInput.text.trim().length > 0 && clinicalEngine.resultCount > 0) ? 172 : 82 },
        { w: 900, h: 600 },
        { w: Math.round(screenW * 0.84), h: Math.round(screenH * 0.84) }
    ]

    property real targetWidth: stateDimensions[widgetState].w
    property real targetHeight: stateDimensions[widgetState].h
    property real targetX: {
        if (widgetState === 3) {
            return Math.round((screenW - targetWidth) / 2)
        }
        if (customDockedPosition) {
            return Math.max(8, Math.min(screenW - targetWidth - 8, dockedX))
        }
        return Math.round(screenW - targetWidth - 28)
    }
    property real targetY: {
        if (widgetState === 3) {
            return Math.round((screenH - targetHeight) / 2)
        }
        if (customDockedPosition) {
            return Math.max(8, Math.min(screenH - targetHeight - 8, dockedY))
        }
        return 40
    }

    x: targetX
    y: targetY
    width: targetWidth
    height: targetHeight

    Behavior on width  { NumberAnimation { duration: 360; easing.type: Easing.OutCubic } }
    Behavior on height { NumberAnimation { duration: 360; easing.type: Easing.OutCubic } }
    Behavior on x      { NumberAnimation { duration: 360; easing.type: Easing.OutCubic } }
    Behavior on y      { NumberAnimation { duration: 360; easing.type: Easing.OutCubic } }

    property real reEntryWave: 0.0
    property bool copiedBannerActive: false
    property bool savedBannerActive: false
    property string lastSettledCardId: ""
    property real cardSettleScale: 1.0

    Component.onCompleted: {
        cadenceTracker.windowReEntered.connect(() => {
            reEntryAnim.restart()
        })
        clinicalEngine.itemToggled.connect((id, added) => {
            cadenceTracker.recordUserActivity()
            if (added) {
                companionWindow.lastSettledCardId = id
                cardSettleAnim.restart()
            }
        })
        clinicalEngine.clipboardCopied.connect(() => {
            companionWindow.copiedBannerActive = true
            copyResetTimer.restart()
        })
        clinicalEngine.fileSaved.connect((path) => {
            companionWindow.savedBannerActive = true
            saveResetTimer.restart()
        })
        if (typeof updateChecker !== "undefined") {
            updateChecker.checkForUpdates()
        }
    }

    Timer {
        id: copyResetTimer
        interval: 2000
        onTriggered: companionWindow.copiedBannerActive = false
    }

    Timer {
        id: saveResetTimer
        interval: 2200
        onTriggered: companionWindow.savedBannerActive = false
    }

    SequentialAnimation {
        id: reEntryAnim
        NumberAnimation {
            target: companionWindow
            property: "reEntryWave"
            from: 1.0
            to: 0.0
            duration: 620
            easing.type: Easing.OutCubic
        }
    }

    SequentialAnimation {
        id: cardSettleAnim
        NumberAnimation {
            target: companionWindow
            property: "cardSettleScale"
            from: 0.965
            to: 1.015
            duration: 130
            easing.type: Easing.OutQuad
        }
        NumberAnimation {
            target: companionWindow
            property: "cardSettleScale"
            from: 1.015
            to: 1.0
            duration: 210
            easing.type: Easing.OutBack
        }
    }

    readonly property var chapterMeta: [
        { label: "Themes",        rangeText: "1–3", placeholder: "Search session themes (e.g., anxiety, grief, boundaries)..." },
        { label: "Interventions", rangeText: "3–5", placeholder: "Search 1,385 interventions or symptoms (e.g., cbt, gottman, defusion)..." },
        { label: "Safety",        rangeText: "0–3", placeholder: "Search clinical safety & risk indicators (e.g., nssi, passive, none)..." },
        { label: "Plan",          rangeText: "0–3", placeholder: "Search between-session plan, homework, or referrals..." }
    ]

    component ContainedBadge : Rectangle {
        id: badgeRoot
        property string textValue: ""
        property color textColor: cadenceTracker.specularRimColor
        property color fillColor: "#18222e"
        property color strokeColor: "#334155"
        property int pixelSize: 9
        property bool boldText: true
        property real maxBadgeWidth: 160
        property real horizontalPad: 8
        property real badgeHeight: 19

        radius: 5
        color: fillColor
        border.color: strokeColor
        border.width: 1
        clip: true
        implicitHeight: badgeHeight
        implicitWidth: Math.min(maxBadgeWidth, Math.ceil(badgeTextItem.implicitWidth) + horizontalPad * 2)
        Layout.preferredHeight: badgeHeight
        Layout.preferredWidth: implicitWidth
        Layout.maximumWidth: maxBadgeWidth

        Text {
            id: badgeTextItem
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: badgeRoot.horizontalPad
            anchors.rightMargin: badgeRoot.horizontalPad
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: badgeRoot.textValue
            color: badgeRoot.textColor
            font.pixelSize: badgeRoot.pixelSize
            font.bold: badgeRoot.boldText
            elide: Text.ElideRight
            maximumLineCount: 1
        }
    }

    component DirectionalCard : Item {
        id: dCard
        property real cardRadius: 11
        property color fillColor: cadenceTracker.elevatedCardColor
        property color topRightRimColor: cadenceTracker.topRightHighlightColor
        property color midRimColor: cadenceTracker.specularRimColor
        property color bottomLeftRimColor: cadenceTracker.occludedRimColor
        property color sheenTint: cadenceTracker.cardSheenColor
        property real rimWidth: 1.6
        property real shadowStrength: 1.0
        property bool hovered: false
        property bool selected: false
        default property alias cardContent: innerContentHolder.data

        Rectangle {
            x: (-6.5 - (dCard.hovered ? 1.5 : 0.0)) * dCard.shadowStrength
            y: (6.5 + (dCard.hovered ? 1.5 : 0.0)) * dCard.shadowStrength
            width: parent.width + 2
            height: parent.height + 2
            radius: dCard.cardRadius + 2
            color: "#2202060d"
            visible: dCard.shadowStrength > 0.01
        }

        Rectangle {
            x: (-4.0 - (dCard.hovered ? 1.0 : 0.0)) * dCard.shadowStrength
            y: (4.0 + (dCard.hovered ? 1.0 : 0.0)) * dCard.shadowStrength
            width: parent.width + 1
            height: parent.height + 1
            radius: dCard.cardRadius + 1
            color: "#3e02060d"
            visible: dCard.shadowStrength > 0.01
        }

        Rectangle {
            x: (-1.8 - (dCard.hovered ? 0.5 : 0.0)) * dCard.shadowStrength
            y: (1.8 + (dCard.hovered ? 0.5 : 0.0)) * dCard.shadowStrength
            width: parent.width
            height: parent.height
            radius: dCard.cardRadius
            color: "#6202060d"
            visible: dCard.shadowStrength > 0.01
        }

        Shape {
            id: cardRimShape
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 4
            readonly property real cornerR: Math.max(1, dCard.cardRadius)

            ShapePath {
                strokeColor: "transparent"
                strokeWidth: 0
                fillGradient: LinearGradient {
                    x1: dCard.width
                    y1: 0
                    x2: 0
                    y2: dCard.height
                    GradientStop {
                        position: 0.0
                        color: dCard.hovered
                               ? "#e0f2fe"
                               : (dCard.selected ? "#a7f3d0" : dCard.topRightRimColor)
                    }
                    GradientStop {
                        position: 0.30
                        color: dCard.hovered
                               ? "#93c5fd"
                               : (dCard.selected ? "#34d399" : dCard.midRimColor)
                    }
                    GradientStop {
                        position: 0.62
                        color: dCard.selected ? "#1f5c46" : "#2f3e50"
                    }
                    GradientStop {
                        position: 1.0
                        color: dCard.selected ? "#0f2920" : dCard.bottomLeftRimColor
                    }
                }
                startX: cardRimShape.cornerR
                startY: 0
                PathLine { x: Math.max(cardRimShape.cornerR, dCard.width - cardRimShape.cornerR); y: 0 }
                PathArc { x: dCard.width; y: cardRimShape.cornerR; radiusX: cardRimShape.cornerR; radiusY: cardRimShape.cornerR; direction: PathArc.Clockwise }
                PathLine { x: dCard.width; y: Math.max(cardRimShape.cornerR, dCard.height - cardRimShape.cornerR) }
                PathArc { x: Math.max(cardRimShape.cornerR, dCard.width - cardRimShape.cornerR); y: dCard.height; radiusX: cardRimShape.cornerR; radiusY: cardRimShape.cornerR; direction: PathArc.Clockwise }
                PathLine { x: cardRimShape.cornerR; y: dCard.height }
                PathArc { x: 0; y: Math.max(cardRimShape.cornerR, dCard.height - cardRimShape.cornerR); radiusX: cardRimShape.cornerR; radiusY: cardRimShape.cornerR; direction: PathArc.Clockwise }
                PathLine { x: 0; y: cardRimShape.cornerR }
                PathArc { x: cardRimShape.cornerR; y: 0; radiusX: cardRimShape.cornerR; radiusY: cardRimShape.cornerR; direction: PathArc.Clockwise }
            }
        }

        Rectangle {
            id: baseBody
            anchors.fill: parent
            anchors.margins: dCard.rimWidth
            radius: Math.max(1, dCard.cardRadius - dCard.rimWidth)
            color: dCard.fillColor
            clip: true
            readonly property real innerR: Math.max(1, dCard.cardRadius - dCard.rimWidth)

            Behavior on color { ColorAnimation { duration: 180 } }

            Shape {
                anchors.fill: parent
                layer.enabled: true
                layer.samples: 4

                ShapePath {
                    strokeColor: "transparent"
                    strokeWidth: 0
                    fillGradient: LinearGradient {
                        x1: baseBody.width
                        y1: 0
                        x2: baseBody.width * 0.22
                        y2: baseBody.height * 0.78
                        GradientStop {
                            position: 0.0
                            color: dCard.hovered
                                   ? Qt.lighter(dCard.sheenTint, 1.35)
                                   : dCard.sheenTint
                        }
                        GradientStop {
                            position: 0.38
                            color: "#0cffffff"
                        }
                        GradientStop {
                            position: 1.0
                            color: "transparent"
                        }
                    }
                    startX: baseBody.innerR
                    startY: 0
                    PathLine { x: Math.max(baseBody.innerR, baseBody.width - baseBody.innerR); y: 0 }
                    PathArc { x: baseBody.width; y: baseBody.innerR; radiusX: baseBody.innerR; radiusY: baseBody.innerR; direction: PathArc.Clockwise }
                    PathLine { x: baseBody.width; y: Math.max(baseBody.innerR, baseBody.height - baseBody.innerR) }
                    PathArc { x: Math.max(baseBody.innerR, baseBody.width - baseBody.innerR); y: baseBody.height; radiusX: baseBody.innerR; radiusY: baseBody.innerR; direction: PathArc.Clockwise }
                    PathLine { x: baseBody.innerR; y: baseBody.height }
                    PathArc { x: 0; y: Math.max(baseBody.innerR, baseBody.height - baseBody.innerR); radiusX: baseBody.innerR; radiusY: baseBody.innerR; direction: PathArc.Clockwise }
                    PathLine { x: 0; y: baseBody.innerR }
                    PathArc { x: baseBody.innerR; y: 0; radiusX: baseBody.innerR; radiusY: baseBody.innerR; direction: PathArc.Clockwise }
                }

                ShapePath {
                    strokeColor: "transparent"
                    strokeWidth: 0
                    fillGradient: RadialGradient {
                        centerX: baseBody.width - 8
                        centerY: 6
                        focalX: baseBody.width - 4
                        focalY: 4
                        centerRadius: Math.max(48, Math.min(baseBody.width, baseBody.height) * 0.72)
                        GradientStop {
                            position: 0.0
                            color: dCard.hovered ? "#38e0f2fe" : "#22ffffff"
                        }
                        GradientStop {
                            position: 0.55
                            color: "#08ffffff"
                        }
                        GradientStop {
                            position: 1.0
                            color: "transparent"
                        }
                    }
                    startX: baseBody.innerR
                    startY: 0
                    PathLine { x: Math.max(baseBody.innerR, baseBody.width - baseBody.innerR); y: 0 }
                    PathArc { x: baseBody.width; y: baseBody.innerR; radiusX: baseBody.innerR; radiusY: baseBody.innerR; direction: PathArc.Clockwise }
                    PathLine { x: baseBody.width; y: Math.max(baseBody.innerR, baseBody.height - baseBody.innerR) }
                    PathArc { x: Math.max(baseBody.innerR, baseBody.width - baseBody.innerR); y: baseBody.height; radiusX: baseBody.innerR; radiusY: baseBody.innerR; direction: PathArc.Clockwise }
                    PathLine { x: baseBody.innerR; y: baseBody.height }
                    PathArc { x: 0; y: Math.max(baseBody.innerR, baseBody.height - baseBody.innerR); radiusX: baseBody.innerR; radiusY: baseBody.innerR; direction: PathArc.Clockwise }
                    PathLine { x: 0; y: baseBody.innerR }
                    PathArc { x: baseBody.innerR; y: 0; radiusX: baseBody.innerR; radiusY: baseBody.innerR; direction: PathArc.Clockwise }
                }
            }

            Item {
                id: innerContentHolder
                anchors.fill: parent
                clip: true
            }
        }
    }

    Rectangle {
        x: mainSurface.x - 9
        y: mainSurface.y + 9
        width: mainSurface.width + 2
        height: mainSurface.height + 2
        radius: mainSurface.radius + 2
        color: "#2402060d"
    }

    Rectangle {
        x: mainSurface.x - 5
        y: mainSurface.y + 5
        width: mainSurface.width + 1
        height: mainSurface.height + 1
        radius: mainSurface.radius + 1
        color: "#4202060d"
    }

    Rectangle {
        x: mainSurface.x - 2
        y: mainSurface.y + 2
        width: mainSurface.width
        height: mainSurface.height
        radius: mainSurface.radius
        color: "#6802060d"
    }

    Shape {
        id: surfaceRimShape
        anchors.fill: mainSurface
        anchors.margins: -1.5
        layer.enabled: true
        layer.samples: 4
        readonly property real cornerR: Math.max(1, mainSurface.radius + 1.5)

        ShapePath {
            strokeColor: "transparent"
            strokeWidth: 0
            fillGradient: LinearGradient {
                x1: surfaceRimShape.width
                y1: 0
                x2: 0
                y2: surfaceRimShape.height
                GradientStop {
                    position: 0.0
                    color: companionWindow.reEntryWave > 0.15
                           ? "#93c5fd"
                           : cadenceTracker.topRightHighlightColor
                }
                GradientStop {
                    position: 0.32
                    color: companionWindow.reEntryWave > 0.15
                           ? "#60a5fa"
                           : (cadenceTracker.peripheralCueActive
                              ? Qt.lighter(cadenceTracker.specularRimColor, 1.0 + 0.25 * cadenceTracker.peripheralLuminance)
                              : cadenceTracker.specularRimColor)
                }
                GradientStop {
                    position: 0.65
                    color: cadenceTracker.windowActive ? "#334357" : "#24303e"
                }
                GradientStop {
                    position: 1.0
                    color: cadenceTracker.occludedRimColor
                }
            }
            startX: surfaceRimShape.cornerR
            startY: 0
            PathLine { x: Math.max(surfaceRimShape.cornerR, surfaceRimShape.width - surfaceRimShape.cornerR); y: 0 }
            PathArc { x: surfaceRimShape.width; y: surfaceRimShape.cornerR; radiusX: surfaceRimShape.cornerR; radiusY: surfaceRimShape.cornerR; direction: PathArc.Clockwise }
            PathLine { x: surfaceRimShape.width; y: Math.max(surfaceRimShape.cornerR, surfaceRimShape.height - surfaceRimShape.cornerR) }
            PathArc { x: Math.max(surfaceRimShape.cornerR, surfaceRimShape.width - surfaceRimShape.cornerR); y: surfaceRimShape.height; radiusX: surfaceRimShape.cornerR; radiusY: surfaceRimShape.cornerR; direction: PathArc.Clockwise }
            PathLine { x: surfaceRimShape.cornerR; y: surfaceRimShape.height }
            PathArc { x: 0; y: Math.max(surfaceRimShape.cornerR, surfaceRimShape.height - surfaceRimShape.cornerR); radiusX: surfaceRimShape.cornerR; radiusY: surfaceRimShape.cornerR; direction: PathArc.Clockwise }
            PathLine { x: 0; y: surfaceRimShape.cornerR }
            PathArc { x: surfaceRimShape.cornerR; y: 0; radiusX: surfaceRimShape.cornerR; radiusY: surfaceRimShape.cornerR; direction: PathArc.Clockwise }
        }
    }

    Rectangle {
        id: mainSurface
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 6
        anchors.topMargin: 6 + (companionWindow.widgetState <= 1 ? cadenceTracker.peripheralOffsetY : 0)
        anchors.bottomMargin: 14 - (companionWindow.widgetState <= 1 ? cadenceTracker.peripheralOffsetY : 0)

        radius: companionWindow.widgetState === 0 ? 34 : (companionWindow.widgetState === 1 ? 26 : 18)
        color: cadenceTracker.surfaceColor
        border.width: 0
        clip: true

        Behavior on radius { NumberAnimation { duration: 280; easing.type: Easing.OutCubic } }
        Behavior on color { ColorAnimation { duration: 400 } }

        Shape {
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 4

            ShapePath {
                strokeColor: "transparent"
                strokeWidth: 0
                fillGradient: LinearGradient {
                    x1: mainSurface.width
                    y1: 0
                    x2: mainSurface.width * 0.28
                    y2: mainSurface.height * 0.72
                    GradientStop { position: 0.0; color: cadenceTracker.keyLightTint }
                    GradientStop { position: 0.45; color: "#08ffffff" }
                    GradientStop { position: 1.0; color: "transparent" }
                }
                startX: mainSurface.radius
                startY: 0
                PathLine { x: Math.max(mainSurface.radius, mainSurface.width - mainSurface.radius); y: 0 }
                PathArc { x: mainSurface.width; y: mainSurface.radius; radiusX: mainSurface.radius; radiusY: mainSurface.radius; direction: PathArc.Clockwise }
                PathLine { x: mainSurface.width; y: Math.max(mainSurface.radius, mainSurface.height - mainSurface.radius) }
                PathArc { x: Math.max(mainSurface.radius, mainSurface.width - mainSurface.radius); y: mainSurface.height; radiusX: mainSurface.radius; radiusY: mainSurface.radius; direction: PathArc.Clockwise }
                PathLine { x: mainSurface.radius; y: mainSurface.height }
                PathArc { x: 0; y: Math.max(mainSurface.radius, mainSurface.height - mainSurface.radius); radiusX: mainSurface.radius; radiusY: mainSurface.radius; direction: PathArc.Clockwise }
                PathLine { x: 0; y: mainSurface.radius }
                PathArc { x: mainSurface.radius; y: 0; radiusX: mainSurface.radius; radiusY: mainSurface.radius; direction: PathArc.Clockwise }
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "#38bdf8"
            opacity: companionWindow.reEntryWave * 0.16
            visible: companionWindow.reEntryWave > 0.01
        }

        Repeater {
            model: [
                { leftAnchor: true,  topAnchor: true  },
                { leftAnchor: false, topAnchor: true  },
                { leftAnchor: true,  topAnchor: false },
                { leftAnchor: false, topAnchor: false }
            ]

            Item {
                visible: companionWindow.widgetState > 0
                width: 18
                height: 18
                x: modelData.leftAnchor ? 0 : mainSurface.width - width
                y: modelData.topAnchor  ? 0 : mainSurface.height - height
                z: 50

                Rectangle {
                    width: 6
                    height: 6
                    radius: 3
                    anchors.centerIn: parent
                    color: cornerMouse.containsMouse ? cadenceTracker.specularRimColor : "#475569"
                    opacity: cornerMouse.containsMouse ? 0.9 : 0.45
                }

                MouseArea {
                    id: cornerMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeAllCursor
                    onPressed: {
                        cadenceTracker.recordUserActivity()
                        companionWindow.startSystemMove()
                    }
                    onReleased: {
                        let nx = companionWindow.x
                        let ny = companionWindow.y
                        if (nx < 42) nx = 10
                        if (nx + companionWindow.width > companionWindow.screenW - 42) {
                            nx = companionWindow.screenW - companionWindow.width - 10
                        }
                        if (ny < 42) ny = 10
                        if (ny + companionWindow.height > companionWindow.screenH - 42) {
                            ny = companionWindow.screenH - companionWindow.height - 10
                        }
                        companionWindow.dockedX = nx
                        companionWindow.dockedY = ny
                        companionWindow.customDockedPosition = true
                    }
                }
            }
        }

        // STATE 0: MICRO-PUCK
        Item {
            anchors.fill: parent
            visible: companionWindow.widgetState === 0

            Shape {
                id: puckRingShape
                anchors.fill: parent
                anchors.margins: 6
                layer.enabled: true
                layer.samples: 4

                ShapePath {
                    strokeColor: "#293545"
                    strokeWidth: 4
                    fillColor: "transparent"
                    capStyle: ShapePath.RoundCap
                    PathAngleArc {
                        centerX: puckRingShape.width / 2
                        centerY: puckRingShape.height / 2
                        radiusX: Math.max(8, puckRingShape.width / 2 - 4)
                        radiusY: Math.max(8, puckRingShape.height / 2 - 4)
                        startAngle: 0
                        sweepAngle: 360
                    }
                }

                ShapePath {
                    strokeColor: clinicalEngine.recommendedNextSection === -1 ? "#34d399" : cadenceTracker.specularRimColor
                    strokeWidth: 4
                    fillColor: "transparent"
                    capStyle: ShapePath.RoundCap
                    PathAngleArc {
                        centerX: puckRingShape.width / 2
                        centerY: puckRingShape.height / 2
                        radiusX: Math.max(8, puckRingShape.width / 2 - 4)
                        radiusY: Math.max(8, puckRingShape.height / 2 - 4)
                        startAngle: -90
                        sweepAngle: 360 * clinicalEngine.endowedProgress
                    }
                }
            }

            DirectionalCard {
                anchors.centerIn: parent
                width: 38
                height: 38
                cardRadius: 19
                shadowStrength: 0.7

                Text {
                    anchors.centerIn: parent
                    text: clinicalEngine.totalSelectedCount > 0 ? String(clinicalEngine.totalSelectedCount) : "✦"
                    font.pixelSize: clinicalEngine.totalSelectedCount > 0 ? 14 : 17
                    font.bold: true
                    color: "#f8fafc"
                }
            }

            MouseArea {
                anchors.fill: parent
                anchors.margins: 10
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    cadenceTracker.recordUserActivity()
                    companionWindow.widgetState = 1
                }
            }
        }

        // STATE 1: QUICK SEARCH STRIP
        Item {
            id: stripContainer
            anchors.fill: parent
            anchors.margins: 10
            visible: companionWindow.widgetState === 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 42
                    spacing: 8

                    DirectionalCard {
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34
                        cardRadius: 17
                        shadowStrength: 0.65

                        Rectangle {
                            anchors.centerIn: parent
                            width: 7
                            height: 7
                            radius: 3.5
                            color: "#93c5fd"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                companionWindow.widgetState = 0
                            }
                        }
                    }

                    Row {
                        spacing: 4
                        Layout.alignment: Qt.AlignVCenter

                        Repeater {
                            model: 4
                            Rectangle {
                                width: 10
                                height: 22
                                radius: 5
                                color: {
                                    const prog = clinicalEngine.sectionProgress[index] || 0
                                    if (prog >= 1.0) return "#34d399"
                                    if (clinicalEngine.activeSection === index) return cadenceTracker.specularRimColor
                                    return "#293545"
                                }
                                border.color: clinicalEngine.recommendedNextSection === index ? "#fbbf24" : "transparent"
                                border.width: clinicalEngine.recommendedNextSection === index ? 1.5 : 0

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        cadenceTracker.recordUserActivity()
                                        clinicalEngine.activeSection = index
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        radius: 18
                        color: "#131b23"
                        border.color: stripSearchInput.cursorVisible ? cadenceTracker.specularRimColor : "#2d3c4e"
                        border.width: stripSearchInput.cursorVisible ? 1.5 : 1
                        clip: true

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 10
                            spacing: 6

                            TextInput {
                                id: stripSearchInput
                                Layout.fillWidth: true
                                verticalAlignment: TextInput.AlignVCenter
                                font.pixelSize: 13
                                color: "#f1f5f9"
                                selectByMouse: true
                                clip: true
                                text: clinicalEngine.query

                                onTextEdited: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.query = text
                                }
                                onAccepted: {
                                    cadenceTracker.recordUserActivity()
                                    companionWindow.widgetState = 2
                                }

                                Text {
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    text: companionWindow.chapterMeta[clinicalEngine.activeSection].placeholder
                                    color: "#64748b"
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                    visible: stripSearchInput.text.length === 0
                                }
                            }

                            Rectangle {
                                visible: stripSearchInput.text.length > 0
                                width: 20
                                height: 20
                                radius: 10
                                color: "#263240"
                                Text { anchors.centerIn: parent; text: "×"; color: "#cbd5e1"; font.pixelSize: 12 }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        cadenceTracker.recordUserActivity()
                                        clinicalEngine.query = ""
                                    }
                                }
                            }
                        }
                    }

                    DirectionalCard {
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34
                        cardRadius: 17
                        shadowStrength: 0.65

                        Text {
                            anchors.centerIn: parent
                            text: "↻"
                            font.pixelSize: 16
                            font.bold: true
                            color: "#cbd5e1"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                clinicalEngine.clearAll()
                            }
                        }
                    }

                    DirectionalCard {
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34
                        cardRadius: 17
                        shadowStrength: 0.65

                        Item {
                            anchors.centerIn: parent
                            width: 16
                            height: 16

                            Rectangle {
                                x: 0; y: 0
                                width: 16; height: 3.2
                                radius: 1.4
                                color: "#60a5fa"
                            }

                            Grid {
                                x: 0; y: 5.5
                                columns: 2
                                spacing: 2
                                Repeater {
                                    model: 4
                                    Rectangle {
                                        width: 4.2
                                        height: 4.2
                                        radius: 1.2
                                        color: "#cbd5e1"
                                    }
                                }
                            }

                            Rectangle {
                                x: 12; y: 5.5
                                width: 4
                                height: 10.4
                                radius: 1.4
                                color: "#34d399"
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                companionWindow.widgetState = 2
                            }
                        }
                    }

                    DirectionalCard {
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34
                        cardRadius: 17
                        shadowStrength: 0.65

                        Item {
                            anchors.centerIn: parent
                            width: 15
                            height: 15

                            Rectangle {
                                anchors.fill: parent
                                radius: 7.5
                                color: "transparent"
                                border.color: "#cbd5e1"
                                border.width: 1.4
                            }

                            Rectangle {
                                anchors.centerIn: parent
                                width: 7
                                height: 7
                                radius: 1.2
                                color: "#cbd5e1"
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                companionWindow.widgetState = 3
                            }
                        }
                    }
                }

                DirectionalCard {
                    visible: stripSearchInput.text.trim().length > 0 && clinicalEngine.resultCount > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    cardRadius: 12
                    fillColor: "#141d26"
                    shadowStrength: 0.85

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        ContainedBadge {
                            textValue: clinicalEngine.topMatchTag
                            maxBadgeWidth: 118
                            badgeHeight: 22
                            pixelSize: 10
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: clinicalEngine.topMatchTitle
                                color: "#f8fafc"
                                font.bold: true
                                font.pixelSize: 12
                                elide: Text.ElideRight
                                maximumLineCount: 1
                                Layout.fillWidth: true
                            }

                            Text {
                                text: "Double-click to copy text • Single-click to add to conjoined note"
                                color: "#94a3b8"
                                font.pixelSize: 10
                                elide: Text.ElideRight
                                maximumLineCount: 1
                                Layout.fillWidth: true
                            }
                        }

                        Rectangle {
                            Layout.preferredHeight: 26
                            Layout.preferredWidth: 68
                            radius: 6
                            color: clinicalEngine.topMatchSelected ? "#059669" : "#2563eb"

                            Text {
                                anchors.centerIn: parent
                                text: clinicalEngine.topMatchSelected ? "✓ Added" : "+ Add"
                                color: "#ffffff"
                                font.pixelSize: 11
                                font.bold: true
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            cadenceTracker.recordUserActivity()
                            if (clinicalEngine.topMatchId.length > 0) {
                                clinicalEngine.toggleItemById(clinicalEngine.topMatchId)
                            }
                        }
                        onDoubleClicked: {
                            cadenceTracker.recordUserActivity()
                            if (clinicalEngine.topMatchText.length > 0) {
                                clinicalEngine.copyToClipboard()
                            }
                        }
                    }
                }
            }
        }

        // STATE 2: GRID SHEET (3-Column Modular Ratio: 2/3 Left 2x2 Options + 1/3 Right Conjoined Note)
        Item {
            anchors.fill: parent
            anchors.margins: 16
            visible: companionWindow.widgetState === 2

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: companionWindow.chapterMeta

                        DirectionalCard {
                            Layout.preferredHeight: 34
                            Layout.preferredWidth: 136
                            cardRadius: 10
                            fillColor: clinicalEngine.activeSection === index ? cadenceTracker.elevatedCardColor : "#151d26"
                            opacity: clinicalEngine.activeSection === index ? 1.0 : 0.84
                            topRightRimColor: {
                                if (clinicalEngine.activeSection === index) return cadenceTracker.topRightHighlightColor
                                if (clinicalEngine.recommendedNextSection === index && cadenceTracker.peripheralCueActive) return "#fde68a"
                                return "#475569"
                            }
                            midRimColor: {
                                if (clinicalEngine.activeSection === index) return cadenceTracker.specularRimColor
                                if (clinicalEngine.recommendedNextSection === index && cadenceTracker.peripheralCueActive) return "#fbbf24"
                                return "#2b394a"
                            }
                            rimWidth: clinicalEngine.activeSection === index ? 1.8 : 1.1
                            shadowStrength: clinicalEngine.activeSection === index ? 0.9 : 0.45

                            transform: Translate {
                                y: (clinicalEngine.recommendedNextSection === index) ? cadenceTracker.peripheralOffsetY * 0.65 : 0
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 6

                                Rectangle {
                                    width: 7
                                    height: 7
                                    radius: 3.5
                                    color: {
                                        const prog = clinicalEngine.sectionProgress[index] || 0
                                        if (prog >= 1.0) return "#34d399"
                                        if (clinicalEngine.recommendedNextSection === index) return "#fbbf24"
                                        return "#64748b"
                                    }
                                }

                                Text {
                                    text: modelData.label
                                    color: clinicalEngine.activeSection === index ? "#f8fafc" : "#cbd5e1"
                                    font.pixelSize: 11
                                    font.bold: clinicalEngine.activeSection === index
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    maximumLineCount: 1
                                }

                                Rectangle {
                                    Layout.preferredWidth: 22
                                    Layout.preferredHeight: 18
                                    radius: 5
                                    color: "#141c24"
                                    Text {
                                        anchors.centerIn: parent
                                        text: {
                                            const counts = [
                                                clinicalEngine.themesCount,
                                                clinicalEngine.interventionsCount,
                                                clinicalEngine.safetyCount,
                                                clinicalEngine.planCount
                                            ]
                                            return String(counts[index])
                                        }
                                        color: (clinicalEngine.sectionProgress[index] || 0) >= 1.0 ? "#34d399" : "#e2e8f0"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.activeSection = index
                                }
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    DirectionalCard {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        cardRadius: 8
                        shadowStrength: 0.55
                        Text { anchors.centerIn: parent; text: "◐"; font.pixelSize: 14; color: cadenceTracker.specularRimColor }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                cadenceTracker.cycleHorizonPhase()
                            }
                        }
                    }

                    DirectionalCard {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        cardRadius: 8
                        shadowStrength: 0.55

                        Rectangle {
                            anchors.centerIn: parent
                            width: 6
                            height: 6
                            radius: 3
                            color: "#93c5fd"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                companionWindow.widgetState = 0
                            }
                        }
                    }

                    DirectionalCard {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        cardRadius: 8
                        shadowStrength: 0.55

                        Rectangle {
                            anchors.centerIn: parent
                            width: 15
                            height: 3
                            radius: 1.5
                            color: "#cbd5e1"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                companionWindow.widgetState = 1
                            }
                        }
                    }

                    DirectionalCard {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        cardRadius: 8
                        shadowStrength: 0.55

                        Rectangle {
                            anchors.centerIn: parent
                            width: 12
                            height: 12
                            radius: 2
                            color: "transparent"
                            border.color: "#f8fafc"
                            border.width: 1.8
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                companionWindow.widgetState = 3
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        radius: 10
                        color: "#131b23"
                        border.color: sheetSearchInput.cursorVisible ? cadenceTracker.specularRimColor : "#2b394a"
                        clip: true

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 8
                            spacing: 6

                            Text { text: "⌕"; color: cadenceTracker.specularRimColor; font.pixelSize: 14 }

                            TextInput {
                                id: sheetSearchInput
                                Layout.fillWidth: true
                                verticalAlignment: TextInput.AlignVCenter
                                font.pixelSize: 12
                                color: "#f8fafc"
                                selectByMouse: true
                                clip: true
                                text: clinicalEngine.query

                                onTextEdited: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.query = text
                                }

                                Text {
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    text: companionWindow.chapterMeta[clinicalEngine.activeSection].placeholder
                                    color: "#64748b"
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                    visible: sheetSearchInput.text.length === 0
                                }
                            }

                            Rectangle {
                                visible: sheetSearchInput.text.length > 0
                                width: 20
                                height: 20
                                radius: 10
                                color: "#263240"
                                Text { anchors.centerIn: parent; text: "×"; color: "#cbd5e1"; font.pixelSize: 12 }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        cadenceTracker.recordUserActivity()
                                        clinicalEngine.query = ""
                                    }
                                }
                            }
                        }
                    }

                    DirectionalCard {
                        visible: (clinicalEngine.activeSection === 2 && clinicalEngine.safetyCount === 0) ||
                                 (clinicalEngine.activeSection === 3 && clinicalEngine.planCount === 0)
                        Layout.preferredHeight: 36
                        Layout.preferredWidth: 118
                        cardRadius: 10
                        fillColor: "#1b362c"
                        selected: true
                        shadowStrength: 0.65

                        Text {
                            anchors.centerIn: parent
                            text: clinicalEngine.activeSection === 2 ? "✓ Routine Safe" : "✓ Weekly Plan"
                            color: "#6ee7b7"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                clinicalEngine.quickCompleteRoutineSection(clinicalEngine.activeSection)
                            }
                        }
                    }

                    Row {
                        spacing: 5
                        DirectionalCard {
                            width: 30; height: 36; cardRadius: 8
                            shadowStrength: 0.5
                            Text { anchors.centerIn: parent; text: "‹"; color: "#e2e8f0"; font.pixelSize: 15 }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.prevSheetPage()
                                }
                            }
                        }
                        Rectangle {
                            width: 64; height: 36; radius: 8
                            color: "#131b23"
                            border.color: "#2b394a"
                            clip: true
                            Text {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.margins: 4
                                anchors.verticalCenter: parent.verticalCenter
                                horizontalAlignment: Text.AlignHCenter
                                text: String(clinicalEngine.sheetPage + 1) + " / " + String(clinicalEngine.sheetTotalPages)
                                color: "#94a3b8"
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                        }
                        DirectionalCard {
                            width: 30; height: 36; cardRadius: 8
                            shadowStrength: 0.5
                            Text { anchors.centerIn: parent; text: "›"; color: "#e2e8f0"; font.pixelSize: 15 }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.nextSheetPage()
                                }
                            }
                        }
                    }
                }

                Item {
                    id: sheetWorkspaceRow
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Item {
                        id: sheetMatrixContainer
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: Math.round((sheetWorkspaceRow.width - 14) * 0.64)

                        Grid {
                            id: sheetGrid
                            anchors.fill: parent
                            anchors.leftMargin: 6
                            anchors.bottomMargin: 6
                            columns: 2
                            spacing: 12

                            Repeater {
                                model: clinicalEngine.sheetCards

                                Item {
                                    id: cardSlot
                                    width: Math.floor((sheetGrid.width - sheetGrid.spacing) / 2)
                                    height: Math.floor((sheetGrid.height - sheetGrid.spacing) / 2)

                                    DirectionalCard {
                                        id: cardItem
                                        anchors.fill: parent
                                        cardRadius: 11
                                        hovered: cardInteriorMouse.containsMouse
                                        rimWidth: cardInteriorMouse.containsMouse ? 2.0 : 1.6
                                        shadowStrength: 1.0

                                        scale: (companionWindow.lastSettledCardId === modelData.itemId)
                                               ? companionWindow.cardSettleScale : 1.0

                                        ColumnLayout {
                                            id: cardInnerCol
                                            anchors.fill: parent
                                            anchors.margins: 11
                                            spacing: 5

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: 6

                                                ContainedBadge {
                                                    textValue: modelData.tag
                                                    maxBadgeWidth: Math.max(80, cardInnerCol.width - 34)
                                                    badgeHeight: 19
                                                    pixelSize: 9
                                                }

                                                Item { Layout.fillWidth: true }

                                                Rectangle {
                                                    width: 20
                                                    height: 20
                                                    radius: 10
                                                    color: cardInteriorMouse.containsMouse ? "#2563eb" : "#1e293b"
                                                    border.color: cardInteriorMouse.containsMouse ? "#93c5fd" : "#475569"
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "+"
                                                        color: "#ffffff"
                                                        font.pixelSize: 12
                                                        font.bold: true
                                                    }
                                                }
                                            }

                                            Text {
                                                Layout.fillWidth: true
                                                text: modelData.title
                                                font.bold: true
                                                font.pixelSize: 12
                                                color: "#f8fafc"
                                                wrapMode: Text.WordWrap
                                                elide: Text.ElideRight
                                                maximumLineCount: 2
                                            }

                                            Text {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                verticalAlignment: Text.AlignTop
                                                text: modelData.description
                                                font.pixelSize: 11
                                                color: "#cbd5e1"
                                                wrapMode: Text.WordWrap
                                                elide: Text.ElideRight
                                                maximumLineCount: 5
                                            }
                                        }

                                        MouseArea {
                                            id: cardInteriorMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                cadenceTracker.recordUserActivity()
                                                clinicalEngine.toggleItemById(modelData.itemId)
                                            }
                                            onWheel: (wheel) => {
                                                cadenceTracker.recordUserActivity()
                                                if (wheel.angleDelta.y < 0) {
                                                    clinicalEngine.nextSheetPage()
                                                } else if (wheel.angleDelta.y > 0) {
                                                    clinicalEngine.prevSheetPage()
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: Math.max(180, sheetWorkspaceRow.width - 14 - sheetMatrixContainer.width)

                        DirectionalCard {
                            anchors.fill: parent
                            anchors.bottomMargin: 6
                            cardRadius: 12
                            fillColor: "#14221d"
                            selected: true
                            rimWidth: 1.7
                            shadowStrength: 1.0

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Rectangle {
                                        width: 9; height: 9; radius: 4.5
                                        color: "#34d399"
                                    }

                                    Text {
                                        text: "Conjoined Note (" + String(clinicalEngine.totalSelectedCount) + ")"
                                        font.bold: true
                                        font.pixelSize: 12
                                        color: "#6ee7b7"
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                        maximumLineCount: 1
                                    }

                                    Rectangle {
                                        Layout.preferredHeight: 24
                                        Layout.preferredWidth: 68
                                        radius: 6
                                        color: companionWindow.copiedBannerActive ? "#059669" : "#10b981"

                                        Text {
                                            anchors.centerIn: parent
                                            text: companionWindow.copiedBannerActive ? "✓ Copied" : "📋 Copy"
                                            color: "#ffffff"
                                            font.pixelSize: 10
                                            font.bold: true
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                cadenceTracker.recordUserActivity()
                                                clinicalEngine.copyToClipboard()
                                            }
                                        }
                                    }
                                }

                                Flickable {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    contentWidth: width
                                    contentHeight: selectedChipsFlow.implicitHeight
                                    clip: true

                                    Flow {
                                        id: selectedChipsFlow
                                        width: parent.width
                                        spacing: 6

                                        Repeater {
                                            model: clinicalEngine.selectedChips

                                            Rectangle {
                                                width: selectedChipsFlow.width
                                                implicitHeight: chipLayout.implicitHeight + 10
                                                radius: 8
                                                color: "#1d3229"
                                                border.color: "#2e5242"
                                                clip: true

                                                RowLayout {
                                                    id: chipLayout
                                                    anchors.fill: parent
                                                    anchors.margins: 6
                                                    spacing: 6

                                                    ContainedBadge {
                                                        textValue: modelData.tag
                                                        textColor: "#6ee7b7"
                                                        fillColor: "#264739"
                                                        strokeColor: "#346350"
                                                        maxBadgeWidth: 102
                                                        badgeHeight: 18
                                                        pixelSize: 9
                                                    }

                                                    Text {
                                                        text: modelData.title
                                                        color: "#f0fdf4"
                                                        font.pixelSize: 11
                                                        Layout.fillWidth: true
                                                        wrapMode: Text.WordWrap
                                                        maximumLineCount: 2
                                                        elide: Text.ElideRight
                                                    }

                                                    Text {
                                                        text: "×"
                                                        color: "#94a3b8"
                                                        font.pixelSize: 13
                                                        font.bold: true
                                                    }
                                                }

                                                MouseArea {
                                                    anchors.fill: parent
                                                    cursorShape: Qt.PointingHandCursor
                                                    onClicked: {
                                                        cadenceTracker.recordUserActivity()
                                                        clinicalEngine.toggleItemById(modelData.itemId)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 104
                                    radius: 8
                                    color: "#0e1713"
                                    border.color: "#224233"
                                    clip: true

                                    Flickable {
                                        anchors.fill: parent
                                        anchors.margins: 7
                                        contentWidth: width
                                        contentHeight: narrativePreviewText.implicitHeight
                                        clip: true

                                        Text {
                                            id: narrativePreviewText
                                            width: parent.width
                                            text: clinicalEngine.conjoinedNarrative.length > 0
                                                ? clinicalEngine.conjoinedNarrative
                                                : "Selected options will be automatically conjoined here into your clinical note draft."
                                            color: clinicalEngine.conjoinedNarrative.length > 0 ? "#cbd5e1" : "#527967"
                                            font.pixelSize: 10
                                            wrapMode: Text.WordWrap
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 42
                    radius: 10
                    color: "#131b23"
                    border.color: "#2b394a"
                    clip: true

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 10
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 116
                            Layout.preferredHeight: 24
                            radius: 12
                            color: "#1e293b"
                            clip: true

                            Rectangle {
                                width: parent.width * clinicalEngine.endowedProgress
                                height: parent.height
                                radius: 12
                                color: clinicalEngine.recommendedNextSection === -1 ? "#10b981" : cadenceTracker.specularRimColor
                                opacity: 0.35
                                Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                            }

                            Text {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.margins: 6
                                anchors.verticalCenter: parent.verticalCenter
                                horizontalAlignment: Text.AlignHCenter
                                text: String(clinicalEngine.totalSelectedCount) + " items • " + String(clinicalEngine.wordCount) + "w"
                                color: "#e2e8f0"
                                font.pixelSize: 10
                                font.bold: true
                                elide: Text.ElideRight
                            }
                        }

                        Rectangle {
                            visible: clinicalEngine.recommendedNextSection >= 0
                            Layout.preferredHeight: 26
                            Layout.preferredWidth: nextStepText.implicitWidth + 20
                            radius: 8
                            color: "#1e293b"
                            border.color: cadenceTracker.peripheralCueActive ? "#fbbf24" : "#334155"
                            clip: true

                            Text {
                                id: nextStepText
                                anchors.centerIn: parent
                                text: "Next: " + companionWindow.chapterMeta[Math.max(0, clinicalEngine.recommendedNextSection)].label + " →"
                                color: "#cbd5e1"
                                font.pixelSize: 10
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.advanceToRecommendedSection()
                                }
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Rectangle {
                            Layout.preferredHeight: 28
                            Layout.preferredWidth: 88
                            radius: 8
                            color: companionWindow.copiedBannerActive ? "#059669" : "#2563eb"

                            Text {
                                anchors.centerIn: parent
                                text: companionWindow.copiedBannerActive ? "✓ Copied" : "📋 Copy"
                                color: "#ffffff"
                                font.pixelSize: 11
                                font.bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.copyToClipboard()
                                }
                            }
                        }

                        DirectionalCard {
                            Layout.preferredHeight: 28
                            Layout.preferredWidth: 32
                            cardRadius: 8
                            shadowStrength: 0.45

                            Text {
                                anchors.centerIn: parent
                                text: "↻"
                                color: "#cbd5e1"
                                font.pixelSize: 15
                                font.bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.clearAll()
                                }
                            }
                        }
                    }
                }
            }
        }

        // STATE 3: FULL CLINICAL WORKSPACE
        Item {
            anchors.fill: parent
            anchors.margins: 20
            visible: companionWindow.widgetState === 3

            ColumnLayout {
                anchors.fill: parent
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Item {
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 42

                        Shape {
                            anchors.fill: parent
                            layer.enabled: true
                            layer.samples: 4

                            ShapePath {
                                strokeColor: "#293545"
                                strokeWidth: 4
                                fillColor: "transparent"
                                PathAngleArc {
                                    centerX: 21; centerY: 21
                                    radiusX: 17; radiusY: 17
                                    startAngle: 0; sweepAngle: 360
                                }
                            }

                            ShapePath {
                                strokeColor: clinicalEngine.recommendedNextSection === -1 ? "#34d399" : cadenceTracker.specularRimColor
                                strokeWidth: 4
                                fillColor: "transparent"
                                capStyle: ShapePath.RoundCap
                                PathAngleArc {
                                    centerX: 21; centerY: 21
                                    radiusX: 17; radiusY: 17
                                    startAngle: -90
                                    sweepAngle: 360 * clinicalEngine.endowedProgress
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: Math.round(clinicalEngine.endowedProgress * 100) + "%"
                            color: "#f8fafc"
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Text {
                                text: "Therapist Documentation Companion"
                                font.bold: true
                                font.pixelSize: 17
                                color: "#f8fafc"
                                elide: Text.ElideRight
                            }
                            ContainedBadge {
                                textValue: "Liz Wegner Proprietary Software Collective"
                                textColor: "#6ee7b7"
                                fillColor: "#1b362c"
                                strokeColor: "#34d399"
                                maxBadgeWidth: 280
                                badgeHeight: 20
                                pixelSize: 10
                            }
                            Item { Layout.fillWidth: true }
                        }
                        Text {
                            Layout.fillWidth: true
                            text: "1,385 Clinical Interventions • 20 Modalities • 160 Session Themes • 100% Local & Private"
                            font.pixelSize: 11
                            color: "#94a3b8"
                            elide: Text.ElideRight
                        }
                    }

                    Rectangle {
                        visible: typeof updateChecker !== "undefined" && updateChecker.updateAvailable
                        Layout.preferredHeight: 28
                        Layout.preferredWidth: updateRow.implicitWidth + 16
                        radius: 14
                        color: "#059669"
                        border.color: "#34d399"
                        RowLayout {
                            id: updateRow
                            anchors.centerIn: parent
                            spacing: 5
                            Text { text: "⬆ Update Available (" + updateChecker.latestVersion + ")"; color: "#ffffff"; font.pixelSize: 10; font.bold: true }
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                updateChecker.openDownloadUrl()
                            }
                        }
                    }

                    ContainedBadge {
                        textValue: Math.round(cadenceTracker.currentFps) + " FPS • " + clinicalEngine.lastSearchLatencyMs.toFixed(2) + " ms"
                        textColor: "#94a3b8"
                        fillColor: "#131b23"
                        strokeColor: "#2b394a"
                        maxBadgeWidth: 160
                        badgeHeight: 28
                        pixelSize: 10
                        boldText: false
                    }

                    DirectionalCard {
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 32
                        cardRadius: 8
                        shadowStrength: 0.55
                        Text { anchors.centerIn: parent; text: "◐"; font.pixelSize: 15; color: cadenceTracker.specularRimColor }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                cadenceTracker.recordUserActivity()
                                cadenceTracker.cycleHorizonPhase()
                            }
                        }
                    }

                    Repeater {
                        model: [
                            { type: "dot",    stateVal: 0 },
                            { type: "line",   stateVal: 1 },
                            { type: "sheet",  stateVal: 2 },
                            { type: "square", stateVal: 3 }
                        ]
                        DirectionalCard {
                            Layout.preferredWidth: 34
                            Layout.preferredHeight: 32
                            cardRadius: 8
                            fillColor: companionWindow.widgetState === modelData.stateVal ? "#1e293b" : cadenceTracker.elevatedCardColor
                            rimWidth: companionWindow.widgetState === modelData.stateVal ? 1.8 : 1.2
                            shadowStrength: 0.55

                            Rectangle {
                                visible: modelData.type === "dot"
                                anchors.centerIn: parent
                                width: 7
                                height: 7
                                radius: 3.5
                                color: "#93c5fd"
                            }

                            Rectangle {
                                visible: modelData.type === "line"
                                anchors.centerIn: parent
                                width: 16
                                height: 3
                                radius: 1.5
                                color: "#cbd5e1"
                            }

                            Item {
                                visible: modelData.type === "sheet"
                                anchors.centerIn: parent
                                width: 16
                                height: 16

                                Rectangle {
                                    x: 0; y: 0
                                    width: 16; height: 3.2
                                    radius: 1.4
                                    color: "#60a5fa"
                                }

                                Grid {
                                    x: 0; y: 5.5
                                    columns: 2
                                    spacing: 2
                                    Repeater {
                                        model: 4
                                        Rectangle {
                                            width: 4.2
                                            height: 4.2
                                            radius: 1.2
                                            color: "#cbd5e1"
                                        }
                                    }
                                }

                                Rectangle {
                                    x: 12; y: 5.5
                                    width: 4
                                    height: 10.4
                                    radius: 1.4
                                    color: "#34d399"
                                }
                            }

                            Rectangle {
                                visible: modelData.type === "square"
                                anchors.centerIn: parent
                                width: 12
                                height: 12
                                radius: 2
                                color: "transparent"
                                border.color: "#f8fafc"
                                border.width: 1.8
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    cadenceTracker.recordUserActivity()
                                    companionWindow.widgetState = modelData.stateVal
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    id: chapterBarRow
                    Layout.fillWidth: true
                    spacing: 12

                    Repeater {
                        model: companionWindow.chapterMeta

                        DirectionalCard {
                            Layout.fillWidth: true
                            Layout.preferredWidth: Math.floor((chapterBarRow.width - 36) / 4)
                            Layout.preferredHeight: 56
                            cardRadius: 12
                            fillColor: clinicalEngine.activeSection === index ? cadenceTracker.elevatedCardColor : "#151d26"
                            opacity: clinicalEngine.activeSection === index ? 1.0 : 0.85
                            topRightRimColor: {
                                if (clinicalEngine.activeSection === index) return cadenceTracker.topRightHighlightColor
                                if (clinicalEngine.recommendedNextSection === index && cadenceTracker.peripheralCueActive) return "#fde68a"
                                return "#475569"
                            }
                            midRimColor: {
                                if (clinicalEngine.activeSection === index) return cadenceTracker.specularRimColor
                                if (clinicalEngine.recommendedNextSection === index && cadenceTracker.peripheralCueActive) return "#fbbf24"
                                return "#2a3746"
                            }
                            rimWidth: clinicalEngine.activeSection === index ? 2.0 : 1.2
                            shadowStrength: clinicalEngine.activeSection === index ? 1.0 : 0.6

                            transform: Translate {
                                y: (clinicalEngine.recommendedNextSection === index) ? cadenceTracker.peripheralOffsetY * 0.75 : 0
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 14
                                spacing: 10

                                Rectangle {
                                    width: 26
                                    height: 26
                                    radius: 13
                                    color: (clinicalEngine.sectionProgress[index] || 0) >= 1.0 ? "#10b981" : "#1e293b"
                                    border.color: (clinicalEngine.sectionProgress[index] || 0) >= 1.0 ? "#6ee7b7" : cadenceTracker.specularRimColor
                                    Text {
                                        anchors.centerIn: parent
                                        text: (clinicalEngine.sectionProgress[index] || 0) >= 1.0 ? "✓" : String(index + 1)
                                        color: "#ffffff"
                                        font.pixelSize: 11
                                        font.bold: true
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.label
                                        color: "#f8fafc"
                                        font.pixelSize: 13
                                        font.bold: true
                                        elide: Text.ElideRight
                                        maximumLineCount: 1
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: "Recommended: " + modelData.rangeText
                                        color: "#94a3b8"
                                        font.pixelSize: 10
                                        elide: Text.ElideRight
                                        maximumLineCount: 1
                                    }
                                }

                                Rectangle {
                                    Layout.preferredWidth: 34
                                    Layout.preferredHeight: 24
                                    radius: 8
                                    color: "#141c24"
                                    border.color: "#2b394a"
                                    Text {
                                        anchors.centerIn: parent
                                        text: {
                                            const counts = [
                                                clinicalEngine.themesCount,
                                                clinicalEngine.interventionsCount,
                                                clinicalEngine.safetyCount,
                                                clinicalEngine.planCount
                                            ]
                                            return String(counts[index])
                                        }
                                        color: (clinicalEngine.sectionProgress[index] || 0) >= 1.0 ? "#34d399" : "#f8fafc"
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    cadenceTracker.recordUserActivity()
                                    clinicalEngine.activeSection = index
                                }
                            }
                        }
                    }
                }

                Item {
                    id: studioRow
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        id: studioLeftCol
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: Math.round((studioRow.width - 16) * 0.58)
                        spacing: 10

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42
                            radius: 12
                            color: "#131b23"
                            border.color: fullSearchInput.cursorVisible ? cadenceTracker.specularRimColor : "#2b394a"
                            border.width: fullSearchInput.cursorVisible ? 1.5 : 1
                            clip: true

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 8
                                spacing: 8

                                Text { text: "⌕"; color: cadenceTracker.specularRimColor; font.pixelSize: 15 }

                                TextInput {
                                    id: fullSearchInput
                                    Layout.fillWidth: true
                                    verticalAlignment: TextInput.AlignVCenter
                                    font.pixelSize: 13
                                    color: "#f8fafc"
                                    selectByMouse: true
                                    clip: true
                                    text: clinicalEngine.query

                                    onTextEdited: {
                                        cadenceTracker.recordUserActivity()
                                        clinicalEngine.query = text
                                    }

                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        text: companionWindow.chapterMeta[clinicalEngine.activeSection].placeholder
                                        color: "#64748b"
                                        font.pixelSize: 12
                                        elide: Text.ElideRight
                                        visible: fullSearchInput.text.length === 0
                                    }
                                }

                                Row {
                                    spacing: 4
                                    Repeater {
                                        model: [
                                            { label: "Hybrid", modeVal: "hybrid" },
                                            { label: "BM25",   modeVal: "lexical" },
                                            { label: "Vector", modeVal: "semantic" }
                                        ]
                                        Rectangle {
                                            width: 54
                                            height: 26
                                            radius: 6
                                            color: clinicalEngine.searchMode === modelData.modeVal ? cadenceTracker.specularRimColor : "#1e293b"
                                            Text {
                                                anchors.centerIn: parent
                                                text: modelData.label
                                                color: clinicalEngine.searchMode === modelData.modeVal ? "#0f172a" : "#cbd5e1"
                                                font.pixelSize: 10
                                                font.bold: true
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    cadenceTracker.recordUserActivity()
                                                    clinicalEngine.searchMode = modelData.modeVal
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        DirectionalCard {
                            visible: clinicalEngine.modalityHintActive && clinicalEngine.activeSection === 1
                            Layout.fillWidth: true
                            Layout.preferredHeight: 34
                            cardRadius: 8
                            fillColor: "#1b332b"
                            selected: true
                            shadowStrength: 0.55

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 10
                                spacing: 8

                                Text {
                                    text: "✦ Modality Cue: " + clinicalEngine.detectedModality +
                                          " (" + String(clinicalEngine.detectedConfidencePercent) + "% on \"" + clinicalEngine.detectedCuesText + "\")"
                                    color: "#a7f3d0"
                                    font.pixelSize: 11
                                    font.bold: true
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    maximumLineCount: 1
                                }

                                Rectangle {
                                    visible: clinicalEngine.selectedModality !== clinicalEngine.detectedModality
                                    Layout.preferredHeight: 22
                                    Layout.preferredWidth: 98
                                    radius: 6
                                    color: "#10b981"
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Filter Modality"
                                        color: "#06281e"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            cadenceTracker.recordUserActivity()
                                            clinicalEngine.selectedModality = clinicalEngine.detectedModality
                                        }
                                    }
                                }
                            }
                        }

                        Flickable {
                            id: modalityFlickable
                            visible: clinicalEngine.activeSection === 1
                            Layout.fillWidth: true
                            Layout.preferredHeight: 32
                            contentWidth: modalityRow.implicitWidth
                            contentHeight: 32
                            clip: true
                            boundsBehavior: Flickable.StopAtBounds

                            WheelHandler {
                                onWheel: (event) => {
                                    const delta = event.angleDelta.y !== 0 ? event.angleDelta.y : event.angleDelta.x
                                    const maxScroll = Math.max(0, modalityFlickable.contentWidth - modalityFlickable.width)
                                    modalityFlickable.contentX = Math.max(0, Math.min(maxScroll, modalityFlickable.contentX - delta * 0.6))
                                }
                            }

                            Row {
                                id: modalityRow
                                spacing: 6

                                Rectangle {
                                    height: 28
                                    width: 82
                                    radius: 8
                                    color: clinicalEngine.selectedModality === "all" ? cadenceTracker.specularRimColor : "#1e293b"
                                    border.color: "#334155"
                                    Text {
                                        anchors.centerIn: parent
                                        text: "All (1,385)"
                                        color: clinicalEngine.selectedModality === "all" ? "#0f172a" : "#cbd5e1"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            cadenceTracker.recordUserActivity()
                                            clinicalEngine.selectedModality = "all"
                                        }
                                    }
                                }

                                Repeater {
                                    model: clinicalEngine.availableModalities
                                    Rectangle {
                                        height: 28
                                        width: modPillText.implicitWidth + 18
                                        radius: 8
                                        color: clinicalEngine.selectedModality === modelData ? cadenceTracker.specularRimColor : "#1e293b"
                                        border.color: "#334155"
                                        clip: true
                                        Text {
                                            id: modPillText
                                            anchors.centerIn: parent
                                            text: modelData
                                            color: clinicalEngine.selectedModality === modelData ? "#0f172a" : "#cbd5e1"
                                            font.pixelSize: 10
                                            font.bold: clinicalEngine.selectedModality === modelData
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                cadenceTracker.recordUserActivity()
                                                clinicalEngine.selectedModality = modelData
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        ListView {
                            id: fullResultsList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            spacing: 6
                            model: clinicalEngine

                            delegate: Item {
                                width: fullResultsList.width
                                height: 112

                                DirectionalCard {
                                    id: listCard
                                    x: 8
                                    y: 2
                                    width: parent.width - 10
                                    height: parent.height - 8
                                    cardRadius: 12
                                    fillColor: isSelected ? "#193028" : cadenceTracker.elevatedCardColor
                                    selected: isSelected
                                    hovered: listEdgeDragMouse.containsMouse || listInteriorClickMouse.containsMouse
                                    rimWidth: isSelected ? 2.0 : 1.6
                                    shadowStrength: 1.0

                                    scale: (companionWindow.lastSettledCardId === itemId)
                                           ? companionWindow.cardSettleScale : 1.0

                                    Drag.active: listEdgeDragMouse.drag.active
                                    Drag.mimeData: { "text/plain": fullText }
                                    Drag.dragType: Drag.Automatic

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 12

                                        ColumnLayout {
                                            id: listCardCol
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            spacing: 4

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: 6

                                                ContainedBadge {
                                                    visible: shortModality.length > 0
                                                    textValue: shortModality
                                                    maxBadgeWidth: 132
                                                    badgeHeight: 19
                                                    pixelSize: 9
                                                }

                                                ContainedBadge {
                                                    textValue: category
                                                    textColor: "#cbd5e1"
                                                    fillColor: "#131b23"
                                                    strokeColor: "#2b394a"
                                                    boldText: false
                                                    maxBadgeWidth: 210
                                                    badgeHeight: 19
                                                    pixelSize: 9
                                                }

                                                Item { Layout.fillWidth: true }
                                            }

                                            Text {
                                                Layout.fillWidth: true
                                                text: title
                                                color: "#f8fafc"
                                                font.pixelSize: 13
                                                font.bold: true
                                                elide: Text.ElideRight
                                                maximumLineCount: 1
                                            }

                                            Text {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                verticalAlignment: Text.AlignTop
                                                text: description
                                                color: "#cbd5e1"
                                                font.pixelSize: 11
                                                wrapMode: Text.WordWrap
                                                elide: Text.ElideRight
                                                maximumLineCount: 2
                                            }
                                        }

                                        Rectangle {
                                            Layout.preferredWidth: 28
                                            Layout.preferredHeight: 28
                                            radius: 14
                                            color: isSelected ? "#10b981" : (listInteriorClickMouse.containsMouse ? "#2563eb" : "#1e293b")
                                            border.color: isSelected ? "#6ee7b7" : "#475569"
                                            Text {
                                                anchors.centerIn: parent
                                                text: isSelected ? "✓" : "+"
                                                color: "#ffffff"
                                                font.pixelSize: 13
                                                font.bold: true
                                            }
                                        }
                                    }

                                    MouseArea {
                                        id: listEdgeDragMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                                        drag.target: listCard
                                        onPressed: {
                                            cadenceTracker.recordUserActivity()
                                            listCard.Drag.start()
                                        }
                                        onReleased: {
                                            listCard.Drag.drop()
                                            listCard.x = 8
                                            listCard.y = 2
                                        }
                                    }

                                    MouseArea {
                                        id: listInteriorClickMouse
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            cadenceTracker.recordUserActivity()
                                            clinicalEngine.toggleItemById(itemId)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: Math.max(260, studioRow.width - 16 - studioLeftCol.width)

                        DirectionalCard {
                            anchors.fill: parent
                            anchors.leftMargin: 6
                            anchors.bottomMargin: 6
                            cardRadius: 14
                            fillColor: "#141d26"
                            topRightRimColor: companionWindow.reEntryWave > 0.15 ? "#93c5fd" : cadenceTracker.topRightHighlightColor
                            midRimColor: companionWindow.reEntryWave > 0.15 ? "#60a5fa" : cadenceTracker.specularRimColor
                            rimWidth: companionWindow.reEntryWave > 0.15 ? 2.2 : 1.6
                            shadowStrength: 1.0

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 10

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text {
                                            Layout.fillWidth: true
                                            text: "Conjoined Clinical Note (" + String(clinicalEngine.totalSelectedCount) + ")"
                                            color: "#f8fafc"
                                            font.pixelSize: 14
                                            font.bold: true
                                            elide: Text.ElideRight
                                            maximumLineCount: 1
                                        }
                                        Text {
                                            Layout.fillWidth: true
                                            text: "Auto-assembled structured progress record"
                                            color: "#94a3b8"
                                            font.pixelSize: 11
                                            elide: Text.ElideRight
                                            maximumLineCount: 1
                                        }
                                    }

                                    DirectionalCard {
                                        Layout.preferredHeight: 32
                                        Layout.preferredWidth: 74
                                        cardRadius: 8
                                        fillColor: companionWindow.savedBannerActive ? "#059669" : "#1e293b"
                                        shadowStrength: 0.5
                                        Text {
                                            anchors.centerIn: parent
                                            text: companionWindow.savedBannerActive ? "✓ Saved" : "Save .txt"
                                            color: "#e2e8f0"
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                cadenceTracker.recordUserActivity()
                                                clinicalEngine.saveToDefaultNoteFile()
                                            }
                                        }
                                    }

                                    Rectangle {
                                        Layout.preferredHeight: 32
                                        Layout.preferredWidth: 102
                                        radius: 8
                                        color: companionWindow.copiedBannerActive ? "#059669" : "#2563eb"
                                        Text {
                                            anchors.centerIn: parent
                                            text: companionWindow.copiedBannerActive ? "✓ Copied!" : "📋 Copy Note"
                                            color: "#ffffff"
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                cadenceTracker.recordUserActivity()
                                                clinicalEngine.copyToClipboard()
                                            }
                                        }
                                    }

                                    Item {
                                        Layout.preferredWidth: 34
                                        Layout.preferredHeight: 32

                                        DirectionalCard {
                                            id: fullNoteDragProxy
                                            width: 34
                                            height: 32
                                            cardRadius: 8
                                            rimWidth: 1.8
                                            shadowStrength: 0.55

                                            Text {
                                                anchors.centerIn: parent
                                                text: "⇄"
                                                color: cadenceTracker.specularRimColor
                                                font.pixelSize: 14
                                                font.bold: true
                                            }

                                            Drag.active: fullNoteDragMouse.drag.active
                                            Drag.mimeData: { "text/plain": clinicalEngine.conjoinedNarrative }
                                            Drag.dragType: Drag.Automatic

                                            MouseArea {
                                                id: fullNoteDragMouse
                                                anchors.fill: parent
                                                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                                                drag.target: fullNoteDragProxy
                                                onPressed: {
                                                    cadenceTracker.recordUserActivity()
                                                    fullNoteDragProxy.Drag.start()
                                                }
                                                onReleased: {
                                                    fullNoteDragProxy.Drag.drop()
                                                    fullNoteDragProxy.x = 0
                                                    fullNoteDragProxy.y = 0
                                                }
                                            }
                                        }
                                    }
                                }

                                Flickable {
                                    visible: clinicalEngine.totalSelectedCount > 0
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: Math.min(84, chipsFlow.implicitHeight + 8)
                                    contentWidth: width
                                    contentHeight: chipsFlow.implicitHeight
                                    clip: true

                                    Flow {
                                        id: chipsFlow
                                        width: parent.width
                                        spacing: 6

                                        Repeater {
                                            model: clinicalEngine.selectedChips

                                            Rectangle {
                                                height: 24
                                                width: Math.min(220, chipText.implicitWidth + 34)
                                                radius: 6
                                                color: "#1e293b"
                                                border.color: "#334155"
                                                clip: true

                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.leftMargin: 8
                                                    anchors.rightMargin: 6
                                                    spacing: 4

                                                    Text {
                                                        id: chipText
                                                        Layout.fillWidth: true
                                                        text: modelData.tag + ": " + modelData.title
                                                        color: "#e2e8f0"
                                                        font.pixelSize: 10
                                                        elide: Text.ElideRight
                                                        maximumLineCount: 1
                                                    }

                                                    Text {
                                                        text: "×"
                                                        color: "#f87171"
                                                        font.pixelSize: 12
                                                        font.bold: true
                                                        MouseArea {
                                                            anchors.fill: parent
                                                            cursorShape: Qt.PointingHandCursor
                                                            onClicked: {
                                                                cadenceTracker.recordUserActivity()
                                                                clinicalEngine.removeItemById(modelData.itemId)
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: "CLINICAL NARRATIVE"
                                        color: "#94a3b8"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                    Item { Layout.fillWidth: true }
                                    Text {
                                        text: "↻ Re-conjoin"
                                        color: "#6ee7b7"
                                        font.pixelSize: 11
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                cadenceTracker.recordUserActivity()
                                                clinicalEngine.regenerateNarrative()
                                            }
                                        }
                                    }
                                    Text {
                                        text: "Clear All"
                                        color: "#f87171"
                                        font.pixelSize: 11
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                cadenceTracker.recordUserActivity()
                                                clinicalEngine.clearAll()
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    radius: 10
                                    color: "#0e151d"
                                    border.color: companionWindow.reEntryWave > 0.1
                                                  ? "#38bdf8"
                                                  : (narrativeEditor.cursorVisible ? cadenceTracker.specularRimColor : "#263240")
                                    border.width: companionWindow.reEntryWave > 0.1 ? 2 : 1
                                    clip: true

                                    ScrollView {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        clip: true

                                        TextArea {
                                            id: narrativeEditor
                                            text: clinicalEngine.conjoinedNarrative
                                            color: "#f1f5f9"
                                            font.pixelSize: 12
                                            wrapMode: TextArea.Wrap
                                            selectByMouse: true
                                            background: null

                                            onTextChanged: {
                                                if (text !== clinicalEngine.conjoinedNarrative) {
                                                    cadenceTracker.recordUserActivity()
                                                    clinicalEngine.conjoinedNarrative = text
                                                }
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: String(clinicalEngine.wordCount) + " words • " + String(clinicalEngine.charCount) + " characters"
                                        color: "#94a3b8"
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }
                                    Item { Layout.fillWidth: true }
                                    Text {
                                        text: "✦ Ready for EHR paste or edge-drag"
                                        color: "#6ee7b7"
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
