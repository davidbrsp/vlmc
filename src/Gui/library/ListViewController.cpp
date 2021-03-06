/*****************************************************************************
 * ListViewController.h:
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Thomas Boquet <thomas.boquet@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "ListViewController.h"
#include "MediaCellView.h"
#include "Library/Library.h"
#include "StackViewController.h"

ListViewController::ListViewController( StackViewController* nav ) : m_nav( nav )
{
    m_title      = tr( "Media List" );
    m_scrollArea = new QScrollArea();
    m_layout     = new QVBoxLayout();
    m_container  = new QWidget( nav );

    m_layout->setAlignment( Qt::AlignTop );
    m_layout->setSpacing( 0 );
    m_layout->setContentsMargins( 0, 0, 0, 0 );
    m_container->setLayout( m_layout );
    m_scrollArea->setWidget( m_container );
    m_scrollArea->setWidgetResizable( true );
    m_scrollArea->setAcceptDrops( true );

}

ListViewController::~ListViewController()
{
    delete m_container;
    delete m_scrollArea;
}

// ViewController method implementation

const QString&
ListViewController::title() const
{
    return m_title;
}

QWidget*
ListViewController::view() const
{
    return m_scrollArea;
}

void
ListViewController::addCell( QWidget* cell )
{
    m_layout->addWidget( cell );
}

void
ListViewController::removeCell( QWidget* cell )
{
    m_layout->removeWidget( cell );
    delete cell;
}
