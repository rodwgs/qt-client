/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "explodeWo.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QButtonGroup>

#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

explodeWo::explodeWo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QButtonGroup* _levelGroupInt = new QButtonGroup(this);
  _levelGroupInt->addButton(_singleLevel);
  _levelGroupInt->addButton(_multipleLevel);

  connect(_explode, SIGNAL(clicked()), this, SLOT(sExplode()));

  _captive = false;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _multipleLevel->setChecked(_metrics->value("WOExplosionLevel") == "M");
  _wo->setType(cWoOpen);
}

explodeWo::~explodeWo()
{
  // no need to delete child widgets, Qt does it all for us
}

void explodeWo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse explodeWo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setEnabled(false);
  }

  return NoError;
}

void explodeWo::sExplode()
{
  XSqlQuery explodeExplode;
  if (_wo->isValid())
  {
    explodeExplode.prepare("SELECT explodeWo(:wo_id, :explodeChildren) AS result;");
    explodeExplode.bindValue(":wo_id", _wo->id());
    explodeExplode.bindValue(":explodeChildren", QVariant(_multipleLevel->isChecked()));
    explodeExplode.exec();
    if (explodeExplode.first())
    {
      int result = explodeExplode.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Exploding Work Order"),
                               storedProcErrorLookup("explodeWo", result)
                               .arg(_wo->id() ? _wo->woNumber() : ""),
                               __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Exploding Work Order"),
                                  explodeExplode, __FILE__, __LINE__))
    {
      return;
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), true);

    if (_captive)
      accept();
    else
    {
      _wo->setId(-1);
      _close->setText(tr("&Close"));
      _wo->setFocus();
    }
  }
}
