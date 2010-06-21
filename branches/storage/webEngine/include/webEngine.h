/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
	by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __WEBENGINE_H__
#define __WEBENGINE_H__

#define WE_VERSION_MAJOR        0
#define WE_VERSION_MINOR        2
#define WE_VERSION_BUILDNO      3
#define WE_VERSION_EXTEND       835

#define WE_VERSION_PRODUCTSTR   "0,2,3,835"

/**
	@namespace	webEngine
	@brief  webEngine library. Set of interfaces and utilities to operate with network targets.

	@mainpage
    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
	by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.

	@image html http://www.gnu.org/graphics/lgplv3-147x51.png

	webEngine contains some subsets of routinees. It may be called sub-libraries.
	For operation with shared objects (*.so on @b *NIX or *.dll on @b Windows) used
	@ref shared_object "dyn::shared_object library". For difference operation used
	@ref dtl "Diff Templates Library". For ability to use different database engines
	webEngine provides @ref db_layer "database abstraction layer". This set of classes
	may be used to sore different structures. See the <a href="classes.html"><b>class
	index</b></a> page to the complete list of library classes.
*/

#endif //__WEBENGINE_H__