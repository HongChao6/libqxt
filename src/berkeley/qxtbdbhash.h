/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtBerkeley module of the Qt eXTension library
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of th Common Public License, version 1.0, as published by
 ** IBM.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL along with this file.
 ** See the LICENSE file and the cpl1.0.txt file included with the source
 ** distribution for more information. If you did not receive a copy of the
 ** license, contact the Qxt Foundation.
 **
 ** <http://libqxt.sourceforge.net>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#ifndef QxtBdbHash_H_kpasd
#define QxtBdbHash_H_kpasd

#include "qxtbdb.h"
#include <QBuffer>
#include <QDataStream>
#include <QVariant>
#include <qxtsharedprivate.h>
#include <qxtglobal.h>



template<class KEY, class VAL>
class QxtBdbHashIterator;

template<class KEY, class VAL>
class QXT_BERKELEY_EXPORT QxtBdbHash
{
public:
    QxtBdbHash();
    QxtBdbHash(QString file);
    bool open(QString file);

    QxtBdbHashIterator<KEY,VAL> begin();
    QxtBdbHashIterator<KEY,VAL> end();
    QxtBdbHashIterator<KEY,VAL> find ( const KEY & key );

    void clear();
    bool contains ( const KEY & key ) const;
    bool remove ( const KEY & key );
    bool insert(KEY k, VAL v);
    const VAL value (  const KEY & key  ) const;
    const VAL operator[] ( const KEY & key ) const;

    bool flush();

private:
    int meta_id_key;
    int meta_id_val;
    QxtSharedPrivate<QxtBdb> qxt_d;
};


class QxtBdbHashIteratorPrivate
{
public:
    BerkeleyDB::DBC *dbc;
    void invalidate()
    {
        if(dbc)
            dbc->close(dbc);
        dbc=0;
    }
    ~QxtBdbHashIteratorPrivate()
    {
        invalidate();
    }
};

template<class KEY, class VAL>
class QxtBdbHashIterator
{
public:
    QxtBdbHashIterator();
    QxtBdbHashIterator(const QxtBdbHashIterator<KEY,VAL> & other);
    QxtBdbHashIterator & operator= ( const QxtBdbHashIterator<KEY,VAL> & other );

    bool isValid() const;

    operator KEY() const;

    KEY     key() const;
    VAL   value() const;

    QxtBdbHashIterator<KEY,VAL>    operator + ( int j ) const;
    QxtBdbHashIterator<KEY,VAL> &  operator ++ ();
    QxtBdbHashIterator<KEY,VAL>    operator ++ (int);
    QxtBdbHashIterator<KEY,VAL> &  operator += ( int j );

    QxtBdbHashIterator<KEY,VAL>    operator - ( int j ) const;
    QxtBdbHashIterator<KEY,VAL> &  operator -- ();
    QxtBdbHashIterator<KEY,VAL>    operator -- (int);
    QxtBdbHashIterator<KEY,VAL> &  operator -= ( int j );

    bool operator== ( const QxtBdbHashIterator<KEY,VAL> & other ) const;
    bool operator!= ( const QxtBdbHashIterator<KEY,VAL> & other ) const;

    QxtBdbHashIterator<KEY,VAL> erase ();


private:
    friend class QxtBdbHash<KEY,VAL>;
    QxtBdbHashIterator(BerkeleyDB::DBC*);
    QxtSharedPrivate<QxtBdbHashIteratorPrivate> qxt_d;

    int meta_id_key;
    int meta_id_val;

};

















template<class KEY, class VAL>
QxtBdbHash<KEY,VAL>::QxtBdbHash()
{
    qxt_d=new QxtBdb();

    ///this will fail to compile if KEY or VALUE are not declared metatypes. Good.
    meta_id_key = qMetaTypeId<KEY>(); 
    meta_id_val = qMetaTypeId<VAL>(); 
}


template<class KEY, class VAL>
QxtBdbHash<KEY,VAL>::QxtBdbHash(QString file)
{
    qxt_d=new QxtBdb();
    open(file);

    ///this will fail to compile if KEY or VALUE are not declared metatypes. Good.
    meta_id_key = qMetaTypeId<KEY>(); 
    meta_id_val = qMetaTypeId<VAL>(); 
}

template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::open(QString file)
{
    return qxt_d().open(file,QxtBdb::CreateDatabase);
}




template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> QxtBdbHash<KEY,VAL>::begin()
{
    BerkeleyDB::DBC *cursor;
    qxt_d().db->cursor(qxt_d().db, NULL,&cursor, 0);

    BerkeleyDB::DBT key,value;
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));
    ///FIMXE: leak
    int ret=cursor->get(cursor,&key,&value,DB_FIRST);
    if(ret==0)
        return QxtBdbHashIterator<KEY,VAL>(cursor);
    else
        return QxtBdbHashIterator<KEY,VAL>();
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> QxtBdbHash<KEY,VAL>::end()
{
    BerkeleyDB::DBC *cursor;
    qxt_d().db->cursor(qxt_d().db, NULL,&cursor, 0);


    BerkeleyDB::DBT key,value;
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));
    cursor->get(cursor,key,value,DB_LAST);
    ///FIMXE: leak
    int ret=cursor->get(cursor,&key,&value,DB_LAST);

    if(ret==0)
        return QxtBdbHashIterator<KEY,VAL>(cursor);
    else
        return QxtBdbHashIterator<KEY,VAL>();
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> QxtBdbHash<KEY,VAL>::find ( const KEY & k )
{
    BerkeleyDB::DBC *cursor;
    qxt_d().db->cursor(qxt_d().db, NULL,&cursor, 0);

    BerkeleyDB::DBT key,value;
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));


    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();

    ///FIMXE: leak
    int ret=cursor->get(cursor,&key,&value,DB_SET);
    if(ret==0)
        return QxtBdbHashIterator<KEY,VAL>(cursor);
    else
        return QxtBdbHashIterator<KEY,VAL>();
}






template<class KEY, class VAL>
void QxtBdbHash<KEY,VAL>::clear()
{
    if(!qxt_d().isOpen)
        return;

    u_int32_t x;
    qxt_d().db->truncate(qxt_d().db,NULL, &x,0);

}


template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::contains ( const KEY & k ) const
{
    if(!qxt_d().isOpen)
        return false;

    BerkeleyDB::DBT key;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));

    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();

    return (qxt_d().db->exists(qxt_d().db, NULL, &key,0)==0);
}

template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::remove ( const KEY & k )
{
    if(!qxt_d().isOpen)
        return false;

    BerkeleyDB::DBT key;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));

    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();


    return (qxt_d().db->del(qxt_d().db, NULL, &key,0)==0);
}



template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::insert(KEY k, VAL v)
{
    if(!qxt_d().isOpen)
        return false;

    QByteArray d_key, d_value;

    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }
    {
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_val, &v));
        buffer.close();
    }

    BerkeleyDB::DBT key, value;
    /* Zero out the DBTs before using them. */
    memset(&key, 0,  sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));

    key.data = d_key.data();
    key.size = d_key.size();

    value.data = d_value.data();
    value.size = d_value.size();


    int ret = qxt_d().db->put(qxt_d().db, NULL, &key, &value, 0);
    return (ret==0);

}



template<class KEY, class VAL>
const VAL QxtBdbHash<KEY,VAL>::value ( const KEY & k  ) const
{

    if(!qxt_d().isOpen)
        return VAL() ;

    BerkeleyDB::DBT key, data;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&data, 0, sizeof(BerkeleyDB::DBT));


    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();

    data.data = 0;
    data.ulen = 0;
    data.flags =0;
    qxt_d().db->get(qxt_d().db, NULL, &key, &data, 0);


    QByteArray  d_value((const char*) data.data, data.size );


    /**
    FIXME: Db deletes the record when i close the database. So i cant even delete it now.
           in other words, it leaks...
    */
//     delete data.data;

    VAL v;
    {
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::ReadOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::load (s,meta_id_val, &v));
        buffer.close();
    }
    return v;
}

template<class KEY, class VAL>
const VAL QxtBdbHash<KEY,VAL>::operator[] (  const KEY & key  ) const
{
    return value(key);
}


template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::flush()
{
    return qxt_d().flush();
}
















template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL>::QxtBdbHashIterator()
{
    qxt_d=new QxtBdbHashIteratorPrivate;
    qxt_d().dbc=0;

    meta_id_key = qMetaTypeId<KEY>(); 
    meta_id_val = qMetaTypeId<VAL>(); 
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL>::QxtBdbHashIterator(const QxtBdbHashIterator<KEY,VAL> & other)
{
    qxt_d=other.qxt_d;

    meta_id_key = qMetaTypeId<KEY>();
    meta_id_val = qMetaTypeId<VAL>();

}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> & QxtBdbHashIterator<KEY,VAL>::operator= ( const QxtBdbHashIterator<KEY,VAL> & other )
{
    qxt_d=other.qxt_d;
    return *this;
}

template<class KEY, class VAL>
bool QxtBdbHashIterator<KEY,VAL>::isValid() const
{
    return (qxt_d().dbc!=0);
}


template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL>::operator KEY() const
{
    return key();
}


template<class KEY, class VAL>
KEY     QxtBdbHashIterator<KEY,VAL>::key() const
{
    if(!isValid())
        return KEY();

    BerkeleyDB::DBT value,key;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));

    qxt_d().dbc->get(qxt_d().dbc,&key,&value,DB_CURRENT);

    QByteArray  d_key((const char*) key.data, key.size );

    ///FIMXE: leak
//     value data.data;

    KEY k;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::ReadOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::load (s,meta_id_key, &k));
        buffer.close();
    }
    return k;
}

template<class KEY, class VAL>
VAL   QxtBdbHashIterator<KEY,VAL>::value() const
{
    if(!isValid())
        return VAL();

    BerkeleyDB::DBT value,key;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));

    qxt_d().dbc->get(qxt_d().dbc,&key,&value,DB_CURRENT);

    QByteArray  d_value((const char*) value.data, value.size );

    ///FIMXE: leak
//     value data.data;

    VAL v;
    {
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::ReadOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::load (s,meta_id_val, &v));
        buffer.close();
    }
    return v;
}


template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL>    QxtBdbHashIterator<KEY,VAL>::operator + ( int j ) const 
{
    QxtBdbHashIterator<KEY,VAL> d= *this;
    for(int i=0;i<j;i++)
        ++d;
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> &  QxtBdbHashIterator<KEY,VAL>::operator ++ () /*prefix*/
{
    if(!isValid())
        return *this;

    BerkeleyDB::DBT key,value;
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));
    ///FIMXE: leak
    if(qxt_d().dbc->get(qxt_d().dbc,&key,&value,DB_NEXT)!=0)
        qxt_d().invalidate();
    return *this;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL>   QxtBdbHashIterator<KEY,VAL>::operator ++ (int) /*postfix*/
{
    if(!isValid())
        return *this;

    QxtBdbHashIterator<KEY,VAL> d=*this;
    this->operator++();
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> &  QxtBdbHashIterator<KEY,VAL>::operator += ( int j )
{
    for(int i=0;i<j;i++)
        this->operator++();
    return *this;
}


template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL>    QxtBdbHashIterator<KEY,VAL>::operator - ( int j ) const
{
    QxtBdbHashIterator<KEY,VAL> d= *this;
    for(int i=0;i<j;i++)
        --d;
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> &  QxtBdbHashIterator<KEY,VAL>::operator -- ()  /*prefix*/
{
    if(!isValid())
        return *this;
    BerkeleyDB::DBT key,value;
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));
    ///FIMXE: leak
    if(qxt_d().dbc->get(qxt_d().dbc,&key,&value,DB_PREV)!=0)
        qxt_d().invalidate();
    return *this;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL>   QxtBdbHashIterator<KEY,VAL>::operator -- (int) /*postfix*/
{
    if(!isValid())
        return *this;

    QxtBdbHashIterator<KEY,VAL> d=*this;
    this->operator--();
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> &  QxtBdbHashIterator<KEY,VAL>::operator -= ( int j )
{
    for(int i=0;i<j;i++)
        this->operator--();
    return *this;
}


template<class KEY, class VAL>
bool QxtBdbHashIterator<KEY,VAL>::operator== ( const QxtBdbHashIterator<KEY,VAL> & other ) const
{
    return (qxt_d().dbc==other.qxt_d().dbc);
}

template<class KEY, class VAL>
bool QxtBdbHashIterator<KEY,VAL>::operator!= ( const QxtBdbHashIterator<KEY,VAL> & other ) const
{
    return (qxt_d().dbc!=other.qxt_d().dbc);
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL> QxtBdbHashIterator<KEY,VAL>::erase ()
{
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY,VAL>::QxtBdbHashIterator(BerkeleyDB::DBC* dbc)
{
    qxt_d=new QxtBdbHashIteratorPrivate;
    qxt_d().dbc=dbc;

    meta_id_key = qMetaTypeId<KEY>();
    meta_id_val = qMetaTypeId<VAL>();
}




















#endif
