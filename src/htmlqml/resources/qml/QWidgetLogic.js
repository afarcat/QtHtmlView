.pragma library // shared library
.import QtQuick 2.0 as QQ

var QCheckBoxComponent = Qt.createComponent("QCheckBox.qml");
var QComboBoxComponent = Qt.createComponent("QComboBox.qml");
var QLineEditComponent = Qt.createComponent("QLineEdit.qml");
var QListWidgetComponent = Qt.createComponent("QListWidget.qml");
var QPushButtonComponent = Qt.createComponent("QPushButton.qml");
var QRadioButtonComponent = Qt.createComponent("QRadioButton.qml");
var QScrollBarComponent = Qt.createComponent("QScrollBar.qml");
var QTextEditComponent = Qt.createComponent("QTextEdit.qml");

function createQmlWidget(parent, className) {
    var WidgetComponent = null;

    if (className === "QCheckBox")
        WidgetComponent = QCheckBoxComponent;
    else if (className === "QComboBox")
        WidgetComponent = QComboBoxComponent;
    else if (className === "QLineEdit")
        WidgetComponent = QLineEditComponent;
    else if (className === "QListWidget")
        WidgetComponent = QListWidgetComponent;
    else if (className === "QPushButton")
        WidgetComponent = QPushButtonComponent;
    else if (className === "QRadioButton")
        WidgetComponent = QRadioButtonComponent;
    else if (className === "QScrollBar")
        WidgetComponent = QScrollBarComponent;
    else if (className === "QTextEdit")
        WidgetComponent = QTextEditComponent;

    if (WidgetComponent === null) {
        console.log("unknown className=", className);
    }

    if (WidgetComponent.status === QQ.Component.Ready) {
        var widget = WidgetComponent.createObject(
                    parent,
                    {"visible": true}); //default is visible

        if (widget === null) {
            // Error Handling
            console.log("Error creating object");

            return null;
        }

        return widget;
    }
    else if (WidgetComponent.status === QQ.Component.Error) {
        // Error Handling
        console.log("Error loading component:", WidgetComponent.errorString());

        return null;
    }

    return null;
}
