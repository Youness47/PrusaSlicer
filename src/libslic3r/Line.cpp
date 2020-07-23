#include "Geometry.hpp"
#include "Line.hpp"
#include "Polyline.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace Slic3r {

bool Line::intersection_infinite(const Line &other, Point* point) const
{
    Vec2d a1 = this->a.cast<double>();
    Vec2d v12 = (other.a - this->a).cast<double>();
    Vec2d v1 = (this->b - this->a).cast<double>();
    Vec2d v2 = (other.b - other.a).cast<double>();
    double denom = cross2(v1, v2);
    if (std::fabs(denom) < EPSILON)
        return false;
    double t1 = cross2(v12, v2) / denom;
    *point = (a1 + t1 * v1).cast<coord_t>();
    return true;
}

double Line::perp_distance_to(const Point &point) const
{
    const Line  &line = *this;
    const Vec2d  v  = (line.b - line.a).cast<double>();
    const Vec2d  va = (point - line.a).cast<double>();
    if (line.a == line.b)
        return va.norm();
    return std::abs(cross2(v, va)) / v.norm();
}

double Line::orientation() const
{
    double angle = this->atan2_();
    if (angle < 0) angle = 2*PI + angle;
    return angle;
}

double Line::direction() const
{
    double atan2 = this->atan2_();
    return (fabs(atan2 - PI) < EPSILON) ? 0
        : (atan2 < 0) ? (atan2 + PI)
        : atan2;
}

bool Line::parallel_to(double angle) const
{
    return Slic3r::Geometry::directions_parallel(this->direction(), angle);
}

bool Line::intersection(const Line &l2, Point *intersection) const
{
    const Line  &l1  = *this;
    const Vec2d  v1  = (l1.b - l1.a).cast<double>();
    const Vec2d  v2  = (l2.b - l2.a).cast<double>();
    double       denom  = cross2(v1, v2);
    if (fabs(denom) < EPSILON)
#if 0
        // Lines are collinear. Return true if they are coincident (overlappign).
        return ! (fabs(nume_a) < EPSILON && fabs(nume_b) < EPSILON);
#else
        return false;
#endif
    const Vec2d v12 = (l1.a - l2.a).cast<double>();
    double nume_a = cross2(v2, v12);
    double nume_b = cross2(v1, v12);
    double t1 = nume_a / denom;
    double t2 = nume_b / denom;
    if (t1 >= 0 && t1 <= 1.0f && t2 >= 0 && t2 <= 1.0f) {
        // Get the intersection point.
        (*intersection) = (l1.a.cast<double>() + t1 * v1).cast<coord_t>();
        return true;
    }
    return false;  // not intersecting
}

bool Line::clip_with_bbox(const BoundingBox &bbox)
{
	Vec2d x0clip, x1clip;
	bool result = Geometry::liang_barsky_line_clipping<double>(this->a.cast<double>(), this->b.cast<double>(), BoundingBoxf(bbox.min.cast<double>(), bbox.max.cast<double>()), x0clip, x1clip);
	if (result) {
		this->a = x0clip.cast<coord_t>();
		this->b = x1clip.cast<coord_t>();
	}
	return result;
}

BoundingBox get_extents(const Lines &lines)
{
    BoundingBox bbox;
    for (const Line &line : lines) {
        bbox.merge(line.a);
        bbox.merge(line.b);
    }
    return bbox;
}

} // namespace Slic3r
