/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __CURRENCIESDIALOG_H___
#define __CURRENCIESDIALOG_H___

class QWidget;
class QBoxLayout;

class currencies;

#include <QDialog>
#include <QLabel>

class currenciesDialog : public QDialog
{
  Q_OBJECT

  public:
    currenciesDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~currenciesDialog();
  
  protected:
    currencies  *_currencies;
    QBoxLayout  *_layout;
    QLabel      *_msg;
};

#endif

