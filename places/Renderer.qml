/*
 * This file is part of unity-2d
 *
 * Copyright 2010-2011 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.0

/* A renderer is used to display a group of a place, that is a set of items from
   a place that share the same group id. Concretely speaking, it will be a section
   of search results, for example, "Available Applications".

   The renderer to be used for a given group is chosen by the place backend
   itself. A typical renderer is the UnityDefaultRender that lays out the items
   in a grid of icons with the item's title underneath it.
*/
Item {
    property string displayName /* Name of the group typically displayed in the header */
    property string iconHint /* Icon id of the group */
    property int groupId /* Index of the group */
    property variant model /* List model containing the items to be displayed by the renderer */
    property variant placeEntryModel /* Reference to the place entry the group belongs to */
    property variant parentListView /* Reference to the ListView the renderer is nested into */
}
