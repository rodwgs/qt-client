/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTCREDITMEMOS_H
#define PRINTCREDITMEMOS_H

#include "printMulticopyDocument.h"
#include "ui_printCreditMemos.h"

class printCreditMemos : public printMulticopyDocument,
                         public Ui::printCreditMemos
{
    Q_OBJECT

  public:
    printCreditMemos(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~printCreditMemos();

    Q_INVOKABLE virtual int distributeInventory(XSqlQuery *qry);

  protected slots:
    virtual void languageChange();
    virtual void sHandleFinishedWithAll();

};

#endif // PRINTCREDITMEMOS_H
