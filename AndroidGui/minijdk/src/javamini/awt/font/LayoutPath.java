/*
 * Copyright 2005 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 */
/*
 * (C) Copyright IBM Corp. 2005, All Rights Reserved.
 */
package javamini.awt.font;

import javamini.awt.geom.Point2D;

/**
 * LayoutPath provides a mapping between locations relative to the
 * baseline and points in user space.  Locations consist of an advance
 * along the baseline, and an offset perpendicular to the baseline at
 * the advance.  Positive values along the perpendicular are in the
 * direction that is 90 degrees clockwise from the baseline vector.
 * Locations are represented as a <code>Point2D</code>, where x is the advance and
 * y is the offset.
 *
 * @since 1.6
 */
public abstract class LayoutPath {
    /**
     * Convert a point in user space to a location relative to the
     * path. The location is chosen so as to minimize the distance
     * from the point to the path (e.g., the magnitude of the offset
     * will be smallest).  If there is more than one such location,
     * the location with the smallest advance is chosen.
     * @param point the point to convert.  If it is not the same
     * object as location, point will remain unmodified by this call.
     * @param location a <code>Point2D</code> to hold the returned location.
     * It can be the same object as point.
     * @return true if the point is associated with the portion of the
     * path preceding the location, false if it is associated with
     * the portion following.  The default, if the location is not at
     * a break or sharp bend in the path, is to return true.
     * @throws NullPointerException if point or location is null
     * @since 1.6
     */
    public abstract boolean pointToPath(Point2D point, Point2D location);

    /**
     * Convert a location relative to the path to a point in user
     * coordinates.  The path might bend abruptly or be disjoint at
     * the location's advance.  If this is the case, the value of
     * 'preceding' is used to disambiguate the portion of the path
     * whose location and slope is to be used to interpret the offset.
     * @param location a <code>Point2D</code> representing the advance (in x) and
     * offset (in y) of a location relative to the path.  If location
     * is not the same object as point, location will remain
     * unmodified by this call.
     * @param preceding if true, the portion preceding the advance
     * should be used, if false the portion after should be used.
     * This has no effect if the path does not break or bend sharply
     * at the advance.
     * @param point a <code>Point2D</code> to hold the returned point.  It can be
     * the same object as location.
     * @throws NullPointerException if location or point is null
     * @since 1.6
     */
    public abstract void pathToPoint(Point2D location, boolean preceding,
                                     Point2D point);
}
