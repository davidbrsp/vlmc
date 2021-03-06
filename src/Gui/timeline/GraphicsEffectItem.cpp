/*****************************************************************************
 * GraphicsEffectItem.cpp: Represent an effect in the timeline.
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#include "GraphicsEffectItem.h"

#include "Backend/IFilter.h"
#include "Backend/ITrack.h"
#include "Commands/Commands.h"
#include "EffectsEngine/EffectHelper.h"
#include "Workflow/TrackWorkflow.h"

#include "effectsengine/EffectInstanceWidget.h"

#include "AbstractGraphicsMediaItem.h"
#include "GraphicsTrack.h"
#include "Timeline.h"
#include "TracksScene.h"
#include "TracksView.h"

#include <QColorDialog>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

GraphicsEffectItem::GraphicsEffectItem( std::shared_ptr<EffectHelper> const& helper ) :
        m_effectHelper( helper ),
        m_container( nullptr )
{
    if ( m_effectHelper->isValid() == false )
        return;
    setWidth( m_effectHelper->length() );
    connect( m_effectHelper.get(), SIGNAL( lengthUpdated() ), this, SLOT( adjustLength() ) );
    setOpacity( 0.8 );
    m_itemColor = Qt::blue;
}

GraphicsEffectItem::~GraphicsEffectItem()
{
}

const QUuid&
GraphicsEffectItem::uuid() const
{
    return m_effectHelper->uuid();
}

int
GraphicsEffectItem::type() const
{
    return Type;
}

bool
GraphicsEffectItem::expandable() const
{
    return true;
}

bool
GraphicsEffectItem::moveable() const
{
    return true;
}

bool
GraphicsEffectItem::hasResizeBoundaries() const
{
    return ( m_container != nullptr );
}

Workflow::TrackType
GraphicsEffectItem::trackType() const
{
    return Workflow::VideoTrack;
}

qint64
GraphicsEffectItem::itemHeight() const
{
    return 10;
}

void
GraphicsEffectItem::paintRect( QPainter* painter, const QStyleOptionGraphicsItem* option )
{
    QRectF drawRect;
    bool drawRound;

    // Disable the matrix transformations
    painter->setWorldMatrixEnabled( false );

    painter->setRenderHint( QPainter::Antialiasing );

    // Get the transformations required to map the text on the viewport
    QTransform viewPortTransform = Timeline::instance()->tracksView()->viewportTransform();

    // Determine if a drawing optimization can be used
    if ( option->exposedRect.left() > AbstractGraphicsItem::RounderRectRadius &&
         option->exposedRect.right() < boundingRect().right() - AbstractGraphicsItem::RounderRectRadius )
    {
        // Optimized: paint only the exposed (horizontal) area
        drawRect = QRectF( option->exposedRect.left(),
                           boundingRect().top(),
                           option->exposedRect.right(),
                           boundingRect().bottom() );
        drawRound = false;
    }
    else
    {
        // Unoptimized: the item must be fully repaint
        drawRect = boundingRect();
        drawRound = true;
    }

    // Do the transformation
    QRectF mapped = deviceTransform( viewPortTransform ).mapRect( drawRect );

    QLinearGradient gradient( mapped.topLeft(), mapped.bottomLeft() );

    gradient.setColorAt( 0, m_itemColor );
    gradient.setColorAt( 1, m_itemColor.darker() );

    painter->setPen( Qt::NoPen );
    painter->setBrush( QBrush( gradient ) );

    if ( drawRound )
        painter->drawRoundedRect( mapped, AbstractGraphicsItem::RounderRectRadius,
                                  AbstractGraphicsItem::RounderRectRadius );
    else
        painter->drawRect( mapped );

    if ( itemColor().isValid() )
    {
        QRectF mediaColorRect = mapped.adjusted( 3, 2, -3, -2 );
        painter->setPen( QPen( itemColor(), 2 ) );
        painter->drawLine( mediaColorRect.topLeft(), mediaColorRect.topRight() );
    }

    if ( isSelected() )
    {
        setZValue( zSelected() );
        painter->setPen( Qt::yellow );
        painter->setBrush( Qt::NoBrush );
        mapped.adjust( 0, 0, 0, -1 );
        if ( drawRound )
            painter->drawRoundedRect( mapped, AbstractGraphicsItem::RounderRectRadius,
                                      AbstractGraphicsItem::RounderRectRadius);
        else
            painter->drawRect( mapped );
    }
    else
        setZValue( zNotSelected() );
}


void
GraphicsEffectItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* )
{
    painter->save();
    paintRect( painter, option );
    painter->restore();

    painter->save();
    paintTitle( painter, option );
    painter->restore();
}

void
GraphicsEffectItem::paintTitle( QPainter* painter, const QStyleOptionGraphicsItem* option )
{
    Q_UNUSED( option );

    // Disable the matrix transformations
    painter->setWorldMatrixEnabled( false );

    // Setup the font
    QFont f = painter->font();
    f.setPointSize( 8 );
    painter->setFont( f );

    // Initiate the font metrics calculation
    QFontMetrics fm( painter->font() );

    // Get the transformations required to map the text on the viewport
    QTransform viewPortTransform = Timeline::instance()->tracksView()->viewportTransform();
    // Do the transformation
    QRectF mapped = deviceTransform( viewPortTransform ).mapRect( boundingRect() );
    // Create an inner rect
    mapped.adjust( 2, 2, -2, -2 );

    painter->setPen( Qt::white );
    painter->drawText( mapped, Qt::AlignVCenter, fm.elidedText( QString::fromStdString( m_effectHelper->filterInfo()->name() ),
                                                                Qt::ElideRight, mapped.width() ) );
}

std::shared_ptr<EffectHelper>
GraphicsEffectItem::effectHelper()
{
    return m_effectHelper;
}


qint64
GraphicsEffectItem::begin() const
{
    return m_effectHelper->begin();
}

qint64
GraphicsEffectItem::end() const
{
    return m_effectHelper->end();
}

qint64
GraphicsEffectItem::maxBegin() const
{
    return 0;
}

qint64
GraphicsEffectItem::maxEnd() const
{
    Q_ASSERT( m_effectHelper->filter() );
    return m_effectHelper->filter()->length();
}

std::shared_ptr<Workflow::Helper>
GraphicsEffectItem::helper()
{
    return m_effectHelper;
}

void
GraphicsEffectItem::triggerMove( TrackWorkflow* target, qint64 startPos )
{
    Commands::trigger( new Commands::Effect::Move( m_effectHelper, m_effectHelper->filter()->input(),
                                                   target->input(), startPos ) );
}

void
GraphicsEffectItem::triggerMove( Backend::IInput* target, qint64 startPos )
{
    Commands::trigger( new Commands::Effect::Move( m_effectHelper, m_effectHelper->filter()->input(),
                                                   target, startPos ) );
}

void
GraphicsEffectItem::triggerResize( TrackWorkflow* target,
                                   qint64 newBegin, qint64 newEnd, qint64 )
{
    Q_UNUSED( target )
    if ( !m_effectHelper )
        return ;
    Commands::trigger( new Commands::Effect::Resize( m_effectHelper, newBegin, newEnd ) );
}

qint32
GraphicsEffectItem::zSelected() const
{
    return 300;
}

qint32
GraphicsEffectItem::zNotSelected() const
{
    return 200;
}

void
GraphicsEffectItem::containerMoved( qint64 pos )
{
    setStartPos( m_effectHelper->begin() + pos );
}

void
GraphicsEffectItem::setContainer( AbstractGraphicsMediaItem *item )
{
    if ( m_container != nullptr )
        m_container->disconnect( this );
    m_container = item;
    if ( item != nullptr )
    {
        connect( item, SIGNAL( moved( qint64 ) ), this, SLOT( containerMoved( qint64 ) ) );
        connect( item, SIGNAL( trackChanged( GraphicsTrack* ) ),
                 this, SLOT( setTrack( GraphicsTrack* ) ) );
        connect( item, SIGNAL( destroyed() ), this, SLOT( deleteLater() ) );
        connect( item, &AbstractGraphicsMediaItem::lengthChanged, this, &GraphicsEffectItem::adjustLength );
        if ( m_effectHelper->length() > item->helper()->length() )
            m_effectHelper->setBoundaries( 0, item->helper()->length() );
        if ( startPos() < item->pos().x() )
            setStartPos( item->pos().x() );
        if ( startPos() + width() > item->pos().x() + item->width() )
            setStartPos( item->pos().x() + item->width() - width() );
    }
}

const AbstractGraphicsMediaItem*
GraphicsEffectItem::container() const
{
    return m_container;
}

void
GraphicsEffectItem::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
    if ( !tracksView() )
        return;

    QMenu menu( tracksView() );

    QAction* removeAction = menu.addAction( tr( "Remove" ) );
    QAction* changeColorAction = menu.addAction( tr( "Set color" ) );
    menu.addSeparator();
    QAction* changeEffectProperties = menu.addAction( tr( "Effect Properties" ) );

    QAction* selectedAction = menu.exec( event->screenPos() );
    if ( !selectedAction )
        return;

    if ( selectedAction == removeAction )
    {
        setSelected( true );
        scene()->askRemoveSelectedItems();
        m_tracksView->removeItem( this );
    }
    else if ( selectedAction == changeColorAction )
    {
        m_itemColor = QColorDialog::getColor( m_itemColor, tracksView() );
        update();
    }
    else if ( selectedAction == changeEffectProperties )
    {
        EffectInstanceWidget *effectProperties = new EffectInstanceWidget();
        effectProperties->setEffectHelper( m_effectHelper );
        effectProperties->show();
    }
}

void
GraphicsEffectItem::setStartPos( qint64 position )
{
    if ( m_effectHelper != nullptr && m_effectHelper->isValid() )
    {
        int     nbFilter = m_effectHelper->filter()->input()->filterCount() - 1;
        QGraphicsItem::setPos( position, nbFilter * itemHeight() );
    }
    else
        QGraphicsItem::setPos( position, 0 );
}
