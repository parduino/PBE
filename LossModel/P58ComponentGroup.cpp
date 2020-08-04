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

// Written: fmckenna, mhgardner, adamzs

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QDebug>
#include <QJsonObject>

#include "P58ComponentGroup.h"

ComponentGroup::ComponentGroup(QWidget *parent, QMap<QString, QString> *CG_data_in)
  : SimCenterWidget(parent) {

    CG_data = CG_data_in;

    mainLayout = new QHBoxLayout();    

    cgLocation = new QLineEdit();
    cgLocation->setToolTip(tr("In buildings, locations are typically stories.\n"
                              "The ground floor is story 1. Providing 'all' \n"
                              "assigns the same setting to every story. You can use\n"
                              "a dash to specify a range of stories, such as '3-7'.\n"
                              "If a component is only assigned to the top story, or \n"
                              "the roof, you can use 'top' or 'roof'. You can also\n"
                              "combine these and use '3-roof', for example."));
    //cgLocation->setMaximumWidth(75);
    //cgLocation->setMinimumWidth(75);
    cgLocation->setText(CG_data->value("location", tr("")));
    this->storeCGLocation();
    connect(cgLocation, SIGNAL(editingFinished()), this,
            SLOT(storeCGLocation()));

    cgDirection = new QLineEdit();
    cgDirection->setToolTip(tr("Directions correspond to EDPs that are used to assess\n"
                               "the fragility of components. They shall match the \n"
                               "directions in the EDP results available from the \n"
                               "simulations. By default, 1 and 2 are horizontal X and Y\n"
                               "directions. If a component is non-directional (see the \n"
                               "additional info field above), it is sufficient to provide \n"
                               "1 as direction."));
    //cgDirection->setMaximumWidth(75);
    //cgDirection->setMinimumWidth(75);
    cgDirection->setText(CG_data->value("direction", tr("")));
    this->storeCGDirection();
    connect(cgDirection, SIGNAL(editingFinished()), this,
            SLOT(storeCGDirection()));

    cgMedianQNT = new QLineEdit();
    cgMedianQNT->setToolTip(tr("The list of quantities provided here specifies the number\n"
                               "of Component Groups in each Performance Group that is \n"
                               "created by this row.\n"
                               "If the fragility of component groups is assumed perfectly \n"
                               "correlated, there is no good reason to have more than \n"
                               "one Component Group (i.e. more than one quantity value)\n"
                               "defined for each Performance Group below."));
    //cgMedianQNT->setMaximumWidth(75);
    //cgMedianQNT->setMinimumWidth(75);
    cgMedianQNT->setText(CG_data->value("median", tr("")));
    this->storeCGMedian();
    connect(cgMedianQNT, SIGNAL(editingFinished()), this,
            SLOT(storeCGMedian()));

    cgUnit = new QLineEdit();
    cgUnit->setToolTip(tr("The unit used to specify component quantities. The default\n"
                          "unit from the fragility database is shown above. As long as\n"
                          "the specified unit belongs to the same class (i.e., length\n"
                          "area, etc.), any commonly used metric or US unit is acceptable.\n"
                          "Squared units are expressed by using a 2 after the name,\n"
                          "such as 'ft2' for square foot."));
    cgUnit->setMaximumWidth(60);
    cgUnit->setMinimumWidth(60);
    cgUnit->setText(CG_data->value("unit", tr("")));
    this->storeCGUnit();
    connect(cgUnit, SIGNAL(editingFinished()), this,
            SLOT(storeCGUnit()));

    cgDistribution = new QComboBox();
    cgDistribution->setToolTip(tr("Distribution family assigned to the component quantities.\n"
                                  "The N/A setting corresponds to known quantities with no uncertainty."));
    cgDistribution->addItem(tr("N/A"));
    cgDistribution->addItem(tr("normal"));
    cgDistribution->addItem(tr("lognormal"));
    cgDistribution->setMaximumWidth(120);
    cgDistribution->setMinimumWidth(120);
    cgDistribution->setCurrentText(CG_data->value("distribution", tr("N/A")));
    this->storeCGDistribution();
    connect(cgDistribution, SIGNAL(currentIndexChanged(int)), this,
            SLOT(storeCGDistribution()));

    cgCOV = new QLineEdit();
    cgCOV->setToolTip(tr("Coefficient of variation for the random distribution of \n"
                         "component quantities.\n"
                         "If the distribution is set to N/A, this field will not \n"
                         "be considered and it can be left blank."));
    cgCOV->setMaximumWidth(60);
    cgCOV->setMinimumWidth(60);
    cgCOV->setText(CG_data->value("cov", tr("")));
    this->storeCGCOV();
    connect(cgCOV, SIGNAL(editingFinished()), this,
            SLOT(storeCGCOV()));

    button = new QRadioButton();

    // Create the main layout inside which we place a spacer & main widget
    // implementation note: spacer added first to ensure it always lines up on left

    // Set up main layout    
    mainLayout->addWidget(cgLocation, 1);
    mainLayout->addWidget(cgDirection, 1);
    mainLayout->addWidget(cgMedianQNT, 1);
    mainLayout->addWidget(cgUnit);
    mainLayout->addWidget(cgDistribution);
    mainLayout->addWidget(cgCOV);
    mainLayout->addWidget(button);
    mainLayout->addStretch();    
    mainLayout->setSpacing(10);
    mainLayout->setMargin(0);

    this->setLayout(mainLayout);
}

ComponentGroup::~ComponentGroup()
{}

void
ComponentGroup::storeCGLocation(){
    CG_data -> insert("location", cgLocation->text());
}

void
ComponentGroup::storeCGDirection(){
    CG_data -> insert("direction", cgDirection->text());
}

void
ComponentGroup::storeCGMedian(){
    CG_data -> insert("median", cgMedianQNT->text());
}

void
ComponentGroup::storeCGUnit(){
    CG_data -> insert("unit", cgUnit->text());
}

void
ComponentGroup::storeCGDistribution(){
    CG_data -> insert("distribution", cgDistribution->currentText());
}

void
ComponentGroup::storeCGCOV(){
    CG_data -> insert("cov", cgCOV->text());
}

bool ComponentGroup::outputToJSON(QJsonObject &outputObject) {

    return true;
}

bool ComponentGroup::inputFromJSON(const QJsonObject & inputObject) {

    return true;
}

bool ComponentGroup::isSelectedForRemoval() const {
  return button->isChecked();
}


QString ComponentGroup::getComponentName() const {
     //return componentName->text();
    return "";
}

void ComponentGroup::delete_CG_data() {
    delete CG_data;
}
