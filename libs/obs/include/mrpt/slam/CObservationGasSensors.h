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
#ifndef CObservationGasSensors_H
#define CObservationGasSensors_H

#include <mrpt/utils/CSerializable.h>
#include <mrpt/slam/CObservation.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/poses/CPose2D.h>

namespace mrpt
{
namespace slam
{

	DEFINE_SERIALIZABLE_PRE_CUSTOM_BASE_LINKAGE( CObservationGasSensors , CObservation, OBS_IMPEXP)

	/** Declares a class derived from "CObservation" that represents a set of readings from gas sensors.
	 *
	 * \sa CObservation
	 * \ingroup mrpt_obs_grp
	 */
	class OBS_IMPEXP CObservationGasSensors : public CObservation
	{
		// This must be added to any CSerializable derived class:
		DEFINE_SERIALIZABLE( CObservationGasSensors )

	 public:
		/** Constructor.
		 */
		CObservationGasSensors(  );

		 /** The structure for each e-nose
		  */
		struct OBS_IMPEXP TObservationENose
		{
			TObservationENose() :
				eNosePoseOnTheRobot(),
				readingsVoltage(),
				sensorTypes(),
				hasTemperature(false),
				temperature()
			{}

			/** The pose of the sensors on the robot
			  */
			math::TPose3D			eNosePoseOnTheRobot;

			/** The set of readings (in volts) from the array of sensors (size of "sensorTypes" is the same that the size of "readingsVoltage")
			  */
			std::vector<float>	readingsVoltage;

			/** The kind of sensors in the array (size of "sensorTypes" is the same that the size of "readingsVoltage")
			  *  The meaning of values for types of sensors is as follows:
			  *	 0x0000 : No sensor installed in this slot
			  *  0x2600 : Figaro TGS 2600
			  *  0x2602 : Figaro TGS 2602
			  *  0x2620 : Figaro TGS 2620
			  *  0x4161 : Figaro TGS 4161
			  */
			vector_int		sensorTypes;

			/** Must be true for "temperature" to contain a valid measurement
			  */
			bool			hasTemperature;

			/** Sensed temperature in Celcius (valid if hasTemperature=true only)
			  */
			float			temperature;

			/** True if the input to this chamber/enose is poluted air, False if clean air
			  */
			bool			isActive;

		};

		/** One entry per e-nose on the robot.
		  */
		std::vector<TObservationENose>		m_readings;

		/** A general method to retrieve the sensor pose on the robot.
		  *  Note that most sensors will return a full (6D) CPose3D, but see the derived classes for more details or special cases.
		  * \sa setSensorPose
		  */
		void getSensorPose( CPose3D &out_sensorPose ) const;


		/** A general method to change the sensor pose on the robot.
		  *  Note that most sensors will use the full (6D) CPose3D, but see the derived classes for more details or special cases.
		  * \sa getSensorPose
		  */
		void setSensorPose( const CPose3D &newSensorPose );


					/** Declares a class within "CObservationGasSensors" that represents a set of gas concentration readings from the modelation of a MOS gas sensor readings.
					 * This class provides the parameters and functions to simulate the inverse model of a MOS gas sensor.
					 *
					 * \sa CObservationGasSensors
					 */
					class OBS_IMPEXP CMOSmodel
					{

					public:
						/** Constructor
						  */
						CMOSmodel();
						~CMOSmodel();

						/** @name MOS-model parameters
						  *  @{  */

						size_t        winNoise_size;              //!< The size of the mobile average window used to reduce noise on sensor reagings.
						int           decimate_value;             //!< [useMOSmodel] The decimate frecuency applied after noise filtering
						
						float         a_rise;                     //!< tau = a*AMPLITUDE +b (linear relationship)
						float         b_rise;                     //!< tau = a*AMPLITUDE +b (linear relationship)
						float         a_decay;                    //!< tau = a*AMPLITUDE +b (linear relationship)
						float         b_decay;                    //!< tau = a*AMPLITUDE +b (linear relationship)
												
						bool          save_maplog;                //!< If true save generated gas map as a log file

						/** @} */

						/** Obtain an estimation of the gas distribution based on raw sensor readings  */
						bool get_GasDistribution_estimation(
							float							&reading,							
							mrpt::system::TTimeStamp		&timestamp );

					protected:

						/** The content of each m_lastObservations in the estimation when using the option : MOS_MODEl (useMOSmodel =1)
							*/
						struct OBS_IMPEXP TdataMap
						{
								float						reading;			//!< Sensore reading
								mrpt::system::TTimeStamp	timestamp;			//!< Timestamp of the observation
								float						tau;				//!< tau value applied								
								float						estimation;			//!< The value estimated according to the MOXmodel
								float						reading_filtered;	//!< Reading after smooth (noise averaging)								
						};

						TdataMap              last_Obs, temporal_Obs; //!< The content of each m_lastObservations in the estimation when using the option : MOS_MODEl (useMOSmodel =1)
						std::vector<TdataMap> m_antiNoise_window;  //!< Vector to temporally store and averge readings to reduce noise
						std::ofstream        *m_debug_dump;     //!< Ofstream to save to file option "save_maplog"
						uint16_t              decimate_count; //!< Decimate value for oversampled enose readings
						double                fixed_incT;  //!< To force e-nose samples to have fixed time increments
						bool                  first_incT;  //!< To force e-nose samples to have fixed time increments
						float                 min_reading;	//!< Minimum reading value till the moment, used as approximation to baeline level
						bool                  first_iteration;  //!< To avoid the model estimation on first iteration

						/** Estimates the gas concentration based on readings and sensor model
						  */
						void inverse_MOSmodeling (
							const float						&reading,							
							const mrpt::system::TTimeStamp	&timestamp);

						/** Reduce noise by averaging with a mobile window of specific size (winNoise_size)
						  */
						void noise_filtering (
							const float						&reading,							
							const mrpt::system::TTimeStamp	&timestamp );

						/** Save the gas distribution estiamtion into a log file for offline representation
						*/
						void save_log_map(
							const mrpt::system::TTimeStamp	&timestamp,
							const float						&reading,
							const float						&estimation,
							const float						&tau);

					}; //End of CMOSmodel class def.

	}; // End of class def.


	} // End of namespace
} // End of namespace

#endif
