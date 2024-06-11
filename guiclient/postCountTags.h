/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTCOUNTTAGS_H
#define POSTCOUNTTAGS_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_postCountTags.h"

class postCountTags : public XDialog, public Ui::postCountTags
{
    Q_OBJECT

public:
    postCountTags(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~postCountTags();

    Q_INVOKABLE virtual bool setParams(ParameterList &params);

public slots:
    virtual void sPost();
    virtual void sParameterTypeChanged();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _codeGroup;

};

#endif // POSTCOUNTTAGS_H
