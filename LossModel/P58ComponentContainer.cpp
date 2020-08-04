/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written: fmckenna, adamzs

#include "P58ComponentContainer.h"
#include "P58ComponentGroup.h"
#include "SimCenterPreferences.h"

#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QDebug>
#include <sectiontitle.h>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QFileDialog>
#include <QScrollArea>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QStringListModel>
#include <QJsonDocument>
#include <QHBoxLayout>
#include <QMap>
#include <QCoreApplication>
#include <QSettings>

P58ComponentContainer::P58ComponentContainer(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    int maxWidth = 950;

    // initialize the compDB Map 
    compDB = new QMap<QString, QMap<QString, QString>* >;

    // initialize component property containers
    compConfig = new QMap<QString, QVector<QMap<QString, QString>* >* >;

    verticalLayout = new QVBoxLayout();    

    // title
    QHBoxLayout *titleLayout = new QHBoxLayout();    

    SectionTitle *title=new SectionTitle();
    title->setText(tr("Define the Performance Model"));
    title->setMinimumWidth(250);

    titleLayout->addWidget(title);    
    titleLayout->addStretch();

    // component selection ----------------------------------------------------
    QGroupBox *compListGroupBox = new QGroupBox("Component Ensemble");
    compListGroupBox->setMaximumWidth(maxWidth);
    //QFormLayout *compListFormLayout = new QFormLayout(compListGroupBox);

    QVBoxLayout *loCEns = new QVBoxLayout(compListGroupBox);

    // data folder
    QHBoxLayout *customFolderLayout = new QHBoxLayout();

    QLabel *lblChooseFragility = new QLabel();
    lblChooseFragility->setText("Damage and Loss Data Folder:");
    customFolderLayout->addWidget(lblChooseFragility);

    fragilityDataBasePath = new QLineEdit;
    fragilityDataBasePath->setToolTip(tr("Location of the folder with damage and loss data files.\n"
                                       "If empty, the default files are used that correspond to \n"
                                       "FEMA P58 Second Edition."));
    customFolderLayout->addWidget(fragilityDataBasePath, 1);

    QPushButton *btnChooseFragility = new QPushButton();
    btnChooseFragility->setMinimumWidth(70);
    btnChooseFragility->setMaximumWidth(70);
    btnChooseFragility->setText(tr("Choose"));
    connect(btnChooseFragility, SIGNAL(clicked()),this,SLOT(chooseFragilityDataBase()));
    connect(fragilityDataBasePath, SIGNAL(textChanged(QString)),this,SLOT(updateAvailableComponents()));
    customFolderLayout->addWidget(btnChooseFragility);

    QPushButton *btnExportDataBase = new QPushButton();
    btnExportDataBase->setMinimumWidth(150);
    btnExportDataBase->setMaximumWidth(150);
    btnExportDataBase->setText(tr("Export Default DB"));
    connect(btnExportDataBase, SIGNAL(clicked()),this,SLOT(exportFragilityDataBase()));
    customFolderLayout->addWidget(btnExportDataBase);

    loCEns->addLayout(customFolderLayout);
    //compListFormLayout->addRow(tr("DL Data Folder: "), customFolderLayout);

    QSpacerItem *spacerGroups1 = new QSpacerItem(10,10);
    loCEns->addItem(spacerGroups1);

    // available components
    QHBoxLayout *availableCLayout = new QHBoxLayout();

    QLabel *lblAvailableComp = new QLabel();
    lblAvailableComp->setText("Available Components:");

    availableCLayout->addWidget(lblAvailableComp);

    availableCompCombo = new QComboBox();
    availableCBModel = new QStringListModel();
    availableCompCombo->setModel(availableCBModel);
    availableCompCombo->setToolTip(tr("List of available components. \n"
                                      "Click on the buttons to add the component to the ensemble."));

    this-> updateAvailableComponents();

    availableCLayout->addWidget(availableCompCombo, 1);

    QPushButton *addComponent = new QPushButton();
    addComponent->setMinimumWidth(150);
    addComponent->setMaximumWidth(150);
    addComponent->setText(tr("Add Selected"));
    connect(addComponent,SIGNAL(clicked()),this,SLOT(addOneComponent()));
    availableCLayout->addWidget(addComponent);

    QPushButton *addAllComponents = new QPushButton();
    addAllComponents->setMinimumWidth(150);
    addAllComponents->setMaximumWidth(150);
    addAllComponents->setText(tr("Add All"));    
    connect(addAllComponents,SIGNAL(clicked()),this,SLOT(addAllComponents()));
    availableCLayout->addWidget(addAllComponents);

    QPushButton* loadConfig = new QPushButton();
    loadConfig->setMinimumWidth(300);
    loadConfig->setMaximumWidth(300);
    loadConfig->setText(tr("Load Performance Model from CSV"));
    loadConfig->setToolTip(tr("Load Performance Model from a CSV file."));
    connect(loadConfig,SIGNAL(clicked()),this,SLOT(onLoadConfigClicked()));
    availableCLayout->addWidget(loadConfig);

    loCEns->addLayout(availableCLayout);
    //compListFormLayout->addRow(tr("Available Components: "),availableCLayout);

    QSpacerItem *spacerGroups2 = new QSpacerItem(10,10);
    loCEns->addItem(spacerGroups2);

    // selected components
    QHBoxLayout *selectedCLayout = new QHBoxLayout();

    QLabel *lblSelectedComp = new QLabel();
    lblSelectedComp->setText("Selected Components:");
    selectedCLayout->addWidget(lblSelectedComp);

    selectedCompCombo = new QComboBox();
    selectedCBModel = new QStringListModel();
    selectedCompCombo->setModel(selectedCBModel);
    selectedCompCombo->setToolTip(tr("List of selected components. \n"
                                     "Click on the buttons to remove the component from the ensemble."));
    connect(selectedCompCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(showSelectedComponent()));
    selectedCLayout->addWidget(selectedCompCombo, 1);

    QPushButton *removeComponent = new QPushButton();
    removeComponent->setMinimumWidth(150);
    removeComponent->setMaximumWidth(150);
    removeComponent->setText(tr("Remove Selected"));
    connect(removeComponent,SIGNAL(clicked()),this,SLOT(removeOneComponent()));
    selectedCLayout->addWidget(removeComponent);

    QPushButton *removeAllComponents = new QPushButton();
    removeAllComponents->setMinimumWidth(150);
    removeAllComponents->setMaximumWidth(150);
    removeAllComponents->setText(tr("Remove All"));
    connect(removeAllComponents,SIGNAL(clicked()),this, SLOT(removeAllComponents()));
    selectedCLayout->addWidget(removeAllComponents);

    QPushButton* saveConfig = new QPushButton();
    saveConfig->setMinimumWidth(300);
    saveConfig->setMaximumWidth(300);
    saveConfig->setText(tr("Save Performance Model to CSV"));
    saveConfig->setToolTip(tr("Save Performance Model to a CSV file."));
    connect(saveConfig,SIGNAL(clicked()),this,SLOT(onSaveConfigClicked()));
    selectedCLayout->addWidget(saveConfig);

    loCEns->addLayout(selectedCLayout);
    //compListFormLayout->addRow(tr("Selected Components: "), selectedCLayout);

    // component details
    QGroupBox *compDetailsGroupBox = new QGroupBox("Component Details");
    compDetailsGroupBox->setMaximumWidth(maxWidth);

    QVBoxLayout *loCDetails = new QVBoxLayout(compDetailsGroupBox);
    //QFormLayout *compDetailsFormLayout = new QFormLayout(compDetailsGroupBox);

    // name
    QHBoxLayout *loCDName = new QHBoxLayout();

    int CD_lbl_width = 110;

    QLabel *lblCompName = new QLabel();
    lblCompName->setText("Name:");
    lblCompName->setMaximumWidth(CD_lbl_width);
    lblCompName->setMinimumWidth(CD_lbl_width);
    lblCompName->setAlignment(Qt::AlignTop);
    loCDName->addWidget(lblCompName);

    compName = new QLabel();
    compName->setWordWrap(true);
    compName->setText("");
    loCDName->addWidget(compName, 1);

    //loCDName->addStretch();
    loCDetails->addLayout(loCDName);
    //compDetailsFormLayout->addRow(tr("Name: "), compName);

    // description
    QHBoxLayout *loCDDesc = new QHBoxLayout();

    QLabel *lblCompDesc = new QLabel();
    lblCompDesc->setText("Description:");
    lblCompDesc->setMaximumWidth(CD_lbl_width);
    lblCompDesc->setMinimumWidth(CD_lbl_width);
    lblCompDesc->setAlignment(Qt::AlignTop);
    loCDDesc->addWidget(lblCompDesc);

    compDescription = new QLabel();
    compDescription->setWordWrap(true);
    compDescription->setText("");
    loCDDesc->addWidget(compDescription, 1);

    //loCDDesc->addStretch();
    loCDetails->addLayout(loCDDesc);
    //compDetailsFormLayout->addRow(tr("Description: "), compDescription);

    // EDP type
    QHBoxLayout *loCDEDP = new QHBoxLayout();

    QLabel *lblEDP_Type = new QLabel();
    lblEDP_Type->setText("EDP type:");
    lblEDP_Type->setMaximumWidth(CD_lbl_width);
    lblEDP_Type->setMinimumWidth(CD_lbl_width);
    loCDEDP->addWidget(lblEDP_Type);

    compEDP = new QLabel();
    compEDP->setWordWrap(true);
    compEDP->setText("");
    loCDEDP->addWidget(compEDP, 1);

    //loCDEDP->addStretch();
    loCDetails->addLayout(loCDEDP);
    //compDetailsFormLayout->addRow(tr("EDP type: "), compEDP);

    // Standard unit
    QHBoxLayout *loCDUnit = new QHBoxLayout();

    QLabel *lblStdUnit = new QLabel();
    lblStdUnit->setText("Default unit:");
    lblStdUnit->setMaximumWidth(CD_lbl_width);
    lblStdUnit->setMinimumWidth(CD_lbl_width);
    loCDUnit->addWidget(lblStdUnit);

    compUnit = new QLabel();
    compUnit->setWordWrap(true);
    compUnit->setText("");
    loCDUnit->addWidget(compUnit, 1);

    //loCDUnit->addStretch();
    loCDetails->addLayout(loCDUnit);

    // Additional info
    QHBoxLayout *loCDInfo = new QHBoxLayout();

    QLabel *lblInfo = new QLabel();
    lblInfo->setText("Additional info:");
    lblInfo->setMaximumWidth(CD_lbl_width);
    lblInfo->setMinimumWidth(CD_lbl_width);
    loCDInfo->addWidget(lblInfo);

    compInfo = new QLabel();
    compInfo->setWordWrap(true);
    compInfo->setText("");
    loCDInfo->addWidget(compInfo, 1);

    loCDetails->addLayout(loCDInfo);

    // Quantity title & buttons
    QHBoxLayout *loCQuantity_title = new QHBoxLayout();

    QPushButton *addCGroup = new QPushButton();
    addCGroup->setMinimumWidth(220);
    addCGroup->setMaximumWidth(220);
    addCGroup->setText(tr("Add Component Group"));
    loCQuantity_title->addWidget(addCGroup);
    connect(addCGroup, SIGNAL(pressed()), this, SLOT(addComponentGroup()));

    QSpacerItem *locQNT_title_spacer = new QSpacerItem(10,10);
    loCQuantity_title->addItem(locQNT_title_spacer);

    QPushButton *removeCGroup = new QPushButton();
    removeCGroup->setMinimumWidth(220);
    removeCGroup->setMaximumWidth(220);
    removeCGroup->setText(tr("Remove Component Group"));
    loCQuantity_title->addWidget(removeCGroup);
    connect(removeCGroup, SIGNAL(pressed()), this, SLOT(removeComponentGroup()));

    loCQuantity_title->addStretch();

    loCDetails->addLayout(loCQuantity_title);

    // Component Group header
    QHBoxLayout *loCGroup_header = new QHBoxLayout();

    QLabel *lblQNT_loc = new QLabel();
    lblQNT_loc->setText("location(s)");
    //lblQNT_loc->setMaximumWidth(100);
    //lblQNT_loc->setMinimumWidth(100);
    loCGroup_header->addWidget(lblQNT_loc, 1);

    QLabel *lblQNT_dir = new QLabel();
    lblQNT_dir->setText("directions(s)");
    //lblQNT_dir->setMaximumWidth(100);
    //lblQNT_dir->setMinimumWidth(100);
    loCGroup_header->addWidget(lblQNT_dir, 1);

    QLabel *lblQNT_mu = new QLabel();
    lblQNT_mu->setText("median quantity");
    //lblQNT_mu->setMaximumWidth(100);
    //lblQNT_mu->setMinimumWidth(100);
    loCGroup_header->addWidget(lblQNT_mu, 1);

    QLabel *lblQNT_unit = new QLabel();
    lblQNT_unit->setText("unit");
    lblQNT_unit->setMaximumWidth(60);
    lblQNT_unit->setMinimumWidth(60);
    loCGroup_header->addWidget(lblQNT_unit);

    QLabel *lblQNT_dist = new QLabel();
    lblQNT_dist->setText("distribution");
    lblQNT_dist->setMaximumWidth(120);
    lblQNT_dist->setMinimumWidth(120);
    loCGroup_header->addWidget(lblQNT_dist);

    QLabel *lblQNT_cov = new QLabel();
    lblQNT_cov->setText("cov");
    lblQNT_cov->setMaximumWidth(90);
    lblQNT_cov->setMinimumWidth(90);
    loCGroup_header->addWidget(lblQNT_cov);
    loCGroup_header->addStretch();
    loCGroup_header->setSpacing(10);
    loCGroup_header->setMargin(0);

    loCDetails->addLayout(loCGroup_header);

    // Component Group list
    QScrollArea *saQuantityList = new QScrollArea;
    saQuantityList->setWidgetResizable(true);
    saQuantityList->setLineWidth(0);
    saQuantityList->setFrameShape(QFrame::NoFrame);

    QWidget *CGWidget = new QWidget;
    loCGList = new QVBoxLayout();
    loCGList->addStretch();
    loCGList->setMargin(0);
    CGWidget->setLayout(loCGList);
    saQuantityList->setWidget(CGWidget);

    loCDetails->addWidget(saQuantityList, 1);

    verticalLayout->addLayout(titleLayout);    
    verticalLayout->addWidget(compListGroupBox);
    verticalLayout->addWidget(compDetailsGroupBox, 1);
    //verticalLayout->addStretch(1);
    verticalLayout->setSpacing(10);
    //verticalLayout->setMargin(0);

    this->setLayout(verticalLayout);
}

void
P58ComponentContainer::clearCompGroupWidget(){

    int CGcount = vComponentGroups.size();
    for (int i = CGcount-1; i >= 0; i--) {
      ComponentGroup *theComponentGroup = vComponentGroups.at(i);
      //remove the widget from the UI
      theComponentGroup->close();
      loCGList->removeWidget(theComponentGroup);
      //remove the CG from the UI vector
      vComponentGroups.remove(i);
      //delete the CG UI object
      theComponentGroup->setParent(nullptr);
      delete theComponentGroup;
    }
}

void
P58ComponentContainer::retrieveCompGroups(){

    if (compConfig->contains(selectedCompCombo->currentText())) {
        // get the vector of CG data from the compConfig dict
        QVector<QMap<QString, QString>* > *vCG_data =
                compConfig->value(selectedCompCombo->currentText(),
                                  nullptr);
                                  //new QVector<QMap<QString, QString>* >);

        if (vCG_data != nullptr) {
            // create the CG UI elements for the existing data
            for (int i=0; i<vCG_data->count(); i++){
                addComponentGroup(vCG_data->at(i));
            }
        }
    }
}

void
P58ComponentContainer::showSelectedComponent(){

    if (selectedCompCombo->count() > 0) {

        if (selectedCompCombo->currentText() != "") {

            if (dbType == "JSON") {

                QString compFileName = selectedCompCombo->currentText() + ".json";
                QDir fragDir(this->getFragilityDataBase());
                QFile compFile(fragDir.absoluteFilePath(compFileName));
                compFile.open(QFile::ReadOnly | QFile::Text);

                QString val;
                val = compFile.readAll();
                QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
                QJsonObject compData = doc.object();
                compFile.close();

                compName->setText(compData["Name"].toString());

                QJsonObject compGI = compData["GeneralInformation"].toObject();
                compDescription->setText(compGI["Description"].toString());

                QJsonObject compEDPVar = compData["EDP"].toObject();
                compEDP->setText(compEDPVar["Type"].toString());

                QJsonArray compQData = compData["QuantityUnit"].toArray();
                //compUnit->setText(QString("%1").arg(compQData[0].toDouble()));
                compUnit->setText(QString("%1").arg(compQData[0].toDouble())+QString(" ")+
                                  compQData[1].toString());

                QString infoString = "";
                if (compData["Directional"] == true) {
                    infoString += "Directional, ";
                } else {
                    infoString += "Non-directional, ";
                }
                if (compData["Correlated"] == true) {
                    infoString += "identical behavior among Component Groups.";
                } else {
                    infoString += "independent behavior among Component Groups.";
                }

                if (compGI["Incomplete"] == true) infoString += "  INCOMPLETE DATA!";
                compInfo->setText(infoString);
            }
            else if (dbType == "HDF5") {

                QString compID = selectedCompCombo->currentText();
                QMap<QString, QString>* C_info = compDB->value(compID);

                compName->setText(C_info->value("Name"));
                compDescription->setText(C_info->value("Description"));
                compEDP->setText(C_info->value("EDPType"));
                compUnit->setText(C_info->value("QuantityUnit"));

                QString infoString = "";
                if (C_info->value("Directional") == "1") {
                    infoString += "Directional, ";
                } else {
                    infoString += "Non-directional, ";
                }
                if (C_info->value("Correlated") == "1") {
                    infoString += "identical behavior among Component Groups.";
                } else {
                    infoString += "independent behavior among Component Groups.";
                }

                if (C_info->value("Incomplete") == "1") infoString += "  INCOMPLETE DATA!";
                compInfo->setText(infoString);
            }

            this->clearCompGroupWidget();
            this->retrieveCompGroups();
        }
    } else {
        compName->setText("");
        compDescription->setText("");
        compEDP->setText("");
        compUnit->setText("");
        compInfo->setText("");
        this->clearCompGroupWidget();
    }
}

void 
P58ComponentContainer::deleteCompDB(){

    qDeleteAll(compDB->begin(), compDB->end());
    compDB->clear();
    delete compDB;
}

void 
P58ComponentContainer::deleteCompConfig(){

    // get an iterator for the main map
    QMap<QString, QVector<QMap<QString, QString>* >* >::iterator m;
    for (m=compConfig->begin(); m!=compConfig->end(); ++m){

        // for each vector in the map, get an iterator
        QVector<QMap<QString, QString>* > *vi = *m;
        QVector<QMap<QString, QString>* >::iterator i;
        for(i=vi->begin(); i != vi->end(); ++i){

            // free each vector element
            delete *i;
        }

        // then remove the vector elements
        vi->clear();

        // and free the vector itself
        delete *m;
    }

    // then remove the vectors from the map
    compConfig->clear();

    // and free the map
    delete compConfig;
}

int
P58ComponentContainer::updateAvailableComponents(){

    availableCompCombo->clear();

    QString fragilityDataBase = this->getFragilityDataBase();

    QStringList compIDs;

    if (fragilityDataBase.endsWith("hdf")) {

        qDebug() << "HDF file recognized";
        dbType = "HDF5";

        // we are dealing with an HDF5 file...

        HDF5Data *DLDB = new HDF5Data(fragilityDataBase);

        QVector<QString> qvComp;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard")), 
                          QString("index"), &qvComp);

        // when using an HDF5 file, all component info is loaded at once here
        QVector<QString> qvName;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard")), 
                          QString("Name"), &qvName);

        QVector<QString> qvDescription;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard/GeneralInformation")), 
                          QString("Description"), &qvDescription);

        QVector<QString> qvEDPType;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard/EDP")), 
                          QString("Type"), &qvEDPType);

        QVector<int> qvQuantityCount;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard")), 
                          QString("QuantityUnit#0"), &qvQuantityCount);

        QVector<QString> qvQuantityUnit;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard")), 
                          QString("QuantityUnit#1"), &qvQuantityUnit);

        QVector<int> qvDirectional;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard")), 
                          QString("Directional"), &qvDirectional);

        QVector<int> qvCorrelated;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard")), 
                          QString("Correlated"), &qvCorrelated);

        QVector<int> qvIncomplete;
        DLDB->getDFEntry(DLDB->getMember(QString("/data_standard/GeneralInformation")), 
                          QString("Incomplete"), &qvIncomplete);

        //initialize the component map
        deleteCompDB();
        compDB = new QMap<QString, QMap<QString, QString>* >;

        for (int i =0; i < qvComp.size(); i++){

            QString compName = qvComp[i];
            compIDs << compName;
            
            // Create a new C_info dict and add it to the compDB dict

            QMap<QString, QString> *C_info = new QMap<QString, QString>;
            compDB->insert(compName, C_info);

            // Then fill it with the info from the DL DB

            C_info -> insert("Name",         qvName[i]);
            C_info -> insert("Description",  qvDescription[i]);
            C_info -> insert("EDPType",      qvEDPType[i]);
            C_info -> insert("QuantityUnit", 
                QString("%1").arg(qvQuantityCount[i])+QString(" ")+qvQuantityUnit[i]);
            C_info -> insert("Directional",  QString::number(qvDirectional[i]));
            C_info -> insert("Correlated",   QString::number(qvCorrelated[i]));
            C_info -> insert("Incomplete",   QString::number(qvIncomplete[i]));
        }

        delete DLDB;

    } else {

        dbType = "JSON";

        // we are dealing with json files...
        QDir directory(fragilityDataBase);

        compIDs = directory.entryList(QStringList() << "*.json" << "*.JSON", QDir::Files);

        for (int i=0; i<compIDs.length(); i++){
            compIDs[i] = compIDs[i].remove(compIDs[i].size()-5, 5);
        }
    }

    availableCompCombo->addItems(compIDs);

    return 0;
}

QString
P58ComponentContainer::getFragilityDataBase(){

    QString fragilityDataBase = fragilityDataBasePath->text();

    if (fragilityDataBase == "") {

        QString appDir = SimCenterPreferences::getInstance()->getAppDir();

        fragilityDataBase = appDir + 
        "/applications/performDL/pelicun/pelicunPBE/resources/FEMA_P58_2nd_ed.hdf";
        //"/applications/performDL/pelicun/pelicunPBE/resources/DL json/";
    }

    qDebug() << "updating frag folder with " << fragilityDataBase;

    return fragilityDataBase;
}

int
P58ComponentContainer::setFragilityDataBase(QString fragilityDataBase){
    fragilityDataBasePath->setText(fragilityDataBase);
    return 0;
}

void
P58ComponentContainer::chooseFragilityDataBase(void) {

    QString appDir = SimCenterPreferences::getInstance()->getAppDir();

    QString fragilityDataBase;
    fragilityDataBase = 
        QFileDialog::getExistingDirectory(this, 
                                          tr("Select Database Folder"), appDir);

    this->setFragilityDataBase(fragilityDataBase);

    this->updateAvailableComponents();
}

void
P58ComponentContainer::exportFragilityDataBase(void) {

    QString appDir = SimCenterPreferences::getInstance()->getAppDir();

    QString destinationFolder;
    destinationFolder = 
        QFileDialog::getExistingDirectory(this, tr("Select Destination Folder"),
                                          appDir);

    qDebug() << QString("Exporting default damage and loss database...");

    // run the export script    
    QDir scriptDir(appDir);
    scriptDir.cd("applications");
    scriptDir.cd("performDL");
    scriptDir.cd("pelicun");
    QString exportScript = scriptDir.absoluteFilePath("export_DB.py");
    scriptDir.cd("pelicunPBE");
    scriptDir.cd("resources");
    QString dbFile = scriptDir.absoluteFilePath("FEMA_P58_2nd_ed.hdf");

    QProcess *proc = new QProcess();
    QString python = QString("python");
    QSettings settingsPy("SimCenter", "Common"); //These names will need to be constants to be shared
    QVariant  pythonLocationVariant = settingsPy.value("pythonExePath");
    if (pythonLocationVariant.isValid()) {
      python = pythonLocationVariant.toString();
    }

#ifdef Q_OS_WIN
    python = QString("\"") + python + QString("\"");
    QStringList args{exportScript, "--DL_DB_path",dbFile,"--target_dir",destinationFolder};

    qDebug() << "PYTHON COMMAND: " << python << args;

    proc->execute(python, args);

#else
    // note the above not working under linux because basrc not being called so no env variables!!

    QString command = QString("source $HOME/.bash_profile; \"") + python + QString("\" \"") +
        exportScript + QString("\"--DL_DB_path\"") + dbFile + QString("\"--target_dir\"") +
        destinationFolder;

    qDebug() << "PYTHON COMMAND: " << command;
    proc->execute("bash", QStringList() << "-c" <<  command);

#endif

    proc->waitForStarted();

    qDebug() << QString("Successfully exported default damage and loss database...");
}

void
P58ComponentContainer::onLoadConfigClicked(void) {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load Configuration"), "",
                                                    tr("All Files (*)"));

    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly))
    {
        // clear the selectedCompCombo
        this->removeAllComponents();

        // remove the existing compConfig and initialize a new one
        deleteCompConfig();
        compConfig = new QMap<QString, QVector<QMap<QString, QString>* >* >;

        QTextStream stream(&file);

        int counter = 0;
        while (!stream.atEnd()) {
            counter ++;

            QString line = stream.readLine();
            QStringList line_list;

            // parse the line considering "" and , and \n
            bool in_commented_block = false;
            bool save_element = false;
            int c_0 = 0;
            for (int c=0; c<line.length(); c++) {
                if (line[c] == "\"") {
                    if (in_commented_block) {
                        save_element = true;
                        in_commented_block = false;
                    } else {
                        in_commented_block = true;
                        c_0 = c+1;
                    }
                } else if (line[c] == ",") {
                    if (c_0 == c){
                        c_0++;
                    } else if (in_commented_block == false) {
                        save_element = true;
                    }
                } else if (c == line.length()-1) {
                    save_element = true;
                    c++;
                }

                if (save_element) {
                    QString element = line.mid(c_0, c-c_0);
                    c_0 = c+1;
                    line_list.append(element);
                    save_element = false;
                }
            }

            QString compName = line_list[0];
            if (compName == "ID") {
                continue;
            }

            // first make sure that the comp ID is in the compConfig dict
            // and a corresponding CG_data vector is also stored there
            QVector<QMap<QString, QString>* > *vCG_data;
            if (compConfig->contains(compName)) {
                // get the vector of CG data from the compConfig dict
                vCG_data = compConfig->value(compName, nullptr);
            } else {
                vCG_data = new QVector<QMap<QString, QString>* >;
                compConfig->insert(compName, vCG_data);
            }

            // create a new CG_data dict and add it to the vector
            QMap<QString, QString> *CG_data = new QMap<QString, QString>;
            vCG_data->append(CG_data);

            if (line_list.count() >= 6) {
                // fill the CG_data dict with the info from the given row in the file
                CG_data -> insert("location",     line_list[1]);
                CG_data -> insert("direction",    line_list[2]);
                CG_data -> insert("median",       line_list[3]);
                CG_data -> insert("unit",         line_list[4]);
                CG_data -> insert("distribution", line_list[5]);
                if (line_list.count() >= 7) {
                    CG_data -> insert("cov",      line_list[6]);
                } else {
                    CG_data -> insert("cov",      "");
                }
            } else {
                qDebug() << "Error while parsing line " << counter << " in the config file";
            }
        }

        file.close();

        // add the component IDs to the selectedCompCombo
        for (auto compName: compConfig->keys()){

            if (availableCompCombo->findText(compName) != -1) {
                selectedCompCombo->addItem(compName);
            } else {
                qDebug() << "Component " << compName << "is not in the DL data folder!";
            }
        }

    }
}

void
P58ComponentContainer::onSaveConfigClicked(void) {

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Configuration"), "",
                                                    tr("All Files (*)"));

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);

        //create the header
        stream << "\"ID\"" << ",";
        stream << "\"Location(s)\"" << ",";
        stream << "\"Direction(s)\"" << ",";
        stream << "\"Median Quantity\"" << ",";
        stream << "\"Unit\"" << ",";
        stream << "\"Distribution\"" << ",";
        stream << "\"COV\"";
        stream << "\n";

        // save the component characteristics
        const QStringList compList = selectedCBModel->stringList();

        for (const auto& compName: compList)
        {
            // get the vector of CG data from the compConfig dict
            QVector<QMap<QString, QString>* > *vCG_data =
                    compConfig->value(compName, nullptr);

            if (vCG_data != nullptr) {
                for (int i=0; i<vCG_data->count(); i++){
                    QMap<QString, QString> *CG_data = vCG_data->at(i);

                    stream << "\"" << compName << "\",";
                    stream << "\"" << CG_data -> value("location", tr("")) << "\",";
                    stream << "\"" << CG_data -> value("direction", tr("")) << "\",";
                    stream << "\"" << CG_data -> value("median", tr("")) << "\",";
                    stream << "\"" << CG_data -> value("unit", tr("")) << "\",";
                    stream << "\"" << CG_data -> value("distribution", tr("")) << "\",";
                    stream << "\"" << CG_data -> value("cov", tr("")) << "\"";
                    stream << "\n";
                }
            }
        }
    }
}


P58ComponentContainer::~P58ComponentContainer()
{

}

void P58ComponentContainer::addOneComponent()
{
    this->addComponent();
}

void P58ComponentContainer::removeOneComponent()
{
    this->removeComponent();
}

void P58ComponentContainer::addComponent()
{
    if (selectedCompCombo->findText(availableCompCombo->currentText()) == -1) {        
        selectedCompCombo->addItem(availableCompCombo->currentText());
    }
}

void P58ComponentContainer::addAllComponents(void)
{
    const QStringList compList = availableCBModel->stringList();

    for (const auto& compName : compList)
    {
        availableCompCombo->setCurrentText(compName);
        this->addComponent();
    }
}

void P58ComponentContainer::removeComponent()
{   
    selectedCompCombo->removeItem(selectedCompCombo->currentIndex());
}

void P58ComponentContainer::removeAllComponents(void)
{
    selectedCompCombo->clear();
}

void P58ComponentContainer::addComponentGroup(QMap<QString, QString> *CG_data_in)
{   
    if (selectedCompCombo->count() > 0) {
       // add a new CG_data dict in the vector
       QMap<QString, QString> *CG_data;
       if (CG_data_in == nullptr) {
           // get the vector of CG data from the compConfig dict
           QVector<QMap<QString, QString>* > *vCG_data =
                   compConfig->value(selectedCompCombo->currentText(),
                                     nullptr);

           // create a new CG_data dict and add it to the vector
           CG_data = new QMap<QString, QString>;

           if (vCG_data != nullptr) {
                vCG_data->append(CG_data);
           } else {
               QVector<QMap<QString, QString>* > *new_vCG_data = 
                                          new QVector<QMap<QString, QString>* >;
               new_vCG_data->append(CG_data);                              
               
               // save the vector of CG data to the compConfig dict
               compConfig->insert(selectedCompCombo->currentText(),
                                  new_vCG_data);
           }
       } else {
           CG_data = CG_data_in;
       }

       // create the new UI object and assign CG_data to it
       ComponentGroup *theComponentGroup = new ComponentGroup(nullptr,
                                                              CG_data);
       // add the ComponentGroup to the vector of CGs
       vComponentGroups.append(theComponentGroup);
       // add the ComponentGroup to the UI
       loCGList->insertWidget(loCGList->count()-1, theComponentGroup);
    }
}


void P58ComponentContainer::removeComponentGroup(void)
{
    if (selectedCompCombo->count() > 0) {
        // get the vector of CG data from the compConfig dict
        QVector<QMap<QString, QString>* > *vCG_data =
                compConfig->value(selectedCompCombo->currentText(),
                                  nullptr);
                                  //new QVector<QMap<QString, QString>* >);

        if (vCG_data != nullptr) {
            // find the ones selected & remove them
            int CGcount = vComponentGroups.size();
            for (int i = CGcount-1; i >= 0; i--) {
              ComponentGroup *theComponentGroup = vComponentGroups.at(i);
              if (theComponentGroup->isSelectedForRemoval()) {
                  //remove the widget from the UI
                  theComponentGroup->close();
                  loCGList->removeWidget(theComponentGroup);
                  //remove the CG_data from the database
                  vCG_data->remove(i);
                  //remove the CG from the UI vector
                  vComponentGroups.remove(i);
                  //delete the CG_data object
                  theComponentGroup->delete_CG_data();
                  //delete the CG UI object
                  theComponentGroup->setParent(nullptr);
                  delete theComponentGroup;
              }
            }
        }
    }
}

bool
P58ComponentContainer::outputToJSON(QJsonObject &jsonObject)
{    
    bool result = true;

    // first, save the DL data folder
    QString pathString;
    pathString = fragilityDataBasePath->text();
    if (pathString != ""){
        jsonObject["ComponentDataFolder"] = pathString;
    }

    // then, for each component save the component information
    QJsonObject compData;
    if (jsonObject.contains("Components"))
        compData = jsonObject["Components"].toObject();

    const QStringList compList = selectedCBModel-> stringList();
    foreach (const QString compID, compList){

        // get the vector of CG data from the compConfig dict
        QVector<QMap<QString, QString>* > *vCG_data =
                compConfig->value(compID, nullptr);

        if (vCG_data != nullptr) {
            QJsonArray compArray;
            for (int i=0; i<vCG_data->count(); i++){
                QMap<QString, QString> *CG_data = vCG_data->at(i);
                QJsonObject CGObj;

                CGObj["location"] = CG_data -> value("location", tr(""));
                CGObj["direction"] = CG_data -> value("direction", tr(""));
                CGObj["median_quantity"] = CG_data -> value("median", tr(""));
                CGObj["unit"] = CG_data -> value("unit", tr(""));
                CGObj["distribution"] = CG_data -> value("distribution", tr(""));
                CGObj["cov"] = CG_data -> value("cov", tr(""));

                compArray.append(CGObj);
            }

            compData[compID] = compArray;
        } 
    }
    jsonObject["Components"]=compData;

    return result;
}

bool
P58ComponentContainer::inputFromJSON(QJsonObject &jsonObject)
{
    bool result = true;

    // first, load the DL data folder
    QString pathString;
    pathString = jsonObject["ComponentDataFolder"].toString();
    fragilityDataBasePath->setText(pathString);

    // clear the selectedCompCombo
    this->removeAllComponents();

    // remove the existing compConfig and initialize a new one
    deleteCompConfig();
    compConfig = new QMap<QString, QVector<QMap<QString, QString>* >* >;

    QJsonObject compData;
    if (jsonObject.contains("Components"))
      compData = jsonObject["Components"].toObject();

    foreach (const QString& compID, compData.keys()) {
        // first make sure that the comp ID is in the compConfig dict
        // and a corresponding CG_data vector is also stored there
        QVector<QMap<QString, QString>* > *vCG_data;
        if (compConfig->contains(compID)) {
          // get the vector of CG data from the compConfig dict
          vCG_data = compConfig->value(compID, nullptr);
        } else {
          vCG_data = new QVector<QMap<QString, QString>* >;
          compConfig->insert(compID, vCG_data);
        }

        QJsonArray CGDataList = compData[compID].toArray();
        foreach (const QJsonValue& compValue, CGDataList) {
            QJsonObject compInfo = compValue.toObject();

            // create a new CG_data dict and add it to the vector
            QMap<QString, QString> *CG_data = new QMap<QString, QString>;
            vCG_data->append(CG_data);

            CG_data -> insert("location",     compInfo["location"].toString());
            CG_data -> insert("direction",    compInfo["direction"].toString());
            CG_data -> insert("median",       compInfo["median_quantity"].toString());
            CG_data -> insert("unit",         compInfo["unit"].toString());
            CG_data -> insert("distribution", compInfo["distribution"].toString());
            if (compInfo.contains("cov"))
                CG_data -> insert("cov",          compInfo["cov"].toString());
            else
                CG_data -> insert("cov", tr(""));
        }
    }

    // add the component IDs to the selectedCompCombo
    for (auto compName: compConfig->keys()){
        if (availableCompCombo->findText(compName) != -1) {
            selectedCompCombo->addItem(compName);
        } else {
            qDebug() << "Component " << compName << "is not in the DL data folder!";
        }
    }

    return result;
}

bool 
P58ComponentContainer::copyFiles(QString &dirName) {
    return true;
}

void
P58ComponentContainer::errorMessage(QString message){
    emit sendErrorMessage(message);
}

