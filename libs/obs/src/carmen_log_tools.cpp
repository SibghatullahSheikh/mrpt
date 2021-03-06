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

#include <mrpt/slam/CObservation2DRangeScan.h>
#include <mrpt/slam/CObservationOdometry.h>
#include <mrpt/utils/TParameters.h>

#include <mrpt/system/string_utils.h>

#include <mrpt/slam/carmen_log_tools.h>


using namespace mrpt;
using namespace mrpt::utils;
using namespace mrpt::slam;
using namespace mrpt::system;
using namespace std;

// Read the declaration in the .h file for documentation.
bool mrpt::slam::carmen_log_parse_line(
	std::istream &in_stream,
	std::vector<mrpt::slam::CObservationPtr> &out_observations,
	const mrpt::system::TTimeStamp &time_start_log )
{
	static TParametersString  global_log_params;  // global parameters loaded in previous calls.

	out_observations.clear(); // empty output container

	// Try to get line:
	string line;
	while (line.empty())
	{
		if (!in_stream)
			return false; // End of file
		std::getline(in_stream, line);
		line = trim(line);
	};

	// Now we have a line: analyze it:
	if ( strStartsI(line, "ROBOTLASER") )
	{
		// ROBOTLASER message
		// ---------------------------
		std::istringstream  S;  // Read from the string as if it was a stream
		S.str(line);

		CObservation2DRangeScanPtr obsLaser_ptr = CObservation2DRangeScan::Create();
		CObservation2DRangeScan* obsLaser = obsLaser_ptr.pointer(); // Faster access

		// Parse:
		int 	laser_type; //  SICK_LMS = 0, SICK_PLS = 1, HOKUYO_URG = 2, SIMULATED_LASER = 3,
		double 	start_angle, angular_resolution ,  accuracy;
		int 	remission_mode; // OFF = 0, DIRECT = 1, NORMALIZED = 2

		if (! (S >> obsLaser->sensorLabel
			    >> laser_type >> start_angle >> obsLaser->aperture >> angular_resolution >> obsLaser->maxRange >> accuracy >> remission_mode ) )
			THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (params):\n'%s'\n", line.c_str() )

		size_t nRanges;
		S >> nRanges;

		obsLaser->scan.resize(nRanges);
		obsLaser->validRange.resize(nRanges);

		for(size_t i=0;i<nRanges;i++)
		{
			if (! (S >> obsLaser->scan[i]) )
				THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (ranges):\n'%s'\n", line.c_str() )
			// Valid value?
			obsLaser->validRange[i] =  (obsLaser->scan[i]>=obsLaser->maxRange || obsLaser->scan[i]<=0 ) ? 0 : 1;
		}

		size_t remmision_count;
		if (! (S >> remmision_count) )
			THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (remmision_count):\n'%s'\n", line.c_str() )

		vector<double> remission;
		remission.resize(remmision_count);

		for(size_t i=0; i<remmision_count; i++)
		{
			if (! (S >> remission[i]) )
				THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (remmision vals):\n'%s'\n", line.c_str() )
		}

		mrpt::math::TPose2D  globalLaserPose;
		mrpt::math::TPose2D  globalRobotPose;

		if (! ( S  >> globalLaserPose.x >> globalLaserPose.y >> globalLaserPose.phi
				>> globalRobotPose.x >> globalRobotPose.y >> globalRobotPose.phi ) )
					THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (poses):\n'%s'\n", line.c_str() )

		// Compute pose of laser on the robot:
		obsLaser->sensorPose = CPose3D( CPose2D(globalLaserPose) - CPose2D(globalRobotPose) );

		double tv,rv,fw_dist, side_dist,turn_axis;
		S >> tv >> rv >> fw_dist >> side_dist >> turn_axis;

		double timestamp;
		string robotName;
		S  >> timestamp >> robotName;

		const mrpt::system::TTimeStamp obs_time = time_start_log + mrpt::system::secondsToTimestamp(timestamp); // seconds -> times

		obsLaser->timestamp = obs_time;

		// Create odometry observation:
		{
			CObservationOdometryPtr obsOdo_ptr = CObservationOdometry::Create();

			obsOdo_ptr->timestamp = obs_time;
			obsOdo_ptr->odometry = CPose2D(globalRobotPose);
			obsOdo_ptr->sensorLabel = "ODOMETRY";

			out_observations.push_back(obsOdo_ptr);
		}

		// Send out laser observation:
		out_observations.push_back(obsLaser_ptr);

	} // end ROBOTLASER
	else
	if ( strStartsI(line, "FLASER") || strStartsI(line,"RLASER") )
	{
		// [F,R]LASER message
		// FLASER num_readings [range_readings] x y theta odom_x odom_y odom_theta
		// ---------------------------
		std::istringstream  S;  // Read from the string as if it was a stream
		S.str(line);

		CObservation2DRangeScanPtr obsLaser_ptr = CObservation2DRangeScan::Create();
		CObservation2DRangeScan* obsLaser = obsLaser_ptr.pointer(); // Faster access

		// Parse:
		size_t  nRanges;

		if (! (S >> obsLaser->sensorLabel >> nRanges) )
			THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (params):\n'%s'\n", line.c_str() )

		// Params:
		{
			double maxRange = 81.0;
			double resolutionDeg = 0.5;

			if (line[0]=='F')
			{	// front:
				maxRange      = atof(global_log_params.getWithDefaultVal("robot_front_laser_max","81.0").c_str());
				resolutionDeg = atof(global_log_params.getWithDefaultVal("laser_front_laser_resolution","0.5").c_str());
			}
			else if (line[0]=='R')
			{	// rear:
				maxRange      = atof(global_log_params.getWithDefaultVal("robot_rear_laser_max","81.0").c_str());
				resolutionDeg = atof(global_log_params.getWithDefaultVal("laser_rear_laser_resolution","0.5").c_str());
			}
			obsLaser->maxRange = maxRange;
			obsLaser->aperture = DEG2RAD(resolutionDeg) * nRanges;
		}

		obsLaser->scan.resize(nRanges);
		obsLaser->validRange.resize(nRanges);

		for(size_t i=0;i<nRanges;i++)
		{
			if (! (S >> obsLaser->scan[i]) )
				THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (ranges):\n'%s'\n", line.c_str() )
			// Valid value?
			obsLaser->validRange[i] =  (obsLaser->scan[i]>=obsLaser->maxRange || obsLaser->scan[i]<=0 ) ? 0 : 1;
		}

		mrpt::math::TPose2D  globalLaserPose;
		mrpt::math::TPose2D  globalRobotPose;
		if (! ( S  >> globalLaserPose.x >> globalLaserPose.y >> globalLaserPose.phi
				>> globalRobotPose.x >> globalRobotPose.y >> globalRobotPose.phi ) )
					THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (poses):\n'%s'\n", line.c_str() )

		// Compute pose of laser on the robot:
		obsLaser->sensorPose = CPose3D( CPose2D(globalLaserPose) - CPose2D(globalRobotPose) );


		double timestamp;
		string robotName;
		S  >> timestamp >> robotName;

		const mrpt::system::TTimeStamp obs_time = time_start_log + mrpt::system::secondsToTimestamp(timestamp); // seconds -> times

		obsLaser->timestamp = obs_time;

		// Create odometry observation:
		{
			CObservationOdometryPtr obsOdo_ptr = CObservationOdometry::Create();

			obsOdo_ptr->timestamp = obs_time;
			obsOdo_ptr->odometry = CPose2D(globalRobotPose);
			obsOdo_ptr->sensorLabel = "ODOMETRY";

			out_observations.push_back(obsOdo_ptr);
		}

		// Send out laser observation:
		out_observations.push_back(obsLaser_ptr);

	} // end RAWLASER
	else
	if ( strStartsI(line, "PARAM ") )
	{
		// PARAM message
		// ---------------------------
		std::istringstream  S;  // Read from the string as if it was a stream
		S.str(line);

		string key, val;
		S >> key; // This is "PARAM"

		if (! (S >> key >> val) )
			THROW_EXCEPTION_CUSTOM_MSG1("Error parsing line from CARMEN log (PARAM):\n'%s'\n", line.c_str() )

		if (!key.empty() && !val.empty() )

		global_log_params[ lowerCase(key) ] = val;


	} // end RAWLASER


	return true; // OK
}
