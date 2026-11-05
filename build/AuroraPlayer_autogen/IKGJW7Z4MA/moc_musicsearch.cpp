/****************************************************************************
** Meta object code from reading C++ file 'musicsearch.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/core/downloads/musicsearch.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'musicsearch.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN11MusicSearchE_t {};
} // unnamed namespace

template <> constexpr inline auto MusicSearch::qt_create_metaobjectdata<qt_meta_tag_ZN11MusicSearchE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MusicSearch",
        "changed",
        "",
        "preferencesChanged",
        "playRequested",
        "QVariantMap",
        "candidate",
        "playLibraryRequested",
        "id",
        "search",
        "query",
        "discover",
        "cancel",
        "download",
        "key",
        "listen",
        "cancelAudition",
        "openInService",
        "libraryId",
        "results",
        "QVariantList",
        "sourceStates",
        "busy",
        "resolving",
        "status",
        "auditionKey",
        "enabledSources",
        "taste"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'changed'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'preferencesChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playRequested'
        QtMocHelpers::SignalData<void(const QVariantMap &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Signal 'playLibraryRequested'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Method 'search'
        QtMocHelpers::MethodData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Method 'discover'
        QtMocHelpers::MethodData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cancel'
        QtMocHelpers::MethodData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'download'
        QtMocHelpers::MethodData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Method 'listen'
        QtMocHelpers::MethodData<void(const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Method 'cancelAudition'
        QtMocHelpers::MethodData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openInService'
        QtMocHelpers::MethodData<void(const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Method 'libraryId'
        QtMocHelpers::MethodData<int(const QString &) const>(18, 2, QMC::AccessPublic, QMetaType::Int, {{
            { QMetaType::QString, 14 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'results'
        QtMocHelpers::PropertyData<QVariantList>(19, 0x80000000 | 20, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'sourceStates'
        QtMocHelpers::PropertyData<QVariantMap>(21, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(22, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'resolving'
        QtMocHelpers::PropertyData<bool>(23, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(24, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'query'
        QtMocHelpers::PropertyData<QString>(10, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'auditionKey'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'enabledSources'
        QtMocHelpers::PropertyData<QStringList>(26, QMetaType::QStringList, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'taste'
        QtMocHelpers::PropertyData<QString>(27, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MusicSearch, qt_meta_tag_ZN11MusicSearchE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MusicSearch::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MusicSearchE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MusicSearchE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11MusicSearchE_t>.metaTypes,
    nullptr
} };

void MusicSearch::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MusicSearch *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->changed(); break;
        case 1: _t->preferencesChanged(); break;
        case 2: _t->playRequested((*reinterpret_cast<std::add_pointer_t<QVariantMap>>(_a[1]))); break;
        case 3: _t->playLibraryRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->search((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->discover(); break;
        case 6: _t->cancel(); break;
        case 7: _t->download((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->listen((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->cancelAudition(); break;
        case 10: _t->openInService((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: { int _r = _t->libraryId((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MusicSearch::*)()>(_a, &MusicSearch::changed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (MusicSearch::*)()>(_a, &MusicSearch::preferencesChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (MusicSearch::*)(const QVariantMap & )>(_a, &MusicSearch::playRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (MusicSearch::*)(int )>(_a, &MusicSearch::playLibraryRequested, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QVariantList*>(_v) = _t->results(); break;
        case 1: *reinterpret_cast<QVariantMap*>(_v) = _t->sourceStates(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->resolving(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->query(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->auditionKey(); break;
        case 7: *reinterpret_cast<QStringList*>(_v) = _t->enabledSources(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->taste(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 7: _t->setEnabledSources(*reinterpret_cast<QStringList*>(_v)); break;
        case 8: _t->setTaste(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *MusicSearch::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MusicSearch::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MusicSearchE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MusicSearch::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void MusicSearch::changed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MusicSearch::preferencesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MusicSearch::playRequested(const QVariantMap & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void MusicSearch::playLibraryRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
