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

#include <mrpt/hwdrivers/CActivMediaRobotBase.h>
#include <mrpt/base.h>

using namespace mrpt::hwdrivers;
using namespace mrpt::utils;
using namespace mrpt::math;
using namespace mrpt::slam;
using namespace mrpt::poses;
using namespace std;

int main(int argc, char **argv)
{
	try
	{
		// -------------------------------------------
		// Create the object:
		// -------------------------------------------
		CActivMediaRobotBase	robot;

		// -------------------------------------------
		// Load/set configuration:
		// -------------------------------------------
		// robot.loadConfig( ..., "robot_config" );    // Load from an .ini file

		// Set manually:
#ifdef MRPT_OS_WINDOWS
		string	port="COM4";
#else
		string	port="/dev/ttyUSB0";
#endif

		int 	port_baud = 115200;

		cout << "Usage: " << argv[0] << " [SERIAL_PORT [BAUD_RATE] ]" << endl;

        if (argc>=2)
        {
            port = std::string(argv[1]);

            if (argc>=3)
                port_baud = atoi(argv[2]);
        }

		cout << "Setting serial port to: " << port << " @ " << port_baud << endl;
		robot.setSerialPortConfig( port, port_baud );


		// -------------------------------------------
		//  Init comms:
		// -------------------------------------------
		robot.enableSonars();
		robot.initialize();

		robot.enableJoystickControl();


		double cur_v = 0;
		double cur_w = 0;


		CActivMediaRobotBase::TRobotDescription  robInfo;
		robot.getRobotInformation(robInfo);

		cout << "Robot # front bumpers : " << robInfo.nFrontBumpers << endl;
		cout << "Robot # rear bumpers  : " << robInfo.nRearBumpers << endl;
		cout << "Robot # sonars        : " << robInfo.nSonars << endl;


		bool show_menu = true;
		while (1)
		{
			if (show_menu)
			{
				show_menu=false;

				cout << "Press the key for your option:" << endl << endl;
				cout << " q/a   : +/- linear speed" << endl;
				cout << " s/d   : +/- angular speed" << endl;
				cout << " space : stop" << endl;
				cout << " o     : Query odometry" << endl;
				cout << " n     : Query sonars" << endl;
				cout << " b     : Query battery level" << endl;
				cout << " p     : Query bumpers" << endl;
				cout << " x     : Quit" << endl;
			}

			if (!mrpt::system::os::kbhit())
			{
				robot.doProcess();

				CGenericSensor::TListObservations dummy;
				robot.getObservations(dummy);  // Empty the list

				mrpt::system::sleep(20);
				continue;
			}
			char c = mrpt::system::os::getch();

			show_menu=true;


			if (c=='x') break;

			if (c=='w' || c=='s')
			{
				if (c=='w') cur_v += 0.05;
				if (c=='s') cur_v -= 0.05;
				robot.setVelocities( cur_v, cur_w );
			}

			if (c=='a' || c=='d')
			{
				if (c=='a') cur_w += 0.05;
				if (c=='d') cur_w -= 0.05;
				robot.setVelocities( cur_v, cur_w );
			}

			if (c==' ')
			{
				cur_v = 0;
				cur_w = 0;
				robot.setVelocities( cur_v, cur_w );
			}

			if (c=='o')
			{
				CPose2D 	odo;
				double 		v,w;
				int64_t  	left_ticks, right_ticks;
				robot.getOdometryFull( odo, v, w, left_ticks, right_ticks );

				cout << "Odometry: " << odo << " v: " << v << " w: " << RAD2DEG(w) << " left: " << left_ticks << " right: " << right_ticks << endl;
			}

			if (c=='p')
			{
				vector_bool bumps;
				robot.getBumpers(bumps);

				cout << "Bumpers: "<< bumps << endl;
			}

			if (c=='n' || c=='N')
			{
				CObservationRange obs;
				bool thereis;
				robot.getSonarsReadings(thereis,obs);

				if (!thereis)
				{
					cout << "Sonars: NO" << endl;
				}
				else
				{
 					cout << "Sonars: ";
					for (CObservationRange::const_iterator i=obs.sensedData.begin();i!=obs.sensedData.end();++i)
						cout << i->sensedDistance << " ";
					cout << endl;
				}
			}

			if (c=='b')
			{
				double bat;
				robot.getBatteryCharge(bat);
				cout << "Battery: " << bat << endl;
			}

		}


	}
	catch(std::exception &e)
	{
		cerr << e.what() << endl;
		return -1;
	}

	return 0;
}

