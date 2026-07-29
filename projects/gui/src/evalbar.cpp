/*
    This file is part of Cute Chess.
    Copyright (C) 2008-2026 Cute Chess authors

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "evalbar.h"

#include <QtMath>
#include <QPalette>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <chessplayer.h>
#include <moveevaluation.h>

EvalBar::BarState::BarState()
	: score(0),
	  hasScore(false),
	  isBook(false)
{
}

EvalBar::EvalBar(QWidget* parent)
	: QWidget(parent),
	  m_player(nullptr),
	  m_animation(new QPropertyAnimation(
		this, "displayedWhiteShare", this)),
	  m_side(Chess::Side::NoSide),
	  m_orientation(Qt::Vertical),
	  m_livePly(-1),
	  m_viewedPly(-1),
	  m_score(0),
	  m_hasScore(false),
	  m_isBook(false),
	  m_displayedWhiteShare(0.5)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setToolTip(tr("White point of view"));
	m_animation->setDuration(180);
	m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void EvalBar::setPlayer(ChessPlayer* player, Chess::Side side, int ply)
{
	if (player != m_player || !player)
	{
		m_history.clear();
		setState(BarState());
	}

	if (m_player)
		m_player->disconnect(this);

	m_player = player;
	m_side = side;
	m_livePly = ply;
	m_viewedPly = ply;

	if (!player)
		return;

	connect(player, SIGNAL(startedThinking(int)),
		this, SLOT(clear()));
	connect(player, SIGNAL(thinking(MoveEvaluation)),
		this, SLOT(onEval(MoveEvaluation)));
}

void EvalBar::setOrientation(Qt::Orientation orientation)
{
	if (orientation == m_orientation)
		return;

	m_orientation = orientation;
	updateGeometry();
	update();
}

QSize EvalBar::sizeHint() const
{
	return m_orientation == Qt::Vertical
		? QSize(40, 220) : QSize(220, 40);
}

QSize EvalBar::minimumSizeHint() const
{
	return m_orientation == Qt::Vertical
		? QSize(20, 48) : QSize(48, 20);
}

void EvalBar::viewMove(int ply)
{
	m_viewedPly = ply;

	// The viewer can select the next position before onMoveMade() receives it.
	if (m_viewedPly == m_livePly + 1)
		return;

	showCurrentPosition();
}

void EvalBar::onMoveMade(int ply)
{
	if (ply <= m_livePly)
		return;

	bool viewingLivePosition = (m_viewedPly == m_livePly
				    || m_viewedPly == ply);

	while (m_livePly < ply)
	{
		m_history[m_livePly + 1] = m_history.value(m_livePly);
		m_livePly++;
	}

	if (viewingLivePosition)
	{
		m_viewedPly = m_livePly;
		showCurrentPosition();
	}
}

void EvalBar::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QRectF bar = rect();
	bar.adjust(5.0, 5.0, -5.0, -5.0);
	if (bar.isEmpty())
		return;

	const qreal radius = qMin<qreal>(6.0, bar.width() / 8.0);
	QPainterPath outline;
	outline.addRoundedRect(bar, radius, radius);

	painter.save();
	painter.setClipPath(outline);
	const bool darkMode = palette().color(QPalette::Base).lightness() < 128;
	const QColor whiteColor = darkMode ? QColor("#a3a3a3") : Qt::white;
	const QColor blackColor = darkMode ? QColor("#222222") : Qt::black;
	painter.fillRect(bar, blackColor);

	const qreal share = m_displayedWhiteShare;
	QRectF whiteRect;
	if (m_orientation == Qt::Vertical)
	{
		const qreal whiteHeight = bar.height() * share;
		whiteRect = QRectF(bar.left(), bar.bottom() - whiteHeight,
				  bar.width(), whiteHeight);
	}
	else
	{
		const qreal whiteWidth = bar.width() * share;
		whiteRect = QRectF(bar.left(), bar.top(),
				  whiteWidth, bar.height());
	}
	painter.fillRect(whiteRect, whiteColor);

	QPen divider(QColor(128, 128, 128));
	divider.setWidthF(1.0);
	painter.setPen(divider);
	if (m_orientation == Qt::Vertical)
	{
		const qreal middle = bar.top() + bar.height() / 2.0;
		painter.drawLine(QPointF(bar.left(), middle),
				 QPointF(bar.right(), middle));
	}
	else
	{
		const qreal middle = bar.left() + bar.width() / 2.0;
		painter.drawLine(QPointF(middle, bar.top()),
				 QPointF(middle, bar.bottom()));
	}
	painter.restore();

	painter.setPen(QPen(QColor(96, 96, 96), 1.0));
	painter.setBrush(Qt::NoBrush);
	painter.drawPath(outline);

	const QString text = scoreText();
	if (text.isEmpty())
		return;

	QFont font = painter.font();
	font.setBold(true);
	painter.setFont(font);

	QRectF textRect;
	bool textOnWhite;
	if (m_orientation == Qt::Vertical)
	{
		const int textHeight = painter.fontMetrics().height() + 8;
		textRect = QRectF(bar.left() + 2.0, bar.top() + 4.0,
				 bar.width() - 4.0, textHeight);
		textOnWhite = textRect.center().y() >= whiteRect.top();
	}
	else
	{
		const int textWidth = painter.fontMetrics()
			.horizontalAdvance(text) + 12;
		textRect = QRectF(bar.center().x() - textWidth / 2.0,
				 bar.top(), textWidth, bar.height());
		textOnWhite = textRect.center().x() <= whiteRect.right();
	}
	painter.setPen(textOnWhite ? blackColor : whiteColor);
	painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, text);
}

void EvalBar::clear()
{
	m_history[m_livePly] = BarState();
	if (m_viewedPly == m_livePly)
		showCurrentPosition();
}

void EvalBar::onEval(const MoveEvaluation& eval)
{
	// Secondary MultiPV lines must not move the position evaluation bar.
	if (eval.pvNumber() > 1)
		return;

	BarState& state = m_history[m_livePly];
	state.isBook = eval.isBookEval();
	if (eval.score() != MoveEvaluation::NULL_SCORE)
	{
		state.score = (m_side == Chess::Side::Black)
			      ? -eval.score() : eval.score();
		state.hasScore = true;
	}

	if (m_viewedPly == m_livePly)
		setState(state);
}

void EvalBar::showCurrentPosition()
{
	auto it = m_history.constFind(m_viewedPly);
	if (it == m_history.cend())
		setState(BarState());
	else
		setState(it.value());
}

void EvalBar::setState(const BarState& state)
{
	m_score = state.score;
	m_hasScore = state.hasScore;
	m_isBook = state.isBook;

	const qreal target = targetWhiteShare();
	if (!qFuzzyCompare(m_displayedWhiteShare + 1.0, target + 1.0))
	{
		m_animation->stop();
		m_animation->setStartValue(m_displayedWhiteShare);
		m_animation->setEndValue(target);
		m_animation->start();
	}
	update();
}

qreal EvalBar::displayedWhiteShare() const
{
	return m_displayedWhiteShare;
}

void EvalBar::setDisplayedWhiteShare(qreal share)
{
	m_displayedWhiteShare = share;
	update();
}

QString EvalBar::scoreText() const
{
	if (m_isBook)
		return tr("book");
	if (!m_hasScore)
		return QString();

	const int absScore = qAbs(m_score);
	if (absScore > MoveEvaluation::MATE_SCORE - 200)
	{
		const int moves = 1000 - (absScore % 1000);
		return QString("%1M%2")
			.arg(m_score < 0 ? "-" : "+")
			.arg(moves);
	}

	return QString("%1%2")
		.arg(m_score > 0 ? "+" : "")
		.arg(double(m_score) / 100.0, 0, 'f', 2);
}

qreal EvalBar::targetWhiteShare() const
{
	if (!m_hasScore)
		return 0.5;

	if (m_score >= MoveEvaluation::MATE_SCORE - 200)
		return 1.0;
	if (m_score <= -MoveEvaluation::MATE_SCORE + 200)
		return 0.0;

	// The nonlinear scale keeps small differences visible without letting
	// ordinary large centipawn scores make either colour disappear.
	return 1.0 / (1.0 + qExp(-0.00368208 * m_score));
}
