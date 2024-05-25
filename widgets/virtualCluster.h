/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef virtCluster_h
#define virtCluster_h

#include "parameter.h"
#include "scriptablewidget.h"
#include "widgets.h"
#include "xlineedit.h"

#include <QDialog>
#include <QSqlQuery>
#include <QWidget>

class GuiClientInterface;
class QAction;
class QCompleter;
class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QMenu;
class QPushButton;
class QSpacerItem;
class QSqlQueryModel;
class QVBoxLayout;
class VirtualClusterLineEdit;
class XCheckBox;
class XDataWidgetMapper;
class XTreeWidget;

#define ID              1
#define NUMBER          2
#define DESCRIPTION     3
#define ACTIVE          4

class XTUPLEWIDGETS_EXPORT VirtualList : public QDialog, public ScriptableWidget
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualList();
        VirtualList(QWidget*, Qt::WindowFlags = Qt::WindowFlags {});

    public slots:
        virtual void sClose();
        virtual void sFillList();
        virtual void sSearch(const QString&);
        virtual void sSelect();

    protected:
        virtual void init();
        virtual void showEvent(QShowEvent *);

        VirtualClusterLineEdit* _parent;
        QVBoxLayout* _dialogLyt;
        QLabel*      _searchLit;
        QLineEdit*   _search;
        QLabel*      _titleLit;
        QDialogButtonBox*  _buttonBox;
        QPushButton* _select;
        XTreeWidget* _listTab;
        int          _id;
};

class XTUPLEWIDGETS_EXPORT VirtualSearch : public QDialog, public ScriptableWidget
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualSearch(QWidget*, Qt::WindowFlags = Qt::WindowFlags {});

        Q_INVOKABLE virtual void setQuery(QSqlQuery query);
        Q_INVOKABLE virtual void setSearchText(const QString& text);

    public slots:
        virtual void sClose();
        virtual void sFillList();
        virtual void sSelect();

    protected:
        virtual void showEvent(QShowEvent *);

        VirtualClusterLineEdit* _parent;
        QVBoxLayout* _dialogLyt;
        int  _id;
        QLabel*      _searchLit;
        QLineEdit*   _search;
        XCheckBox*   _searchNumber;
        XCheckBox*   _searchName;
        XCheckBox*   _searchDescrip;
        QLabel*      _titleLit;
        QDialogButtonBox*  _buttonBox;
        QPushButton* _select;
        XTreeWidget* _listTab;
        QLayout*     dialogLyt;
        QLayout*     searchLyt;
        QLayout*     searchStrLyt;
        QGridLayout* selectorsLyt;
        QLayout*     tableLyt;
        QLayout*     buttonsLyt;
};

class XTUPLEWIDGETS_EXPORT VirtualInfo : public QDialog, public ScriptableWidget
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualInfo(QWidget*,  Qt::WindowFlags = Qt::WindowFlags {});

    public slots:
        virtual void sPopulate();

    protected:
        virtual void showEvent(QShowEvent *);

        VirtualClusterLineEdit* _parent;
        QLabel*         _titleLit;
        QLabel*         _numberLit;
        QLabel*         _number;
        QLabel*         _nameLit;
        QLabel*         _name;
        QLabel*         _descripLit;
        QLabel*         _descrip;
        QPushButton*    _close;

    private:
        int _id;
};

/*
    VirtualClusterLineEdit is an abstract class that encapsulates
    the basics of retrieving an ID given a NUMBER or a NUMBER given
    an ID.  This class and its subclasses are intended to be used
    by VirtualCluster objects, not directly by an application.

    Users of this class need to supply the names of the table, its
    id column, and its number column (numbers are user-readable
    strings such as customer abbreviations or purchase order numbers).

    Subclasses must supply implementations for the sInfo(), sList(),
    and sSearch() slots.  sInfo() is a distinct dialog that shows
    extended data from the record with table_id = id(). sList() and
    sSearch() are dialogs that let the user select a number/id pair
    value for the widget.
*/

class XTUPLEWIDGETS_EXPORT VirtualClusterLineEdit : public XLineEdit
{
    Q_OBJECT

    friend class VirtualCluster;
    friend class VirtualInfo;
    friend class VirtualList;
    friend class VirtualSearch;

    public:
        VirtualClusterLineEdit(QWidget    *parent,
                               const char *pTabName,
                               const char *pIdColumn,
                               const char *pNumberColumn,
                               const char *pNameColumn,
                               const char *pDescripColumn,
                               const char *pExtra,
                               const char *pName         = 0,
                               const char *pActiveColumn = 0);

       void setMenu(QMenu *menu);
       QMenu *menu() const { return _menu; }

       static GuiClientInterface *_guiClientInterface;

       Q_INVOKABLE        virtual bool canOpen();
       Q_INVOKABLE inline virtual QString uiName()         const { return _uiName; }
       Q_INVOKABLE inline virtual QString editPriv()       const { return _editPriv; }
       Q_INVOKABLE inline virtual QString viewPriv()       const { return _viewPriv; }

       Q_INVOKABLE inline virtual QSqlQueryModel* model() const { return _model;}

       Q_INVOKABLE inline virtual QString name()        const { return _name; }
       Q_INVOKABLE inline virtual QString description() const { return _description; }

    public slots:
        virtual void clear();
        virtual QString extraClause() const { return _extraClause; }
        virtual void sEllipses();
        virtual void sInfo();
        virtual void sList();
        virtual void sOpen();
        virtual void sCopy();
        virtual void sNew();
        virtual void sSearch();
        virtual void setId(const int);
        virtual void setId(const int, const QString&);
        virtual void setNumber(const QString&);
        virtual void setShowInactive(const bool);

        virtual void setUiName(const QString& name);
        virtual void setEditPriv(const QString& priv);
        virtual void setNewPriv(const QString& priv);
        virtual void setViewPriv(const QString& priv);

    protected slots:
        inline virtual void clearExtraClause() { _extraClause = ""; }

        virtual VirtualList*    listFactory();
        virtual VirtualSearch*  searchFactory();
        virtual VirtualInfo*    infoFactory();

        virtual void setStrikeOut(bool enable = false);
        virtual void sHandleCompleter();
        virtual void sHandleNullStr();
        virtual void sParse();
        virtual void sUpdateMenu();
        virtual QWidget* sOpenWindow(const QString& uiName, ParameterList &params);

        virtual void setTitles(const QString&, const QString& = 0);
        inline virtual void setExtraClause(const QString& pExt) { _extraClause = pExt; }
        virtual void setTableAndColumnNames(const char* pTabName,
                                            const char* pIdColumn,
                                            const char* pNumberColumn,
                                            const char* pNameColumn,
                                            const char* pDescripColumn,
                                            const char* pActiveColumn,
                                            const char* = 0,
                                            const char* = 0);

        void setStrict(bool);
        bool isStrict() const { return _strict; }

        virtual void completerActivated(const QModelIndex &);
        virtual void completerHighlighted(const QModelIndex &);

    signals:
        void newId(int);
        void parsed();
        void valid(bool);

    protected:
        virtual bool eventFilter(QObject *obj, QEvent* event);
        virtual void focusInEvent(QFocusEvent * event);
        virtual void resizeEvent(QResizeEvent *e);

        QAction* _infoAct;
        QAction* _openAct;
        QAction* _copyAct;
        QAction* _newAct;
        QCompleter* _completer;
        QLabel* _menuLabel;
        QMenu* _menu;
        QString _titleSingular;
        QString _titlePlural;
        QString _query;
        QString _activeClause;
        QString _idClause;
        QString _numClause;
        QString _extraClause;
        QString _name;
        QString _description;
        QString _idColName;
        QString _numColName;
        QString _nameColName;
        QString _descripColName;
        QString _activeColName;
        QString _uiName;
        QString _editPriv;
        QString _newPriv;
        QString _viewPriv;
        bool _hasDescription;
        bool _hasName;
        bool _hasActive;
        bool _strict;
        bool _showInactive;
        int _completerId;

        virtual void silentSetId(const int);

        QSqlQueryModel* _model;

    private:
        void positionMenuLabel();

        QString _cText;
};

/*

    VirtualCluster is a widget that contains a VirtualClusterLineEdit
    and surrounds it with a label and two buttons.  One button lets
    the user invoke VirtualClusterLineEdit's sInfo slot while another
    button invokes either sSearch() or sList(), depending on the
    user's preferences.

    VirtualCluster provides a copy of much of the VirtualClusterLineEdit
    API to hide from the caller the fact that it contains one of these
    objects.  It also lets the calling code set it as read-only.

*/

class XTUPLEWIDGETS_EXPORT VirtualCluster : public QWidget, public ScriptableWidget
{
    Q_OBJECT

    Q_PROPERTY(QString label          READ label          WRITE setLabel)
    Q_PROPERTY(bool    nameVisible    READ nameVisible    WRITE setNameVisible)
    Q_PROPERTY(bool    descriptionVisible    READ descriptionVisible    WRITE setDescriptionVisible)
    Q_PROPERTY(bool    readOnly       READ readOnly       WRITE setReadOnly)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)
    Q_PROPERTY(QString number         READ number         WRITE setNumber         DESIGNABLE false)
    Q_PROPERTY(QString defaultNumber  READ defaultNumber  WRITE setDefaultNumber  DESIGNABLE false)
    Q_PROPERTY(QString nullStr        READ nullStr        WRITE setNullStr)

    friend class VirtualClusterLineEdit;

    public:
        VirtualCluster(QWidget *pParent, const char *pName = 0);
        VirtualCluster(QWidget *pParent, VirtualClusterLineEdit *pNumberWidget = 0, const char *pName = 0);

        Q_INVOKABLE virtual int     id()             const;
                    virtual QString label()          const;
                    virtual QString number()         const;
        Q_INVOKABLE virtual QString description()    const;
        Q_INVOKABLE virtual bool    isValid()        const;
        Q_INVOKABLE virtual QString name()           const;
        Q_INVOKABLE virtual bool    isStrict()       const;
                    virtual bool    readOnly()       const;
                    virtual QString defaultNumber()  const;
                    virtual QString fieldName()      const;
        Q_INVOKABLE virtual QString extraClause()    const;

        virtual Qt::Orientation orientation();
        virtual void setOrientation(Qt::Orientation orientation);

        virtual bool   nameVisible() const;
        virtual void   setNameVisible(const bool p);

        virtual bool   descriptionVisible() const;
        virtual void   setDescriptionVisible(const bool p);

        virtual QString nullStr() const;
        virtual void setNullStr(const QString &text);

    public slots:
        // most of the heavy lifting is done by VirtualClusterLineEdit _number
        virtual void clearExtraClause();
        virtual void setDefaultNumber(const QString& p);
        virtual void setDescription(const QString& p);
        virtual void setExtraClause(const QString& p, const QString& = QString {});
        virtual void setFieldName(QString p);
        virtual void setId(const int p, const QString& = QString {});
        virtual void setName(int, const QString& p);
        virtual void setNumber(const int p);
        virtual void setNumber(QString p);

        virtual void clear();
        virtual void setDataWidgetMap(XDataWidgetMapper* m);
        virtual void setEnabled(const bool p);
        virtual void setLabel(const QString& p);
        virtual void setStrict(const bool b);
        virtual void setReadOnly(const bool b);
        virtual void setShowInactive(const bool);
        virtual void sRefresh();
        virtual void updateMapperData();

        virtual void sEllipses();
        virtual void sInfo();
        virtual void sList();
        virtual void sSearch();

    signals:
        void newId(int);
        void valid(bool);

    protected:
        virtual void addNumberWidget(VirtualClusterLineEdit* pNumberWidget);
        virtual void showEvent(QShowEvent* e);

        QGridLayout*            _grid;
        QLabel*                 _label;
        VirtualClusterLineEdit* _number;
        QLabel*                 _description;
        QLabel*                 _name;
        QSpacerItem*            _hspcr;
        bool                    _readOnly;
	QString                 _fieldName;
        QString                 _default;
        XDataWidgetMapper       *_mapper;
        Qt::Orientation         _orientation;

    private:
        virtual void init();
};

void setupVirtualCluster(QScriptEngine *engine);

#endif
