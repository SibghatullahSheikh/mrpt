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

#pragma once

#include <mrpt/srba/landmark_render_models.h>
#include <mrpt/srba/landmark_jacob_families.h>

namespace mrpt { namespace srba {
namespace landmarks {

	/** \defgroup mrpt_srba_landmarks Landmark parameterizations
		* \ingroup mrpt_srba_grp */

	/** \addtogroup mrpt_srba_landmarks
		* @{ */

	/** A parameterization of landmark positions in Euclidean coordinates (3D) */
	struct Euclidean3D
	{
		static const size_t  LM_DIMS = 3; //!< The number of parameters in each LM parameterization relative to its base KF: (x,y,z)
		static const landmark_jacob_family_t  jacob_family = jacob_point_landmark;  //!< Specify the kind of Jacobian to be used for compute_jacobian_dAepsDx_deps<>
		//static const size_t  LM_EUCLIDEAN_DIMS = 3; //!< Either 2 or 3, depending on the real minimum number of coordinates needed to parameterize the landmark.
		typedef mrpt::srba::landmark_rendering_as_point render_mode_t;

		/** Converts the landmark parameterization into 3D Eucliden coordinates (used for OpenGL rendering, etc.) */
		template <class VECTOR> inline static void relativeEuclideanLocation(const VECTOR &posParams, mrpt::math::TPoint3D &posEuclidean)
		{
			posEuclidean.x = posParams[0];
			posEuclidean.y = posParams[1];
			posEuclidean.z = posParams[2];
		}

		/** Evaluates pt = pose (+) pt
		  * \param[in,out] pt A vector with the landmark parameterization values
		  * \param[in] pose The relative pose */
		template <class POSE,class VECTOR>
		inline static void composePosePoint(VECTOR & pt, const POSE & pose) {
			pose.composePoint(pt[0],pt[1],pt[2], pt[0],pt[1],pt[2]);
		}
		/** Evaluates lm_local = lm_global (-) pose
		  * \param[in]  lm_global A vector with the landmark parameterization values in "global" coordinates
		  * \param[out] lm_local A vector with the landmark parameterization values in "local" coordinates, as seen from "pose"
		  * \param[in] pose The relative pose */
		template <class POSE,class VECTOR>
		static inline void inverseComposePosePoint(const VECTOR &lm_global,VECTOR &lm_local,const POSE &pose) {
			pose.inverseComposePoint(lm_global[0],lm_global[1],lm_global[2], lm_local[0],lm_local[1],lm_local[2]);
		}
	};

	/** A parameterization of landmark positions in Euclidean coordinates (2D) */
	struct Euclidean2D
	{
		static const size_t  LM_DIMS = 2; //!< The number of parameters in each LM parameterization relative to its base KF: (x,y)
		static const landmark_jacob_family_t  jacob_family = jacob_point_landmark;  //!< Specify the kind of Jacobian to be used for compute_jacobian_dAepsDx_deps<>
		//static const size_t  LM_EUCLIDEAN_DIMS = 2; //!< Either 2 or 3, depending on the real minimum number of coordinates needed to parameterize the landmark.
		typedef mrpt::srba::landmark_rendering_as_point render_mode_t;

		/** Converts the landmark parameterization into 3D Eucliden coordinates (used for OpenGL rendering, etc.) */
		template <class VECTOR> inline static void relativeEuclideanLocation(const VECTOR &posParams, mrpt::math::TPoint3D &posEuclidean)
		{
			posEuclidean.x = posParams[0];
			posEuclidean.y = posParams[1];
			posEuclidean.z = 0;
		}

		/** Evaluates pt = pose (+) pt
		  * \param[in,out] pt A vector with the landmark parameterization values
		  * \param[in] pose The relative pose */
		template <class POSE,class VECTOR>
		inline static void composePosePoint(VECTOR & pt, const POSE & pose) {
			double lx,ly,lz;
			pose.composePoint(pt[0],pt[1],0, lx,ly,lz);
			pt[0]=lx; pt[1]=ly;
			ASSERTMSG_(std::abs(lz)<1e-2, "Error: Using a 3D transformation to obtain a 2D point but it results in |z|>eps")
		}
		/** Evaluates lm_local = lm_global (-) pose
		  * \param[in]  lm_global A vector with the landmark parameterization values in "global" coordinates
		  * \param[out] lm_local A vector with the landmark parameterization values in "local" coordinates, as seen from "pose"
		  * \param[in] pose The relative pose */
		template <class POSE,class VECTOR>
		static inline void inverseComposePosePoint(const VECTOR &lm_global,VECTOR &lm_local,const POSE &pose) {
			pose.inverseComposePoint(lm_global[0],lm_global[1], lm_local[0],lm_local[1]);
		}
	};

	/** A parameterization of SE(2) relative poses ("fake landmarks" to emulate graph-SLAM) */
	struct RelativePoses2D
	{
		static const size_t  LM_DIMS = 3; //!< The number of parameters in each LM parameterization relative to its base KF
		static const landmark_jacob_family_t  jacob_family = jacob_relpose_landmark;  //!< Specify the kind of Jacobian to be used for compute_jacobian_dAepsDx_deps<>
		//static const size_t  LM_EUCLIDEAN_DIMS = 3; //!< Either 2 or 3, depending on the real minimum number of coordinates needed to parameterize the landmark.
		typedef mrpt::srba::landmark_rendering_as_pose_constraints render_mode_t;

		/** Evaluates pt = pose (+) pt
		  * \param[in,out] pt A vector with the landmark parameterization values
		  * \param[in] pose The relative pose */
		template <class POSE,class VECTOR>
		inline static void composePosePoint(VECTOR & pt, const POSE & pose) {
			// Not applicable: nothing to do on "pt"
		}
		/** Evaluates lm_local = lm_global (-) pose
		  * \param[in]  lm_global A vector with the landmark parameterization values in "global" coordinates
		  * \param[out] lm_local A vector with the landmark parameterization values in "local" coordinates, as seen from "pose"
		  * \param[in] pose The relative pose */
		template <class POSE,class VECTOR>
		static inline void inverseComposePosePoint(const VECTOR &lm_global,VECTOR &lm_local,const POSE &pose) {
			// Not applicable
			lm_local=lm_global;
		}

	};


	/** @} */

}
} } // end NS
