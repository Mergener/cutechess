/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#pragma once

#include <QIcon>
#include <QPalette>
#include <QString>
#include <QWidget>

inline QIcon themedToolButtonIcon(const QWidget* widget,
					 const QString& lightIcon,
					 const QString& darkIcon)
{
	const QColor buttonColor = widget->palette().color(QPalette::Button);
	return QIcon(buttonColor.lightness() < 128 ? darkIcon : lightIcon);
}

inline QIcon themedToggleIcon(const QWidget* widget,
					const QString& lightOff, const QString& lightOn,
					const QString& darkOff, const QString& darkOn)
{
	const QColor buttonColor = widget->palette().color(QPalette::Button);
	const bool dark = buttonColor.lightness() < 128;
	QIcon icon(dark ? darkOff : lightOff);
	icon.addFile(dark ? darkOn : lightOn, QSize(), QIcon::Normal, QIcon::On);
	return icon;
}
