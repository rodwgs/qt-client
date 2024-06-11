/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#ifndef __XMAINWINDOW_H__
#define __XMAINWINDOW_H__

#include <QMainWindow>

#include <parameter.h>

#include "scriptablePrivate.h"

class QScriptEngine;
class XMainWindowPrivate;

class XMainWindow : public QMainWindow, protected ScriptablePrivate
{
  Q_OBJECT

  Q_PROPERTY(ParameterList params READ get WRITE set DESIGNABLE false);

  public:
    XMainWindow(QWidget * parent = 0, Qt::WindowFlags flags = Qt::WindowFlags {});
    XMainWindow(QWidget * parent, const char * name, Qt::WindowFlags flags = Qt::WindowFlags {});
    virtual ~XMainWindow();

    Q_INVOKABLE virtual ParameterList get() const;
    Q_INVOKABLE QAction *action() const;

    Q_INVOKABLE bool forceFloat() { return _forceFloat; }
    Q_INVOKABLE void setForceFloat(bool forceFloat) { _forceFloat = forceFloat; }

  public slots:
    virtual enum SetResponse set(const ParameterList &);

  protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void changeEvent(QEvent *);

  protected slots:
    virtual enum SetResponse postSet();

  private:
    friend class XMainWindowPrivate;
    friend class ScriptToolbox;
    XMainWindowPrivate *_private;
    friend QScriptEngine *engine(XMainWindow*);

    bool _forceFloat;

  private slots:
    void showMe(bool);
};

#endif // __XMAINWINDOW_H__

