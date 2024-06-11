/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purgeCreditMemos.h"

#include <QVariant>
#include <QMessageBox>

purgeCreditMemos::purgeCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_purge, SIGNAL(clicked()), this, SLOT(sPurge()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
}

purgeCreditMemos::~purgeCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void purgeCreditMemos::languageChange()
{
  retranslateUi(this);
}

void purgeCreditMemos::sPurge()
{
  XSqlQuery purgePurge;
  if (!_cutOffDate->isValid())
  {
    QMessageBox::warning( this, tr("Enter Cutoff Date"),
                          tr("You must enter a valid cutoff date before purging Invoice Records.") );
    return;
  }

  if ( QMessageBox::warning( this, tr("Delete Sales Credit Records"),
                             tr( "You will not be able to re-print a Sales Credit if you delete it.\n"
                                 "Are you sure that you want to delete the selected Sales Credits?" ),
                             tr("Yes"), tr("No"), QString {}, 0, 1) == 0)
  {
    purgePurge.prepare("SELECT purgeCreditMemos(:cutOffDate) AS result;");
    purgePurge.bindValue(":cutOffDate", _cutOffDate->date());
    purgePurge.exec();

    accept();
  }
}

