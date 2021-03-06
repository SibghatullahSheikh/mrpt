/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2013, Individual contributors, see AUTHORS file        |
   | Copyright (c) 2005-2013, MAPIR group, University of Malaga                |
   | Copyright (c) 2012-2013, University of Almeria                            |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   +---------------------------------------------------------------------------+ */

#include <mrpt/base.h>  // Precompiled headers


#include <mrpt/poses/CPose2D.h>
#include <mrpt/poses/CPoint2D.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/poses/CPoint3D.h>
#include <mrpt/math/utils.h>

using namespace mrpt;
using namespace mrpt::math;
using namespace mrpt::poses;
using namespace mrpt::utils;

IMPLEMENTS_SERIALIZABLE(CPose2D, CSerializable ,mrpt::poses)

/*---------------------------------------------------------------
	Constructors
  ---------------------------------------------------------------*/
CPose2D::CPose2D() : m_phi(0),m_cossin_uptodate(false)
{
	m_coords[0] =
	m_coords[1] = 0;
}

CPose2D::CPose2D(const double x,const double y,const double _phi) : m_phi(_phi),m_cossin_uptodate(false)
{
	m_coords[0] = x;
	m_coords[1] = y;
	normalizePhi();
}

CPose2D::CPose2D(const CPoint2D &p) : m_phi(0),m_cossin_uptodate(false)
{
	m_coords[0] = p.x();
	m_coords[1] = p.y();
}

CPose2D::CPose2D(const CPose3D &p) : m_phi(p.yaw()),m_cossin_uptodate(false)
{
	m_coords[0] = p.x();
	m_coords[1] = p.y();
}

/*---------------------------------------------------------------
   Implements the writing to a CStream capability of
     CSerializable objects
  ---------------------------------------------------------------*/
void  CPose2D::writeToStream(CStream &out,int *version) const
{
	if (version)
		*version = 1;
	else
	{
		// The coordinates:
		out << m_coords[0] << m_coords[1] << m_phi;
	}
}

/*---------------------------------------------------------------
	Implements the reading from a CStream capability of
		CSerializable objects
  ---------------------------------------------------------------*/
void  CPose2D::readFromStream(CStream &in,int version)
{
	switch(version)
	{
	case 0:
		{
			// The coordinates:
			float x0,y0,phi0;
			in >> x0 >> y0 >> phi0;
			m_coords[0] = x0;
			m_coords[1] = y0;
			this->phi( phi0 );
		} break;
	case 1:
		{
			// The coordinates:
			in >> m_coords[0] >> m_coords[1] >> m_phi;
			m_cossin_uptodate=false;
		} break;
	default:
		MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version)
	};
}

/**  Textual output stream function.
 */
std::ostream& mrpt::poses::operator << (std::ostream& o, const CPose2D& p)
{
	o << format("(%.03f,%.03f,%.02fdeg)",p.x(),p.y(),RAD2DEG(p.phi()));
	return o;
}

/*---------------------------------------------------------------
The operator a="this"+D is the pose compounding operator.
 ---------------------------------------------------------------*/
CPose2D  CPose2D::operator + (const CPose2D& D)const
{
	update_cached_cos_sin();

	return CPose2D(
		m_coords[0] + D.m_coords[0] * m_cosphi - D.m_coords[1] * m_sinphi,
		m_coords[1] + D.m_coords[0] * m_sinphi + D.m_coords[1] * m_cosphi,
		m_phi + D.m_phi );
}

/*---------------------------------------------------------------
				composeFrom
 ---------------------------------------------------------------*/
void CPose2D::composeFrom(const CPose2D &A, const CPose2D &B)
{
	A.update_cached_cos_sin();

	// Use temporary variables for the cases (A==this) or (B==this)
	const double new_x = A.m_coords[0] + B.m_coords[0] * A.m_cosphi - B.m_coords[1] * A.m_sinphi;
	const double new_y = A.m_coords[1] + B.m_coords[0] * A.m_sinphi + B.m_coords[1] * A.m_cosphi;
	m_coords[0] = new_x;
	m_coords[1] = new_y;

	m_phi = math::wrapToPi(A.m_phi + B.m_phi);
	m_cossin_uptodate=false;
}

/*---------------------------------------------------------------
				getRotationMatrix
 ---------------------------------------------------------------*/
void CPose2D::getRotationMatrix(mrpt::math::CMatrixDouble22 &R) const
{
	update_cached_cos_sin();
	R(0,0) = m_cosphi;  R(0,1) = -m_sinphi;
	R(1,0) = m_sinphi;  R(1,1) =  m_cosphi;
}

void CPose2D::getRotationMatrix(mrpt::math::CMatrixDouble33 &R) const
{
	update_cached_cos_sin();
	R(0,0) = m_cosphi;  R(0,1) = -m_sinphi; R(0,2)=0;
	R(1,0) = m_sinphi;  R(1,1) =  m_cosphi; R(1,2)=0;
	R(2,0) = 0; R(2,1)=0; R(2,2)=1;
}




/*---------------------------------------------------------------
The operator a="this"+D is the pose compounding operator.
 ---------------------------------------------------------------*/
CPose3D  CPose2D::operator + (const CPose3D& D) const
{
	return CPose3D(*this) + D;
}

/*---------------------------------------------------------------
The operator u'="this"+u is the pose/point compounding operator.
 ---------------------------------------------------------------*/
CPoint2D CPose2D::operator + (const CPoint2D& u)const
{
	update_cached_cos_sin();

	return CPoint2D(
		m_coords[0] + u.x() * m_cosphi - u.y() * m_sinphi,
		m_coords[1] + u.x() * m_sinphi + u.y() * m_cosphi );
}

/** An alternative, slightly more efficient way of doing \f$ G = P \oplus L \f$ with G and L being 2D points and P this 2D pose.  */
void CPose2D::composePoint(double lx,double ly,double &gx, double &gy) const
{
	update_cached_cos_sin();

	gx = m_coords[0] + lx * m_cosphi - ly * m_sinphi;
	gy = m_coords[1] + lx * m_sinphi + ly * m_cosphi;
}

void CPose2D::composePoint(const mrpt::math::TPoint2D &l, mrpt::math::TPoint2D &g) const
{
	this->composePoint(l.x,l.y, g.x,g.y);
}

/** \overload \f$ G = P \oplus L \f$ with G and L being 3D points and P this 2D pose (the "z" coordinate remains unmodified) */
void CPose2D::composePoint(const mrpt::math::TPoint3D &l, mrpt::math::TPoint3D &g) const
{
	this->composePoint(l.x,l.y,l.z, g.x,g.y,g.z);
}

void CPose2D::composePoint(double lx,double ly,double lz, double &gx, double &gy, double &gz) const
{
	update_cached_cos_sin();
	gx = m_coords[0] + lx * m_cosphi - ly * m_sinphi;
	gy = m_coords[1] + lx * m_sinphi + ly * m_cosphi;
	gz = lz;
}


/*---------------------------------------------------------------
The operator u'="this"+u is the pose/point compounding operator.
 ---------------------------------------------------------------*/
CPoint3D CPose2D::operator + (const CPoint3D& u)const
{
	update_cached_cos_sin();

	return CPoint3D(
		m_coords[0] + u.x() * m_cosphi - u.y() * m_sinphi,
		m_coords[1] + u.x() * m_sinphi + u.y() * m_cosphi,
		u.z() );
}

/*---------------------------------------------------------------
The operator D="this"-b is the pose inverse compounding operator.
   The resulting pose "D" is the diference between this pose and "b"
 ---------------------------------------------------------------*/
void CPose2D::inverseComposeFrom(const CPose2D& A, const CPose2D& B )
{
	B.update_cached_cos_sin();

	m_coords[0] = (A.m_coords[0] - B.m_coords[0]) * B.m_cosphi + (A.m_coords[1] - B.m_coords[1]) * B.m_sinphi;
	m_coords[1] =-(A.m_coords[0] - B.m_coords[0]) * B.m_sinphi + (A.m_coords[1] - B.m_coords[1]) * B.m_cosphi;
	m_phi = math::wrapToPi(A.m_phi - B.m_phi);
	m_cossin_uptodate=false;
}

/*---------------------------------------------------------------
 Scalar sum of components: This is diferent from poses
   composition, which is implemented as "+" operators in "CPose" derived classes.
 ---------------------------------------------------------------*/
void CPose2D::AddComponents(CPose2D &p)
{
	m_coords[0]+=p.m_coords[0];
	m_coords[1]+=p.m_coords[1];
	m_phi+=p.m_phi;
	m_cossin_uptodate=false;
}


/*---------------------------------------------------------------
 Scalar multiplication.
 ---------------------------------------------------------------*/
void CPose2D::operator *= (const double s)
{
	m_coords[0]*=s;
	m_coords[1]*=s;
	m_phi*=s;
	m_cossin_uptodate=false;
}

/*---------------------------------------------------------------
	Returns the corresponding 4x4 homogeneous
	  transformation matrix for the point(translation),
	  or pose (translation+orientation).
---------------------------------------------------------------*/
void  CPose2D::getHomogeneousMatrix(CMatrixDouble44& m) const
{
	m.unit(4,1.0);

	m.set_unsafe(0,3, m_coords[0] );
	m.set_unsafe(1,3, m_coords[1] );

	update_cached_cos_sin();

	m.get_unsafe(0,0) = m_cosphi;  m.get_unsafe(0,1)=-m_sinphi;
	m.get_unsafe(1,0) = m_sinphi;  m.get_unsafe(1,1)= m_cosphi;
}

/** Forces "phi" to be in the range [-pi,pi];
*/
void  CPose2D::normalizePhi()
{
	m_phi = math::wrapToPi(m_phi);
	m_cossin_uptodate=false;
}

CPose2D::CPose2D(const mrpt::math::TPose2D &o):m_phi(o.phi),m_cossin_uptodate(false) {
	m_coords[0]=o.x;
	m_coords[1]=o.y;
}

CPose2D::CPose2D(const CPoint3D &o): m_phi(0),m_cossin_uptodate(false) {
	this->m_coords[0]	= o.x();
	this->m_coords[1]	= o.y();
	normalizePhi();
}

/*---------------------------------------------------------------
		unary -
---------------------------------------------------------------*/
CPose2D mrpt::poses::operator -(const CPose2D&b)
{
	CPose2D ret = b;
	ret.inverse();
	return ret;
}

/** Convert this pose into its inverse, saving the result in itself. \sa operator- */
void CPose2D::inverse()
{
	update_cached_cos_sin();
	const double x = m_coords[0];
	const double y = m_coords[1];

	m_coords[0] = -x * m_cosphi - y * m_sinphi;
	m_coords[1] =  x * m_sinphi - y * m_cosphi;
	m_phi = - m_phi;
	m_cossin_uptodate=false;
}


/*---------------------------------------------------------------
		getAsVector
---------------------------------------------------------------*/
void CPose2D::getAsVector(vector_double &v) const
{
	v.resize(3);
	v[0]=m_coords[0];
	v[1]=m_coords[1];
	v[2]=m_phi;
}

void CPose2D::getAsVector(mrpt::math::CArrayDouble<3> &v) const
{
	v[0]=m_coords[0];
	v[1]=m_coords[1];
	v[2]=m_phi;
}



bool mrpt::poses::operator==(const CPose2D &p1,const CPose2D &p2)
{
	return (p1.x()==p2.x())&&(p1.y()==p2.y())&&(p1.phi()==p2.phi());
}

bool mrpt::poses::operator!=(const CPose2D &p1,const CPose2D &p2)
{
	return (p1.x()!=p2.x())||(p1.y()!=p2.y())||(p1.phi()!=p2.phi());
}


TPoint2D mrpt::poses::operator +(const CPose2D &pose, const TPoint2D &u)
{
	const double ccos = pose.phi_cos();
	const double ssin = pose.phi_sin();
	return TPoint2D(
		pose.x() + u.x * ccos - u.y * ssin,
		pose.y() + u.x * ssin + u.y * ccos );
}


/** Make \f$ this = this \oplus b \f$  */
CPose2D& CPose2D::operator += (const CPose2D& b)
{
	composeFrom(*this,b);
	return *this;
}

void CPose2D::fromString(const std::string &s)
{
	CMatrixDouble  m;
	if (!m.fromMatlabStringFormat(s)) THROW_EXCEPTION("Malformed expression in ::fromString");
	ASSERTMSG_(mrpt::math::size(m,1)==1 && mrpt::math::size(m,2)==3, "Wrong size of vector in ::fromString");
	x( m.get_unsafe(0,0) );
	y( m.get_unsafe(0,1) );
	phi( DEG2RAD(m.get_unsafe(0,2)) );
}
