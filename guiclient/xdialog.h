/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#ifndef __XDIALOG_H__
#define __XDIALOG_H__

#include <QDialog>

#include <parameter.h>
#include <guiclient.h>
#include "scriptablePrivate.h"

class XDialogPrivate;

class XDialog : public QDialog, protected ScriptablePrivate
{
  Q_OBJECT

  public:
    XDialog(QWidget * parent = 0, Qt::WindowFlags flags = Qt::WindowFlags {});
    XDialog(QWidget * parent, const char * name, bool modal = false, Qt::WindowFlags flags = Qt::WindowFlags {});
    virtual ~XDialog();
    Q_INVOKABLE virtual ParameterList get() const;

  public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void setRememberPos(bool);
    virtual void setRememberSize(bool);
    int exec();

  protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void showEvent(QShowEvent * event);

  protected slots:
    virtual enum SetResponse postSet();
    virtual void saveSize();

  private:
    friend class XDialogPrivate;
    XDialogPrivate *_private;
};

#endif // __XDIALOG_H__

