/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SYSTEMMESSAGE_H
#define SYSTEMMESSAGE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_systemMessage.h"

#include <QHash>

class systemMessage : public XDialog, public Ui::systemMessage
{
    Q_OBJECT

  public:
    systemMessage(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~systemMessage();

    static systemMessage *windowForId(int id);

  public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sClose();
    virtual void sSave();
    virtual void populate();

  protected slots:
    virtual void languageChange();

  protected:
    static QHash<int, systemMessage*> open;

  private:
    int _mode;
    int _msguserid;

};

#endif // SYSTEMMESSAGE_H
