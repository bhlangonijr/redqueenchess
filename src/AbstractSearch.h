/*
	Redqueen Chess Engine
    Copyright (C) 2008-2009 Ben-Hur Carlos Vieira Langoni Junior

    This file is part of Redqueen Chess Engine.

    Redqueen is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Redqueen is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Redqueen.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * AbstractSearch.h
 *
 *  Created on: May 16, 2009
 *      Author: bhlangonijr
 */

#ifndef ABSTRACTSEARCH_H_
#define ABSTRACTSEARCH_H_

namespace AbstractSearchTypes {

}

class AbstractSearch {
public:
	virtual ~AbstractSearch();
	virtual void search() = 0;
	virtual int getScore() = 0;

protected:
	AbstractSearch();
};

template <class SearchClass>
class Search : public  AbstractSearch {
	typedef void (SearchClass::* Action)();

	Search(SearchClass* searchClass, Action action) : _searchClass(searchClass), _action(action)
	{}
	virtual void search();
	virtual int getScore();
private:
	Action _action;
	SearchClass _searchClass;
};

template <class SearchClass>
void Search<SearchClass>::search () {
	(_searchClass->*_action)();
}


#endif /* ABSTRACTSEARCH_H_ */
