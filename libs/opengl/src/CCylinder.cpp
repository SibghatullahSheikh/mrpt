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

#include <mrpt/opengl.h>  // Precompiled header
#include <mrpt/opengl/CCylinder.h>
#include <mrpt/math/geometry.h>

#include "opengl_internals.h"

using namespace mrpt;
using namespace mrpt::opengl;
using namespace mrpt::utils;
using namespace std;

IMPLEMENTS_SERIALIZABLE(CCylinder,CRenderizableDisplayList,mrpt::opengl)

/*---------------------------------------------------------------
							render
  ---------------------------------------------------------------*/
void CCylinder::render_dl() const	{
#if MRPT_HAS_OPENGL_GLUT
	glEnable (GL_BLEND);
	checkOpenGLError();
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	checkOpenGLError();
	GLUquadricObj *obj=gluNewQuadric();

	// This is required to draw cylinders of negative height.
	const float absHeight = std::abs(mHeight);
	if (mHeight<0)
	{
		glPushMatrix();
		glTranslatef(0,0,mHeight);
	}

	gluCylinder(obj,mBaseRadius,mTopRadius,absHeight,mSlices,mStacks);

	if (mHeight<0)
		glPopMatrix();

	if (mHasBottomBase) gluDisk(obj,0,mBaseRadius,mSlices,1);
	if (mHasTopBase&&mTopRadius>0)	{
		glPushMatrix();
		glTranslatef(0,0,mHeight);
		gluDisk(obj,0,mTopRadius,mSlices,1);
		glPopMatrix();
	}
	gluDeleteQuadric(obj);
	glDisable(GL_BLEND);

#endif
}

/*---------------------------------------------------------------
   Implements the writing to a CStream capability of
     CSerializable objects
  ---------------------------------------------------------------*/
void CCylinder::writeToStream(CStream &out,int *version) const	{
	if (version) *version=0;
	else	{
		writeToStreamRender(out);
		//version 0
		out<<mBaseRadius<<mTopRadius<<mHeight<<mSlices<<mStacks<<mHasBottomBase<<mHasTopBase;
	}
}

/*---------------------------------------------------------------
	Implements the reading from a CStream capability of
		CSerializable objects
  ---------------------------------------------------------------*/
void CCylinder::readFromStream(CStream &in,int version)	{
	switch (version)	{
		case 0:
			readFromStreamRender(in);
			in>>mBaseRadius>>mTopRadius>>mHeight>>mSlices>>mStacks>>mHasBottomBase>>mHasTopBase;
			break;
		default:
			MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version)
	};
	CRenderizableDisplayList::notifyChange();
}

bool solveEqn(double a,double b,double c,double &t)	{	//Actually, the b from the quadratic equation is the DOUBLE of this. But this way, operations are simpler.
	if (a<0)	{
		a=-a;
		b=-b;
		c=-c;
	}
	if (a>=mrpt::math::geometryEpsilon)	{
		double delta=square(b)-a*c;
		if (delta==0) return (t=-b/a)>=0;
		else if (delta>=0)	{
			delta=sqrt(delta);
			if (-b-delta>0)	{
				t=(-b-delta)/a;
				return true;
			}	else if (-b+delta>0)	{
				t=(-b+delta)/a;
				return true;
			}	//else return false;	Both solutions are negative
		}	//else return false;	Both solutions are complex
	}	else if (abs(b)>=geometryEpsilon)	 {
		t=-c/(b+b);
		return t>=0;
	}	//else return false;	This actually isn't an equation
	return false;
}

bool CCylinder::traceRay(const mrpt::poses::CPose3D &o,double &dist) const	{
	TLine3D lin;
	createFromPoseX(o-this->m_pose,lin);
	lin.unitarize();	//By adding this line, distance from any point of the line to its base is exactly equal to the "t".
	if (abs(lin.director[2])<geometryEpsilon)	{
		if (!reachesHeight(lin.pBase.z)) return false;
		float r;
		return getRadius(static_cast<float>(lin.pBase.z),r)?solveEqn(square(lin.director[0])+square(lin.director[1]),lin.director[0]*lin.pBase.x+lin.director[1]*lin.pBase.y,square(lin.pBase.x)+square(lin.pBase.y)-square(r),dist):false;
	}
	bool fnd=false;
	double nDist,tZ0;
	if (mHasBottomBase&&(tZ0=-lin.pBase.z/lin.director[2])>0)	{
		nDist=sqrt(square(lin.pBase.x+tZ0*lin.director[0])+square(lin.pBase.y+tZ0*lin.director[1]));
		if (nDist<=mBaseRadius)	{
			fnd=true;
			dist=tZ0;
		}
	}
	if (mHasTopBase)	{
		tZ0=(mHeight-lin.pBase.z)/lin.director[2];
		if (tZ0>0&&(!fnd||tZ0<dist))	{
			nDist=sqrt(square(lin.pBase.x+tZ0*lin.director[0])+square(lin.pBase.y+tZ0*lin.director[1]));
			if (nDist<=mTopRadius)	{
				fnd=true;
				dist=tZ0;
			}
		}
	}
	if (mBaseRadius==mTopRadius)	{
		if (solveEqn(square(lin.director[0])+square(lin.director[1]),lin.director[0]*lin.pBase.x+lin.director[1]*lin.pBase.y,square(lin.pBase.x)+square(lin.pBase.y)-square(mBaseRadius),nDist)) if ((!fnd||nDist<dist)&&reachesHeight(lin.pBase.z+nDist*lin.director[2]))	{
			dist=nDist;
			fnd=true;
		}
	}	else	{
		double slope=(mTopRadius-mBaseRadius)/mHeight;
		if (solveEqn(square(lin.director[0])+square(lin.director[1])-square(lin.director[2]*slope),lin.pBase.x*lin.director[0]+lin.pBase.y*lin.director[1]-(mBaseRadius+slope*lin.pBase.z)*slope*lin.director[2],square(lin.pBase.x)+square(lin.pBase.y)-square(mBaseRadius+slope*lin.pBase.z),nDist)) if ((!fnd||nDist<dist)&&reachesHeight(lin.pBase.z+nDist*lin.director[2]))	{
			dist=nDist;
			fnd=true;
		}
	}
	return fnd;
}



void CCylinder::getBoundingBox(mrpt::math::TPoint3D &bb_min, mrpt::math::TPoint3D &bb_max) const
{
	bb_min.x = -std::max(mBaseRadius,mTopRadius);
	bb_min.y = bb_min.x;
	bb_min.z = 0;

	bb_max.x = std::max(mBaseRadius,mTopRadius);
	bb_max.y = bb_max.x;
	bb_max.z = mHeight;

	// Convert to coordinates of my parent:
	m_pose.composePoint(bb_min, bb_min);
	m_pose.composePoint(bb_max, bb_max);
}
