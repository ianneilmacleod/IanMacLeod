# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_som_om.ui'
#
# Created by: PyQt4 UI code generator 4.11.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_som_om(object):
    def setupUi(self, som_om):
        som_om.setObjectName(_fromUtf8("som_om"))
        som_om.resize(438, 777)
        self.layoutWidget = QtGui.QWidget(som_om)
        self.layoutWidget.setGeometry(QtCore.QRect(10, 10, 421, 34))
        self.layoutWidget.setObjectName(_fromUtf8("layoutWidget"))
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.layoutWidget)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.label_5 = QtGui.QLabel(self.layoutWidget)
        font = QtGui.QFont()
        font.setBold(True)
        font.setWeight(75)
        self.label_5.setFont(font)
        self.label_5.setObjectName(_fromUtf8("label_5"))
        self.verticalLayout_2.addWidget(self.label_5)
        self.databaseName = QtGui.QLabel(self.layoutWidget)
        self.databaseName.setObjectName(_fromUtf8("databaseName"))
        self.verticalLayout_2.addWidget(self.databaseName)
        self.layoutWidget1 = QtGui.QWidget(som_om)
        self.layoutWidget1.setGeometry(QtCore.QRect(332, 47, 101, 461))
        self.layoutWidget1.setObjectName(_fromUtf8("layoutWidget1"))
        self.verticalLayout_3 = QtGui.QVBoxLayout(self.layoutWidget1)
        self.verticalLayout_3.setObjectName(_fromUtf8("verticalLayout_3"))
        self.label_6 = QtGui.QLabel(self.layoutWidget1)
        self.label_6.setAlignment(QtCore.Qt.AlignCenter)
        self.label_6.setObjectName(_fromUtf8("label_6"))
        self.verticalLayout_3.addWidget(self.label_6)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.norm = QtGui.QComboBox(self.layoutWidget1)
        self.norm.setObjectName(_fromUtf8("norm"))
        self.verticalLayout.addWidget(self.norm)
        self.norm_1 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_1.setObjectName(_fromUtf8("norm_1"))
        self.verticalLayout.addWidget(self.norm_1)
        self.norm_2 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_2.setObjectName(_fromUtf8("norm_2"))
        self.verticalLayout.addWidget(self.norm_2)
        self.norm_3 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_3.setObjectName(_fromUtf8("norm_3"))
        self.verticalLayout.addWidget(self.norm_3)
        self.norm_4 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_4.setObjectName(_fromUtf8("norm_4"))
        self.verticalLayout.addWidget(self.norm_4)
        self.norm_5 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_5.setObjectName(_fromUtf8("norm_5"))
        self.verticalLayout.addWidget(self.norm_5)
        self.norm_6 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_6.setObjectName(_fromUtf8("norm_6"))
        self.verticalLayout.addWidget(self.norm_6)
        self.norm_7 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_7.setObjectName(_fromUtf8("norm_7"))
        self.verticalLayout.addWidget(self.norm_7)
        self.norm_8 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_8.setObjectName(_fromUtf8("norm_8"))
        self.verticalLayout.addWidget(self.norm_8)
        self.norm_9 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_9.setObjectName(_fromUtf8("norm_9"))
        self.verticalLayout.addWidget(self.norm_9)
        self.norm_10 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_10.setObjectName(_fromUtf8("norm_10"))
        self.verticalLayout.addWidget(self.norm_10)
        self.norm_11 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_11.setObjectName(_fromUtf8("norm_11"))
        self.verticalLayout.addWidget(self.norm_11)
        self.norm_12 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_12.setObjectName(_fromUtf8("norm_12"))
        self.verticalLayout.addWidget(self.norm_12)
        self.norm_13 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_13.setObjectName(_fromUtf8("norm_13"))
        self.verticalLayout.addWidget(self.norm_13)
        self.norm_14 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_14.setObjectName(_fromUtf8("norm_14"))
        self.verticalLayout.addWidget(self.norm_14)
        self.norm_15 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_15.setObjectName(_fromUtf8("norm_15"))
        self.verticalLayout.addWidget(self.norm_15)
        self.norm_16 = QtGui.QComboBox(self.layoutWidget1)
        self.norm_16.setObjectName(_fromUtf8("norm_16"))
        self.verticalLayout.addWidget(self.norm_16)
        self.verticalLayout_3.addLayout(self.verticalLayout)
        self.layoutWidget2 = QtGui.QWidget(som_om)
        self.layoutWidget2.setGeometry(QtCore.QRect(10, 620, 421, 148))
        self.layoutWidget2.setObjectName(_fromUtf8("layoutWidget2"))
        self.verticalLayout_5 = QtGui.QVBoxLayout(self.layoutWidget2)
        self.verticalLayout_5.setObjectName(_fromUtf8("verticalLayout_5"))
        self.gridLayout = QtGui.QGridLayout()
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.filtLab = QtGui.QLabel(self.layoutWidget2)
        font = QtGui.QFont()
        font.setBold(True)
        font.setWeight(75)
        self.filtLab.setFont(font)
        self.filtLab.setObjectName(_fromUtf8("filtLab"))
        self.gridLayout.addWidget(self.filtLab, 0, 0, 1, 1)
        self.label = QtGui.QLabel(self.layoutWidget2)
        font = QtGui.QFont()
        font.setBold(True)
        font.setWeight(75)
        self.label.setFont(font)
        self.label.setObjectName(_fromUtf8("label"))
        self.gridLayout.addWidget(self.label, 0, 1, 1, 1)
        self.filterChan = QtGui.QComboBox(self.layoutWidget2)
        self.filterChan.setEditable(False)
        self.filterChan.setObjectName(_fromUtf8("filterChan"))
        self.gridLayout.addWidget(self.filterChan, 1, 0, 1, 1)
        self.label_3 = QtGui.QLabel(self.layoutWidget2)
        font = QtGui.QFont()
        font.setBold(True)
        font.setWeight(75)
        self.label_3.setFont(font)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.gridLayout.addWidget(self.label_3, 2, 0, 1, 1)
        self.label_4 = QtGui.QLabel(self.layoutWidget2)
        font = QtGui.QFont()
        font.setBold(True)
        font.setWeight(75)
        self.label_4.setFont(font)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.gridLayout.addWidget(self.label_4, 2, 1, 1, 1)
        self.outClass = QtGui.QLineEdit(self.layoutWidget2)
        self.outClass.setObjectName(_fromUtf8("outClass"))
        self.gridLayout.addWidget(self.outClass, 3, 0, 1, 1)
        self.outError = QtGui.QLineEdit(self.layoutWidget2)
        self.outError.setObjectName(_fromUtf8("outError"))
        self.gridLayout.addWidget(self.outError, 3, 1, 1, 1)
        self.filterVal = QtGui.QLineEdit(self.layoutWidget2)
        self.filterVal.setObjectName(_fromUtf8("filterVal"))
        self.gridLayout.addWidget(self.filterVal, 1, 1, 1, 1)
        self.verticalLayout_5.addLayout(self.gridLayout)
        self.gridLayout_2 = QtGui.QGridLayout()
        self.gridLayout_2.setObjectName(_fromUtf8("gridLayout_2"))
        self.classButton = QtGui.QPushButton(self.layoutWidget2)
        font = QtGui.QFont()
        font.setBold(True)
        font.setWeight(75)
        self.classButton.setFont(font)
        self.classButton.setObjectName(_fromUtf8("classButton"))
        self.gridLayout_2.addWidget(self.classButton, 0, 0, 1, 1)
        self.progLabel = QtGui.QLabel(self.layoutWidget2)
        self.progLabel.setObjectName(_fromUtf8("progLabel"))
        self.gridLayout_2.addWidget(self.progLabel, 0, 1, 1, 1)
        self.stopButton = QtGui.QPushButton(self.layoutWidget2)
        font = QtGui.QFont()
        font.setBold(True)
        font.setWeight(75)
        self.stopButton.setFont(font)
        self.stopButton.setObjectName(_fromUtf8("stopButton"))
        self.gridLayout_2.addWidget(self.stopButton, 1, 0, 1, 1)
        self.progressBar = QtGui.QProgressBar(self.layoutWidget2)
        self.progressBar.setProperty("value", 0)
        self.progressBar.setTextVisible(False)
        self.progressBar.setObjectName(_fromUtf8("progressBar"))
        self.gridLayout_2.addWidget(self.progressBar, 1, 1, 1, 1)
        self.verticalLayout_5.addLayout(self.gridLayout_2)
        self.line = QtGui.QFrame(som_om)
        self.line.setGeometry(QtCore.QRect(12, 511, 421, 16))
        self.line.setFrameShape(QtGui.QFrame.HLine)
        self.line.setFrameShadow(QtGui.QFrame.Sunken)
        self.line.setObjectName(_fromUtf8("line"))
        self.layoutWidget3 = QtGui.QWidget(som_om)
        self.layoutWidget3.setGeometry(QtCore.QRect(14, 57, 311, 450))
        self.layoutWidget3.setObjectName(_fromUtf8("layoutWidget3"))
        self.verticalLayout_4 = QtGui.QVBoxLayout(self.layoutWidget3)
        self.verticalLayout_4.setObjectName(_fromUtf8("verticalLayout_4"))
        self.label_9 = QtGui.QLabel(self.layoutWidget3)
        self.label_9.setText(_fromUtf8(""))
        self.label_9.setObjectName(_fromUtf8("label_9"))
        self.verticalLayout_4.addWidget(self.label_9)
        self.label_2 = QtGui.QLabel(self.layoutWidget3)
        font = QtGui.QFont()
        font.setBold(True)
        font.setWeight(75)
        self.label_2.setFont(font)
        self.label_2.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.verticalLayout_4.addWidget(self.label_2)
        self.chan_1 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_1.setObjectName(_fromUtf8("chan_1"))
        self.verticalLayout_4.addWidget(self.chan_1)
        self.chan_2 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_2.setObjectName(_fromUtf8("chan_2"))
        self.verticalLayout_4.addWidget(self.chan_2)
        self.chan_3 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_3.setObjectName(_fromUtf8("chan_3"))
        self.verticalLayout_4.addWidget(self.chan_3)
        self.chan_4 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_4.setObjectName(_fromUtf8("chan_4"))
        self.verticalLayout_4.addWidget(self.chan_4)
        self.chan_5 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_5.setObjectName(_fromUtf8("chan_5"))
        self.verticalLayout_4.addWidget(self.chan_5)
        self.chan_6 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_6.setObjectName(_fromUtf8("chan_6"))
        self.verticalLayout_4.addWidget(self.chan_6)
        self.chan_7 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_7.setObjectName(_fromUtf8("chan_7"))
        self.verticalLayout_4.addWidget(self.chan_7)
        self.chan_8 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_8.setObjectName(_fromUtf8("chan_8"))
        self.verticalLayout_4.addWidget(self.chan_8)
        self.chan_9 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_9.setObjectName(_fromUtf8("chan_9"))
        self.verticalLayout_4.addWidget(self.chan_9)
        self.chan_10 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_10.setObjectName(_fromUtf8("chan_10"))
        self.verticalLayout_4.addWidget(self.chan_10)
        self.chan_11 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_11.setObjectName(_fromUtf8("chan_11"))
        self.verticalLayout_4.addWidget(self.chan_11)
        self.chan_12 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_12.setObjectName(_fromUtf8("chan_12"))
        self.verticalLayout_4.addWidget(self.chan_12)
        self.chan_13 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_13.setObjectName(_fromUtf8("chan_13"))
        self.verticalLayout_4.addWidget(self.chan_13)
        self.chan_14 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_14.setObjectName(_fromUtf8("chan_14"))
        self.verticalLayout_4.addWidget(self.chan_14)
        self.chan_15 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_15.setObjectName(_fromUtf8("chan_15"))
        self.verticalLayout_4.addWidget(self.chan_15)
        self.chan_16 = QtGui.QComboBox(self.layoutWidget3)
        self.chan_16.setObjectName(_fromUtf8("chan_16"))
        self.verticalLayout_4.addWidget(self.chan_16)
        self.layoutWidget4 = QtGui.QWidget(som_om)
        self.layoutWidget4.setGeometry(QtCore.QRect(88, 530, 238, 76))
        self.layoutWidget4.setObjectName(_fromUtf8("layoutWidget4"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.layoutWidget4)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.verticalLayout_7 = QtGui.QVBoxLayout()
        self.verticalLayout_7.setObjectName(_fromUtf8("verticalLayout_7"))
        self.similarity = QtGui.QLabel(self.layoutWidget4)
        self.similarity.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.similarity.setObjectName(_fromUtf8("similarity"))
        self.verticalLayout_7.addWidget(self.similarity)
        self.label_7 = QtGui.QLabel(self.layoutWidget4)
        self.label_7.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.label_7.setObjectName(_fromUtf8("label_7"))
        self.verticalLayout_7.addWidget(self.label_7)
        self.label_8 = QtGui.QLabel(self.layoutWidget4)
        self.label_8.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.label_8.setObjectName(_fromUtf8("label_8"))
        self.verticalLayout_7.addWidget(self.label_8)
        self.horizontalLayout.addLayout(self.verticalLayout_7)
        self.verticalLayout_6 = QtGui.QVBoxLayout()
        self.verticalLayout_6.setObjectName(_fromUtf8("verticalLayout_6"))
        self.similarity_func = QtGui.QComboBox(self.layoutWidget4)
        self.similarity_func.setCurrentText(_fromUtf8(""))
        self.similarity_func.setObjectName(_fromUtf8("similarity_func"))
        self.verticalLayout_6.addWidget(self.similarity_func)
        self.nClasses = QtGui.QComboBox(self.layoutWidget4)
        self.nClasses.setObjectName(_fromUtf8("nClasses"))
        self.verticalLayout_6.addWidget(self.nClasses)
        self.anomPercent = QtGui.QLineEdit(self.layoutWidget4)
        self.anomPercent.setObjectName(_fromUtf8("anomPercent"))
        self.verticalLayout_6.addWidget(self.anomPercent)
        self.horizontalLayout.addLayout(self.verticalLayout_6)
        self.layoutWidget.raise_()
        self.layoutWidget.raise_()
        self.layoutWidget.raise_()
        self.layoutWidget.raise_()
        self.layoutWidget.raise_()
        self.line.raise_()

        self.retranslateUi(som_om)
        self.similarity_func.setCurrentIndex(-1)
        QtCore.QMetaObject.connectSlotsByName(som_om)

    def retranslateUi(self, som_om):
        som_om.setWindowTitle(_translate("som_om", "Self Organizing Classification", None))
        self.label_5.setText(_translate("som_om", "Database:", None))
        self.databaseName.setText(_translate("som_om", "database name...", None))
        self.label_6.setText(_translate("som_om", "normalize", None))
        self.filtLab.setText(_translate("som_om", "Filter on:", None))
        self.label.setText(_translate("som_om", "Filter value:", None))
        self.label_3.setText(_translate("som_om", "Save classification to:", None))
        self.label_4.setText(_translate("som_om", "Save fit to:", None))
        self.classButton.setText(_translate("som_om", "Classify", None))
        self.progLabel.setText(_translate("som_om", "...", None))
        self.stopButton.setText(_translate("som_om", "Stop", None))
        self.label_2.setText(_translate("som_om", "Analyse data:", None))
        self.similarity.setText(_translate("som_om", "Similarity function", None))
        self.label_7.setText(_translate("som_om", "Base classes", None))
        self.label_8.setText(_translate("som_om", "Anomalous %", None))
        self.similarity_func.setToolTip(_translate("som_om", "Method to determine similarity", None))
        self.nClasses.setToolTip(_translate("som_om", "Number of base classes", None))

