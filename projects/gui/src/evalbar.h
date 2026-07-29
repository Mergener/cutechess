/*
    This file is part of Cute Chess.
    Copyright (C) 2008-2026 Cute Chess authors

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#ifndef EVALBAR_H
#define EVALBAR_H

#include <QHash>
#include <QPointer>
#include <QWidget>
#include <board/side.h>

class ChessPlayer;
class MoveEvaluation;

/*!
 * \brief A white-point-of-view evaluation bar for one engine.
 *
 * The white portion grows from the bottom of the bar as White's
 * advantage increases.  Engine evaluations are converted from the
 * player's point of view before they are displayed.
 */
class EvalBar : public QWidget
{
	Q_OBJECT

	public:
		explicit EvalBar(QWidget* parent = nullptr);

		/*!
		 * Connects the bar to \a player.  \a side is the colour played by
		 * that engine and is used to convert its score to White's point
		 * of view.
		 */
		void setPlayer(ChessPlayer* player, Chess::Side side, int ply = -1);

		/*! Sets the direction in which the evaluation bar is drawn. */
		void setOrientation(Qt::Orientation orientation);

		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;

	public slots:
		void viewMove(int ply);
		void onMoveMade(int ply);

	protected:
		void paintEvent(QPaintEvent* event) override;

	private slots:
		void clear();
		void onEval(const MoveEvaluation& eval);

	private:
		struct BarState
		{
			int score;
			bool hasScore;
			bool isBook;

			BarState();
		};

		void showCurrentPosition();
		void setState(const BarState& state);
		QString scoreText() const;
		qreal whiteShare() const;

		QPointer<ChessPlayer> m_player;
		Chess::Side m_side;
		Qt::Orientation m_orientation;
		QHash<int, BarState> m_history;
		int m_livePly;
		int m_viewedPly;
		int m_score;
		bool m_hasScore;
		bool m_isBook;
};

#endif // EVALBAR_H
