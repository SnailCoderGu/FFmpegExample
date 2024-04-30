/****************************************************************************
** Meta object code from reading C++ file 'MyQtMainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../MyQtMainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MyQtMainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MyQtMainWindow_t {
    QByteArrayData data[9];
    char stringdata0[123];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyQtMainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyQtMainWindow_t qt_meta_stringdata_MyQtMainWindow = {
    {
QT_MOC_LITERAL(0, 0, 14), // "MyQtMainWindow"
QT_MOC_LITERAL(1, 15, 20), // "OnSliderValueChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 5), // "value"
QT_MOC_LITERAL(4, 43, 15), // "OnSliderPressed"
QT_MOC_LITERAL(5, 59, 21), // "OnSliderValueReleased"
QT_MOC_LITERAL(6, 81, 14), // "OpenFileDialog"
QT_MOC_LITERAL(7, 96, 16), // "UpdatePlayerInfo"
QT_MOC_LITERAL(8, 113, 9) // "ClickPlay"

    },
    "MyQtMainWindow\0OnSliderValueChanged\0"
    "\0value\0OnSliderPressed\0OnSliderValueReleased\0"
    "OpenFileDialog\0UpdatePlayerInfo\0"
    "ClickPlay"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyQtMainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x0a /* Public */,
       4,    0,   47,    2, 0x0a /* Public */,
       5,    0,   48,    2, 0x0a /* Public */,
       6,    0,   49,    2, 0x0a /* Public */,
       7,    0,   50,    2, 0x0a /* Public */,
       8,    0,   51,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MyQtMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MyQtMainWindow *_t = static_cast<MyQtMainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OnSliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->OnSliderPressed(); break;
        case 2: _t->OnSliderValueReleased(); break;
        case 3: _t->OpenFileDialog(); break;
        case 4: _t->UpdatePlayerInfo(); break;
        case 5: _t->ClickPlay(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MyQtMainWindow::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_MyQtMainWindow.data,
    qt_meta_data_MyQtMainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MyQtMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyQtMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MyQtMainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MyQtMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
