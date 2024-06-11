/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTPURCHASEORDERSBYAGENT_H
#define PRINTPURCHASEORDERSBYAGENT_H

#include "xdialog.h"
#include "ui_printPurchaseOrdersByAgent.h"

class printPurchaseOrdersByAgent : public XDialog, public Ui::printPurchaseOrdersByAgent
{
    Q_OBJECT

  public:
    printPurchaseOrdersByAgent(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~printPurchaseOrdersByAgent();

  public slots:
    virtual void sPrint();

  signals:
            void finishedPrinting(int);

  protected slots:
    virtual void languageChange();

};

#endif // PRINTPURCHASEORDERSBYAGENT_H
