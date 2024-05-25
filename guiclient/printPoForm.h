/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTPOFORM_H
#define PRINTPOFORM_H

#include "printSinglecopyDocument.h"
#include "ui_printPoForm.h"

class printPoForm : public printSinglecopyDocument,
                    public Ui::printPoForm
{
    Q_OBJECT

  public:
    printPoForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~printPoForm();

    Q_INVOKABLE virtual ParameterList getParamsDocList();

  public slots:
    virtual void sFinishedWithAll();
    virtual void sHandleButtons();
    virtual void sHandleNewOrderId();
    virtual void sPopulate(XSqlQuery *docq);

  protected slots:
    virtual void languageChange();

};

#endif // PRINTPOFORM_H
