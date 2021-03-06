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

#include <mrpt/obs.h>   // Precompiled headers

#include <mrpt/slam/CActionCollection.h>
#include <mrpt/slam/CActionRobotMovement2D.h>
#include <mrpt/slam/CActionRobotMovement3D.h>
#include <mrpt/poses/CPosePDF.h>

using namespace mrpt;
using namespace mrpt::slam;
using namespace mrpt::poses;
using namespace mrpt::utils;

#include <mrpt/utils/metaprogramming.h>
using namespace mrpt::utils::metaprogramming;

IMPLEMENTS_SERIALIZABLE(CActionCollection, CSerializable, mrpt::slam)


/*---------------------------------------------------------------
						Constructor
  ---------------------------------------------------------------*/
CActionCollection::CActionCollection() : m_actions()
{
}

/*---------------------------------------------------------------
						Constructor
  ---------------------------------------------------------------*/
CActionCollection::CActionCollection(CAction &a ) : m_actions()
{
	m_actions.push_back( CActionPtr( static_cast<CAction*>(a.duplicate()) ) );
}

/*---------------------------------------------------------------
						Copy constructor
  ---------------------------------------------------------------*/
CActionCollection::CActionCollection(const CActionCollection &o ) : CSerializable(), m_actions()
{
	m_actions = o.m_actions;
	for_each(m_actions.begin(),m_actions.end(), ObjectMakeUnique() );
}

/*---------------------------------------------------------------
						Copy operator
  ---------------------------------------------------------------*/
CActionCollection& CActionCollection::operator = (const CActionCollection &o )
{
	if (this==&o) return *this;

	m_actions = o.m_actions;
	for_each(m_actions.begin(),m_actions.end(), ObjectMakeUnique() );

	return *this;
}


/*---------------------------------------------------------------
						Destructor
  ---------------------------------------------------------------*/
CActionCollection::~CActionCollection()
{
	clear();
}


/*---------------------------------------------------------------
  Implements the writing to a CStream capability of CSerializable objects
 ---------------------------------------------------------------*/
void  CActionCollection::writeToStream(CStream &out, int *version) const
{
	if (version)
		*version = 0;
	else
	{
		uint32_t		n;

		n = static_cast<uint32_t> ( m_actions.size() );
		out << n;
		for (const_iterator it=begin();it!=end();++it)
			out << *(*it);
	}
}

/*---------------------------------------------------------------
  Implements the reading from a CStream capability of CSerializable objects
 ---------------------------------------------------------------*/
void  CActionCollection::readFromStream(CStream &in, int version)
{
	switch(version)
	{
	case 0:
		{
			uint32_t	n;

			clear();

			in >> n;
			m_actions.resize(n);
			for_each( begin(),end(), ObjectReadFromStream(&in) );

		} break;
	default:
		MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version)
	};
}

/*---------------------------------------------------------------
						clear
 ---------------------------------------------------------------*/
void  CActionCollection::clear()
{
	m_actions.clear();
}

/*---------------------------------------------------------------
						get
 ---------------------------------------------------------------*/
CActionPtr CActionCollection::get(size_t index)
{
	if (index>=m_actions.size())
		THROW_EXCEPTION("Index out of bounds");

	return m_actions[index];
}


/*---------------------------------------------------------------
						size
 ---------------------------------------------------------------*/
size_t  CActionCollection::size()
{
	return m_actions.size();
}

/*---------------------------------------------------------------
						insert
 ---------------------------------------------------------------*/
void  CActionCollection::insert(CAction	&action)
{
	m_actions.push_back( CActionPtr( static_cast<CAction*>( action.duplicate() )) );
}


/*---------------------------------------------------------------
						getBestMovementEstimation
 ---------------------------------------------------------------*/
CActionRobotMovement2DPtr  CActionCollection::getBestMovementEstimation() const
{
	CActionRobotMovement2DPtr	bestEst;
	double						bestDet = 1e3;

	// Find the best
	for (const_iterator it=begin();it!=end();++it)
	{
		if ((*it)->GetRuntimeClass()->derivedFrom( CLASS_ID( CActionRobotMovement2D ) ) )
		{
			CActionRobotMovement2DPtr temp = CActionRobotMovement2DPtr( *it );

			if (temp->estimationMethod == CActionRobotMovement2D::emScan2DMatching )
			{
				return temp;
			}

			double	det = temp->poseChange->getCovariance().det();

			// If it is the best until now, save it:
			if ( det<bestDet )
			{
				bestEst = temp;
				bestDet = det;
			}
		}
	}

	return bestEst;
}


/*---------------------------------------------------------------
							eraseByIndex
 ---------------------------------------------------------------*/
void  CActionCollection::eraseByIndex(const size_t & index)
{
	if (index>=m_actions.size())
		THROW_EXCEPTION("Index out of bounds");

	iterator it = m_actions.begin() + index;
	m_actions.erase( it );
}

/*---------------------------------------------------------------
							eraseByIndex
 ---------------------------------------------------------------*/
CActionRobotMovement2DPtr CActionCollection::getMovementEstimationByType( CActionRobotMovement2D::TEstimationMethod method)
{
	// Find it:
	for (iterator it=begin();it!=end();++it)
	{
		if ((*it)->GetRuntimeClass()->derivedFrom( CLASS_ID( CActionRobotMovement2D ) ) )
		{
			CActionRobotMovement2DPtr temp = CActionRobotMovement2DPtr( *it );

			// Is it of the required type?
			if ( temp->estimationMethod == method )
			{
				// Yes!:
				return temp;
			}
		}
	}

	// Not found:
	return CActionRobotMovement2DPtr();
}

/*---------------------------------------------------------------
							erase
 ---------------------------------------------------------------*/
CActionCollection::iterator CActionCollection::erase( const iterator &it)
{
	MRPT_START
	ASSERT_(it!=end());

	return m_actions.erase(it);
	MRPT_END

}

/*---------------------------------------------------------------
							getFirstMovementEstimationMean
 ---------------------------------------------------------------*/
bool CActionCollection::getFirstMovementEstimationMean( CPose3D &out_pose_increment ) const
{
	CActionRobotMovement3DPtr act3D = getActionByClass<CActionRobotMovement3D>();
	if (act3D)
	{
		out_pose_increment = act3D->poseChange.mean;
		return true;
	}
	CActionRobotMovement2DPtr act2D = getActionByClass<CActionRobotMovement2D>();
	if (act2D)
	{
		out_pose_increment = CPose3D(act2D->poseChange->getMeanVal());
		return true;
	}
	return false;
}

/*---------------------------------------------------------------
					getFirstMovementEstimation
 ---------------------------------------------------------------*/
bool CActionCollection::getFirstMovementEstimation( CPose3DPDFGaussian &out_pose_increment ) const
{
	CActionRobotMovement3DPtr act3D = getActionByClass<CActionRobotMovement3D>();
	if (act3D)
	{
		out_pose_increment = act3D->poseChange;
		return true;
	}
	CActionRobotMovement2DPtr act2D = getActionByClass<CActionRobotMovement2D>();
	if (act2D)
	{
		out_pose_increment.copyFrom( *act2D->poseChange );
		return true;
	}
	return false;
}
